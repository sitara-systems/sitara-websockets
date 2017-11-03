#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
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
          mEndpoint.clear_access_channels(websocketpp::log::alevel::all);
          mEndpoint.clear_error_channels(websocketpp::log::elevel::all);
          mEndpoint.init_asio();
          mEndpoint.start_perpetual();
          mThread.reset(new std::thread(&mClient::run, &mEndpoint));
        };

        ~Client() {
          mClient.stop_perpetual();

          for (std::map<int, std::shared_ptr<Connection>>::const_iterator it = mConnectionList.begin(); it != mConnectionList.end(); ++it) {
            if (it->second->getStatus() != Connection::ConnectionStatus::OPEN) {
              // Only close open connections
              continue;
            }

            std::printf("Closing connection %d\n", it->second->get_id());

            std::error_code errorCode;

            mClient.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", errorCode);
            if (errorCode) {
                std::printf("Error closing connection %d: %s\n", it->second->get_id(), errorCode.message().c_str());
            }
          }

          mThread->join();
        };

        int connect(std::string& uri) {
          std::error_code errorCode;
          websocketpp::client<websocketpp::config::asio_client>::connection_ptr connection = mClient.get_connection(uri, errorCode);
          if(errorCode) {
            std::printf("Connect initialization error: %s\n", errorCode.message().c_str());
            return -1;
          }

          int newId = mNextId++;
          std::shared_ptr<Connection> newConnection(new Connection(newId, connection->getHandle(), uri));
          mConnectionList[newId] = newConnection;

          connection->set_open_handler(std::bind(
            &Connection::onOpen,
            newConnection,
            &mClient,
            std::placeholders::_1
          ));

          connection_>set_fail_handler(std::bind(
            &Connection::onFail,
            newConnection,
            &mClient,
            std::placeholders::_1
          ))

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
            mClient.close(it->second->get_hdl(), close_code, "", errorCode);
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
            mClient.close(it->second->get_hdl(), close_code, "", errorCode);
            if(errorCode) {
              std::printf("Error initiating close: %s\n", errorCode.message().c_str());
            }
          }

      protected:
        websocketpp::client<websocketpp::config::asio_client> mClient;
        std::shared_ptr<std::thread> mThread;
        std::map<int, std::shared_ptr<Connection>> mConnectionList;
        int mNextId;
    }
  }
}
