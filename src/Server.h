#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <functional>
#include <memory>
#include <system_error>

#include "Connection.h"
#include "Endpoint.h"

namespace midnight {
  namespace websocket {
    class Server : public Endpoint {
    public:
      Server() : Endpoint() {
        mPort = 9002;
        init();
	  };

      Server(int port) : Endpoint () {
        mPort = port;
        init();
      };

      ~Server() {
        if(!mServer.stopped()) {
          mServer.stop_listening();
          mServer.stop();
        }
      };

      void run() {
        mServer.listen(mPort);
        mServer.start_accept();
        mServer.run();
      };

      void send(int id, std::string message) {
        std::error_code errorCode;
		std::shared_ptr<Connection> connection = getConnection(id);
		websocketpp::client<websocketpp::config::asio_client>::message_ptr messagePtr;
        mServer.send(connection, message, websocketpp::frame::opcode::text, errorCode);
        if (errorCode) {
          std::printf("Error sending message: %s\n", errorCode.message().c_str());
          return;
        }
		messagePtr->set_payload(message);
		connection->recordMessage(messagePtr);
	  }

      void send(int id, void const * message, size_t length) {
		  std::shared_ptr<Connection> connection = getConnection(id);
		  std::error_code errorCode;
		  websocketpp::client<websocketpp::config::asio_client>::message_ptr messagePtr;
		  mServer.send(connection, message, length, websocketpp::frame::opcode::binary, errorCode);
        if (errorCode) {
          std::printf("Error sending message: %s\n", errorCode.message().c_str());
          return;
        }
		messagePtr->set_payload(message, length);
		connection->recordMessage(messagePtr);
	  }

	protected:
      void init() {
        mServer.set_error_channels(websocketpp::log::elevel::all);
        mServer.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
        mServer.init_asio();

        mServer.set_open_handler(std::bind(
          &Server::onOpen,
          this,
          &mServer,
          std::placeholders::_1
        ));

	    mServer.set_fail_handler(std::bind(
          &Server::onFail,
          this,
          &mServer,
          std::placeholders::_1
        ));

        mServer.set_close_handler(std::bind(
          &Server::onClose,
          this,
          &mServer,
          std::placeholders::_1
        ));

        mServer.set_message_handler(std::bind(
          &Server::onReceive,
          this,
          &mServer,
          std::placeholders::_1,
          std::placeholders::_2
        ));
      }

      static void onOpen(std::shared_ptr<Server> server, websocketpp::server<websocketpp::config::asio> wsServer, websocketpp::connection_hdl handle) {
			  std::printf("Server opened!\n");
			  int newId = server->mNextId++;
			  std::shared_ptr<Connection> newConnection(new Connection(newId, handle, ""));
			  connection->setStatus(ConnectionStatus::OPEN);
			  mConnectionList[newId] = newConnection;
      };

      static void onFail(std::shared_ptr<Connection> connection, websocketpp::server<websocketpp::config::asio> server, websocketpp::connection_hdl handle) {
		     connection->setStatus(ConnectionStatus::FAILED);
      };

      static void onClose(std::shared_ptr<Connection> connection, websocketpp::server<websocketpp::config::asio> server, websocketpp::connection_hdl handle) {
          connection->setStatus(ConnectionStatus::CLOSED);
          std::printf("Server closed.\n");
      };

	  static void onReceive(std::shared_ptr<Connection> connection, websocketpp::connection_hdl handle, websocketpp::server<websocketpp::config::asio>::message_ptr message) {
		  connection->callOnReceiveFns(message);
		  connection->recordMessage(message);
		  std::printf("received message %s", message->get_payload().c_str());
	  };

      websocketpp::server<websocketpp::config::asio> mServer;
      int mPort;
	};
  }
}
