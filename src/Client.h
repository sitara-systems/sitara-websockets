#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <functional>
#include <memory>
#include <thread>
#include <system_error>
#include "Connection.h"

namespace midnight {
  namespace websocket {
    class Client {
      public:
        Client() {
          mNextId = 0;
          mClient.clear_access_channels(websocketpp::log::alevel::all);
          mClient.clear_error_channels(websocketpp::log::elevel::all);
          mClient.init_asio();
          mClient.start_perpetual();
          mThread = std::thread(&websocketpp::client<websocketpp::config::asio_client>::run, &mClient);
        };

        ~Client() {
          mClient.stop_perpetual();

          for (std::map<int, std::shared_ptr<Connection>>::const_iterator it = mConnectionList.begin(); it != mConnectionList.end(); ++it) {
            if (it->second->getStatus() != ConnectionStatus::OPEN) {
              // Only close open connections
              continue;
            }

            std::printf("Closing connection %d\n", it->second->getId());

            std::error_code errorCode;

            mClient.close(it->second->getHandle(), websocketpp::close::status::going_away, "", errorCode);
            if (errorCode) {
                std::printf("Error closing connection %d: %s\n", it->second->getId(), errorCode.message().c_str());
            }
          }

          if(mThread.joinable()) {
            mThread.join();
          }
        };

        int connect(std::string& uri) {
          std::error_code errorCode;
          websocketpp::client<websocketpp::config::asio_client>::connection_ptr connection = mClient.get_connection(uri, errorCode);
          if(errorCode) {
            std::printf("Connect initialization error: %s\n", errorCode.message().c_str());
            return -1;
          }

          int newId = mNextId++;
          std::shared_ptr<Connection> newConnection(new Connection(newId, connection->get_handle(), uri));
          mConnectionList[newId] = newConnection;

          connection->set_open_handler(std::bind(
            &Connection::onOpen,
            newConnection,
            &mClient,
            std::placeholders::_1
          ));

          connection->set_fail_handler(std::bind(
            &Connection::onFail,
            newConnection,
            &mClient,
            std::placeholders::_1
          ));

          connection->set_close_handler(std::bind(
            &Connection::onClose,
            newConnection,
            &mClient,
            std::placeholders::_1
          ));

          connection->set_message_handler(std::bind(
            &Connection::onReceive,
            newConnection,
            &mClient,
            std::placeholders::_1,
            std::placeholders::_2
          ));

          mClient.connect(connection);
          return newId;
        };

        std::shared_ptr<Connection> getConnectionData(int id) {
          std::map<int, std::shared_ptr<Connection>>::const_iterator it = mConnectionList.find(id);
          if(it == mConnectionList.end()) {
            return std::shared_ptr<Connection>();
          }
          else {
            return it->second;
          }
        };

        void close(int id) {
            int close_code = websocketpp::close::status::normal;
            std::error_code errorCode;
            std::map<int, std::shared_ptr<Connection>>::const_iterator it = mConnectionList.find(id);
            if(it == mConnectionList.end()) {
              std::printf("No Connection Found with id %d\n", id);
              return;
            }
            mClient.close(it->second->getHandle(), close_code, "", errorCode);
            if(errorCode) {
              std::printf("Error initiating close: %s\n", errorCode.message().c_str());
            }
          }

          void close(int id, websocketpp::close::status::value close_code) {
            std::error_code errorCode;
            std::map<int, std::shared_ptr<Connection>>::const_iterator it = mConnectionList.find(id);
            if(it == mConnectionList.end()) {
              std::printf("No Connection Found with id %d\n", id);
              return;
            }
            mClient.close(it->second->getHandle(), close_code, "", errorCode);
            if(errorCode) {
              std::printf("Error initiating close: %s\n", errorCode.message().c_str());
            }
          }

          void send(int id, std::string message) {
            std::error_code errorCode;
            std::map<int, std::shared_ptr<Connection>>::iterator it = mConnectionList.find(id);
            if(it == mConnectionList.end()) {
              std::printf("No connection found with id %d\n", id);
              return;
            }
            mClient.send(it->second->getHandle(), message, websocketpp::frame::opcode::text, errorCode);
            if (errorCode) {
              std::printf("Error sending message: %s\n", errorCode.message().c_str());
              return;
            }
            it->second->recordMessage(message);
          }

          void send(int id, void const * payload, size_t length) {
            std::error_code errorCode;
            std::map<int, std::shared_ptr<Connection>>::iterator it = mConnectionList.find(id);
            if(it == mConnectionList.end()) {
              std::printf("No connection found with id %d\n", id);
              return;
            }
            mClient.send(it->second->getHandle(), payload, length, websocketpp::frame::opcode::binary, errorCode);
            if (errorCode) {
              std::printf("Error sending message: %s\n", errorCode.message().c_str());
              return;
            }
            //it->second->recordMessage();
          }

          void send(int id, websocketpp::client<websocketpp::config::asio_client>::message_ptr message) {
            std::error_code errorCode;
            std::map<int, std::shared_ptr<Connection>>::iterator it = mConnectionList.find(id);
            if(it == mConnectionList.end()) {
              std::printf("No connection found with id %d\n", id);
              return;
            }
            mClient.send(it->second->getHandle(), message, errorCode);
            if (errorCode) {
              std::printf("Error sending message: %s\n", errorCode.message().c_str());
              return;
            }
            it->second->recordMessage(message->get_payload());
          }

      protected:
        websocketpp::client<websocketpp::config::asio_client> mClient;
        std::thread mThread;
        std::map<int, std::shared_ptr<Connection>> mConnectionList;
        int mNextId;
	};
  }
}
