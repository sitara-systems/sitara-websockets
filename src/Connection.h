#include "websocketpp/common/connection_hdl.hpp"

namespace midnight {
  namespace websocket {
    enum ConnectionStatus {
      CONNECTING,
      OPEN,
      CLOSED,
      FAILED
    }

    class Connection {
      public:
        Connection(int id, websocketpp::connection_hdl handle, std::string uri) {
          mId = id;
          mHandle = handle;
          mStatus = ConnectionStatus::CONNECTING;
          mUri = uri;
          mServer = "N/A";
        };

        ~Connection() {
        };

      protected:
        void onOpen(websocketpp::client<websocketpp::config::asio_client> client, websocketpp::connection_hdl handle) {
          mStatus = ConnectionStatus::OPEN;
          websocketpp::client<websocketpp::config::asio_client>::connection_ptr connection = client->get_con_from_hdl(handle);
          mServer = connection->get_response_header("Server");
        };

        void onFail(websocketpp::client<websocketpp::config::asio_client> client, websocketpp::connection_hdl handle) {
          mStatus = ConnectionStatus::FAILED;
          websocketpp::client<websocketpp::config::asio_client>::connection_ptr connection = client->get_con_from_hdl(handle);
          mServer = connection->get_response_header("Server");
          mErrorReason = connection->get_ec().message();
        };

        void onClose(websocketpp::client<websocketpp::config::asio_client> client, websocketpp::connection_hdl handle) {
          mStatus = ConnectionStatus::CLOSED;
          websocketpp::client<websocketpp::config::asio_client>::connection_ptr connection = client->get_con_from_hdl(handle);
          // sprintf would be sloppy here, so I'm using stringstreams
          // I'm not typically a fan of stringstreams :(
          std::stringstream s;
          s << "Close Code: " << connection->get_remote_close_code() << " ("
          << websocketpp::close::status::get_string(connection->get_remote_close_code())
          << "), Close Reason: " << connection->get_remote_close_reason();
          mErrorReason = s.str();
        };

        ConnectionStatus getStatus() {
          return mStatus;
        }

        void statusUpdate() {
          std::printf("URI: %s\n
            Status: %s\n
            Remote Server: %s\n
            Error/Close Reason: %s\n",
            mUri.c_str(), getStatusString(mStatus).c_str(), mServer.c_str(), mErrorReason.c_str());
        };

        std::string getStatusString(ConnectionStatus status) {
            switch(status) {
              case 0:
                return "CONNECTING";
              case 1:
                return "OPEN";
              case 2:
                return "CLOSED";
              case 3:
                return "FAILED";
              default:
                return "N/A";
            }
        }
        int mId;
        websocketpp::connection_hdl mHandle;
        ConnectionStatus mStatus;
        std::string mUri;
        std::string mServer;
        std::string mErrorReason;
    }
  }
}
