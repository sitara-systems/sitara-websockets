#pragma once

#include <functional>
#include <memory>
#include <system_error>
#include <thread>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

#include "Connection.h"
#include "Endpoint.h"

namespace sitara {
namespace websocket {
class Client : public Endpoint {
   public:
    Client() : Endpoint() {
        mClient.clear_access_channels(websocketpp::log::alevel::all);
        mClient.clear_error_channels(websocketpp::log::elevel::all);
        mClient.init_asio();
        mClient.start_perpetual();
        mThread = std::thread(
            &websocketpp::client<websocketpp::config::asio_client>::run,
            &mClient);
    };

    ~Client() {
        mClient.stop_perpetual();

        for (std::map<int, std::shared_ptr<Connection>>::const_iterator it =
                 mConnectionList.begin();
             it != mConnectionList.end(); ++it) {
            if (it->second->getStatus() != ConnectionStatus::OPEN) {
                // Only close open connections
                continue;
            }

            std::printf("Closing connection %d\n", it->second->getId());

            std::error_code errorCode;

            mClient.close(it->second->getHandle(),
                          websocketpp::close::status::going_away, "",
                          errorCode);
            if (errorCode) {
                std::printf("Error closing connection %d: %s\n",
                            it->second->getId(), errorCode.message().c_str());
            }
        }

        if (mThread.joinable()) {
            mThread.join();
        }
    };

    std::shared_ptr<Connection> createConnection(
        int id,
        websocketpp::client<websocketpp::config::asio_client>::connection_ptr
            connection) {
        std::shared_ptr<Connection> newConnection(new Connection(
            mNextId, connection->get_handle(), connection->get_uri()->str()));
        mConnectionList.insert(std::make_pair(mNextId, newConnection));
        mHandleMap.insert(
            std::make_pair(connection->get_handle(), newConnection));
        return newConnection;
    }

    int connect(const std::string& uri) {
        std::error_code errorCode;
        websocketpp::client<websocketpp::config::asio_client>::connection_ptr
            connection = mClient.get_connection(uri, errorCode);
        if (errorCode) {
            std::printf("Connect initialization error: %s\n",
                        errorCode.message().c_str());
            return -1;
        }

        mNextId++;
        std::shared_ptr<Connection> newConnection =
            createConnection(mNextId, connection);

        connection->set_open_handler(std::bind(
            &Client::onOpen, newConnection, &mClient, std::placeholders::_1));

        connection->set_fail_handler(std::bind(
            &Client::onFail, newConnection, &mClient, std::placeholders::_1));

        connection->set_close_handler(std::bind(
            &Client::onClose, newConnection, &mClient, std::placeholders::_1));

        connection->set_message_handler(
            std::bind(&Client::onReceive, newConnection, &mClient,
                      std::placeholders::_1, std::placeholders::_2));

        mClient.connect(connection);
        return mNextId;
    };

    void close(int id) {
        int close_code = websocketpp::close::status::normal;
        std::error_code errorCode;
        std::shared_ptr<Connection> connection = getConnection(id);
        if (connection != nullptr) {
            mClient.close(connection->getHandle(), close_code, "", errorCode);
            if (errorCode) {
                std::printf("Error initiating close: %s\n",
                            errorCode.message().c_str());
            }
        }
    };

    void close(int id, websocketpp::close::status::value close_code) {
        std::error_code errorCode;
        std::shared_ptr<Connection> connection = getConnection(id);
        if (connection != nullptr) {
            mClient.close(connection->getHandle(), close_code, "", errorCode);
            if (errorCode) {
                std::printf("Error initiating close: %s\n",
                            errorCode.message().c_str());
            }
        }
    }

    void send(int id, const std::string& message) {
        std::error_code errorCode;
        std::shared_ptr<Connection> connection = getConnection(id);
        websocketpp::client<websocketpp::config::asio_client>::message_ptr
            messagePtr;
        if (connection != nullptr) {
            mClient.send(connection->getHandle(), message,
                         websocketpp::frame::opcode::text, errorCode);
            if (errorCode) {
                std::printf("Error sending message: %s\n",
                            errorCode.message().c_str());
                return;
            }
            // messagePtr->set_payload(message);
            // connection->recordMessage(messagePtr);
        }
    };

    void send(int id, void const* payload, size_t length) {
        std::error_code errorCode;
        std::shared_ptr<Connection> connection = getConnection(id);
        websocketpp::client<websocketpp::config::asio_client>::message_ptr
            messagePtr;
        if (connection != nullptr) {
            mClient.send(connection->getHandle(), payload, length,
                         websocketpp::frame::opcode::binary, errorCode);
            if (errorCode) {
                std::printf("Error sending message: %s\n",
                            errorCode.message().c_str());
                return;
            }
        }
        // messagePtr->set_payload(payload, length);
        // connection->recordMessage(messagePtr);
    }

    void send(int id,
              websocketpp::client<websocketpp::config::asio_client>::message_ptr
                  message) {
        std::error_code errorCode;
        std::shared_ptr<Connection> connection = getConnection(id);
        if (connection != nullptr) {
            mClient.send(connection->getHandle(), message, errorCode);
            if (errorCode) {
                std::printf("Error sending message: %s\n",
                            errorCode.message().c_str());
                return;
            }
            connection->recordMessage(message);
        }
    }

    static void onOpen(
        std::shared_ptr<Connection> connection,
        websocketpp::client<websocketpp::config::asio_client>* client,
        websocketpp::connection_hdl handle) {
        connection->setStatus(ConnectionStatus::OPEN);
        websocketpp::client<websocketpp::config::asio_client>::connection_ptr
            clientConnection = client->get_con_from_hdl(handle);
        connection->setEndpoint(
            clientConnection->get_response_header("Server"));
        std::printf("Client Connected!\n");
        connection->printStatus();
        connection->callOnOpenFn(connection, client, handle);
    };

    static void onFail(
        std::shared_ptr<Connection> connection,
        websocketpp::client<websocketpp::config::asio_client>* client,
        websocketpp::connection_hdl handle) {
        std::printf("Client failed to connect.\n");
        connection->setStatus(ConnectionStatus::FAILED);
        websocketpp::client<websocketpp::config::asio_client>::connection_ptr
            clientConnection = client->get_con_from_hdl(handle);
        connection->setEndpoint(
            clientConnection->get_response_header("Server"));
        connection->setError(clientConnection->get_ec().message());
        connection->callOnFailFn(connection, client, handle);
    };

    static void onClose(
        std::shared_ptr<Connection> connection,
        websocketpp::client<websocketpp::config::asio_client>* client,
        websocketpp::connection_hdl handle) {
        connection->setStatus(ConnectionStatus::CLOSED);
        websocketpp::client<websocketpp::config::asio_client>::connection_ptr
            clientConnection = client->get_con_from_hdl(handle);
        // sprintf would be sloppy here, so I'm using stringstreams
        // I'm not typically a fan of stringstreams :(
        std::stringstream s;
        s << "Close Code: " << clientConnection->get_remote_close_code() << " ("
          << websocketpp::close::status::get_string(
                 clientConnection->get_remote_close_code())
          << "), Close Reason: " << clientConnection->get_remote_close_reason();
        connection->setError(s.str());
        connection->callOnCloseFn(connection, client, handle);
    };

    static void onReceive(
        std::shared_ptr<Connection> connection,
        websocketpp::client<websocketpp::config::asio_client>* client,
        websocketpp::connection_hdl handle,
        websocketpp::client<websocketpp::config::asio_client>::message_ptr
            message) {
        connection->callOnReceiveFn(handle, message);
        connection->recordMessage(message);
    };

   protected:
    websocketpp::client<websocketpp::config::asio_client> mClient;
    std::thread mThread;
};
}  // namespace websocket
}  // namespace sitara
