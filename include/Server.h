#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "Connection.h"
#include "Endpoint.h"

namespace sitara {
namespace websocket {
class Server : public Endpoint {
   public:
    Server() : Endpoint() {
        mPort = 9002;
        init();
    }

    explicit Server(int port) : Endpoint() {
        mPort = port;
        init();
    }

    ~Server() {
        if (!mServer.stopped()) {
            mServer.stop_listening();
            mServer.stop();
        }
    }

    void run() {
        mServer.listen(mPort);
        mServer.start_accept();
        mServer.run();
    }

    void stop() {
        std::error_code errorCode;
        mServer.stop_listening(errorCode);
        if (errorCode) {
            std::printf("Failed to stop listening: %s\n",
                        errorCode.message().c_str());
            return;
        }
        for (std::map<int, std::shared_ptr<Connection>>::const_iterator it =
                 mConnectionList.begin();
             it != mConnectionList.end(); ++it) {
            if (it->second->getStatus() != ConnectionStatus::OPEN) {
                // Only close open connections
                continue;
            }
            std::printf("Closing connection %d\n", it->second->getId());
            std::error_code errorCode;
        }

        mServer.stop();
    }

    std::shared_ptr<Connection> createConnection(
        websocketpp::server<websocketpp::config::asio>::connection_ptr
            connection) {
        mNextId++;
        std::shared_ptr<Connection> newConnection(new Connection(
            mNextId, connection->get_handle(), connection->get_uri()->str()));
        mConnectionList.insert(std::make_pair(mNextId, newConnection));
        mHandleMap.insert(
            std::make_pair(connection->get_handle(), newConnection));
        return newConnection;
    }

    void send(int id, std::string message) {
        std::error_code errorCode;
        std::shared_ptr<Connection> connection = getConnection(id);
        websocketpp::client<websocketpp::config::asio_client>::message_ptr
            messagePtr;
        mServer.send(connection, message, websocketpp::frame::opcode::text,
                     errorCode);
        if (errorCode) {
            std::printf("Error sending message: %s\n",
                        errorCode.message().c_str());
            return;
        }
        // messagePtr->set_payload(message);
        // connection->recordMessage(messagePtr);
    }

    void send(int id, void const* message, size_t length) {
        std::shared_ptr<Connection> connection = getConnection(id);
        std::error_code errorCode;
        websocketpp::client<websocketpp::config::asio_client>::message_ptr
            messagePtr;
        mServer.send(connection, message, length,
                     websocketpp::frame::opcode::binary, errorCode);
        if (errorCode) {
            std::printf("Error sending message: %s\n",
                        errorCode.message().c_str());
            return;
        }
        // messagePtr->set_payload(message, length);
        // connection->recordMessage(messagePtr);
    }

    void send(
        int id,
        websocketpp::server<websocketpp::config::asio>::message_ptr message) {
        std::error_code errorCode;
        std::shared_ptr<Connection> connection = getConnection(id);
        if (connection != nullptr) {
            mServer.send(connection->getHandle(), message, errorCode);
            if (errorCode) {
                std::printf("Error sending message: %s\n",
                            errorCode.message().c_str());
                return;
            }
            connection->recordMessage(message);
        }
    }

    bool sendClose(int id) {
        websocketpp::connection_hdl handle;
        std::error_code errorCode;
        std::shared_ptr<Connection> connection = getConnection(id);
        if (connection == nullptr) {
            return false;
        }
        handle = connection->getHandle();
        std::string data = "Terminating connection...";
        mServer.close(handle, websocketpp::close::status::normal, data,
                      errorCode);
        if (errorCode) {
            return false;
        }
        mConnectionList.erase(id);
        return true;
    }

    void addOnReceiveFn(
        std::function<void(websocketpp::connection_hdl handle,
                           websocketpp::server<websocketpp::config::asio>::
                               message_ptr message)> response) {
        mOnReceiveFns.push_back(response);
    }

   protected:
    void init() {
        mServer.set_access_channels(websocketpp::log::alevel::all ^
                                    websocketpp::log::alevel::frame_payload);
        mServer.clear_access_channels(websocketpp::log::alevel::frame_payload);
        mServer.set_error_channels(websocketpp::log::elevel::all);
        mServer.init_asio();

        mServer.set_open_handler(
            std::bind(&Server::onOpen, this, &mServer, std::placeholders::_1));

        mServer.set_fail_handler(
            std::bind(&Server::onFail, this, &mServer, std::placeholders::_1));

        mServer.set_close_handler(
            std::bind(&Server::onClose, this, &mServer, std::placeholders::_1));

        mServer.set_message_handler(std::bind(&Server::onReceive, this,
                                              &mServer, std::placeholders::_1,
                                              std::placeholders::_2));

        mServer.set_socket_init_handler(
            std::bind(&Server::onSocketInit, this, &mServer,
                      std::placeholders::_1, std::placeholders::_2));
    }

    // these handlers should probably use a std::shared_ptr<Server>, but a
    // regular pointer will do for now...
    static void onOpen(Server* server,
                       websocketpp::server<websocketpp::config::asio>* wsServer,
                       websocketpp::connection_hdl handle) {
        std::printf("Server Connected!\n");
        websocketpp::server<websocketpp::config::asio>::connection_ptr
            wsConnection = wsServer->get_con_from_hdl(handle);

        std::shared_ptr<Connection> newConnection =
            server->createConnection(wsConnection);
        newConnection->setStatus(ConnectionStatus::OPEN);
        newConnection->printStatus();
    }

    static void onFail(Server* server,
                       websocketpp::server<websocketpp::config::asio>* wsServer,
                       websocketpp::connection_hdl handle) {
        std::printf("Server failed to connect.\n");
        std::shared_ptr<Connection> connection = server->getConnection(handle);
        connection->setStatus(ConnectionStatus::FAILED);

        websocketpp::server<websocketpp::config::asio>::connection_ptr
            wsConnection = wsServer->get_con_from_hdl(handle);
        connection->setEndpoint(wsConnection->get_response_header("Server"));
        connection->setError(wsConnection->get_ec().message());
    }

    static void onClose(
        Server* server,
        websocketpp::server<websocketpp::config::asio>* wsServer,
        websocketpp::connection_hdl handle) {
        std::shared_ptr<Connection> connection = server->getConnection(handle);
        connection->setStatus(ConnectionStatus::CLOSED);
        websocketpp::server<websocketpp::config::asio>::connection_ptr
            wsConnection = wsServer->get_con_from_hdl(handle);
        // sprintf would be sloppy here, so I'm using stringstreams
        // I'm not typically a fan of stringstreams :(
        std::stringstream s;
        s << "Close Code: " << wsConnection->get_remote_close_code() << " ("
          << websocketpp::close::status::get_string(
                 wsConnection->get_remote_close_code())
          << "), Close Reason: " << wsConnection->get_remote_close_reason();
        connection->setError(s.str());
    }

    static void onReceive(
        Server* server,
        websocketpp::server<websocketpp::config::asio>* wsServer,
        websocketpp::connection_hdl handle,
        websocketpp::server<websocketpp::config::asio>::message_ptr message) {
        std::shared_ptr<Connection> connection = server->getConnection(handle);
        for (auto callback : server->mOnReceiveFns) {
            callback(handle, message);
        }
        // connection->callOnReceiveFns(handle, message);
        // connection->recordMessage(message);
    }

    static void onSocketInit(
        Server* server,
        websocketpp::server<websocketpp::config::asio>* wsServer,
        websocketpp::connection_hdl handle,
        const asio::ip::tcp::socket& socket) {
        std::shared_ptr<Connection> connection = server->getConnection(handle);
    }

    websocketpp::server<websocketpp::config::asio> mServer;
    std::vector<std::function<void(
        websocketpp::connection_hdl handle,
        websocketpp::server<websocketpp::config::asio>::message_ptr message)>>
        mOnReceiveFns;
    int mPort;
};
}  // namespace websocket
}  // namespace sitara
