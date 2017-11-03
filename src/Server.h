#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace midnight {
  namespace websocket {
    class Server {
    public:
      Server() {
        mPort = 9002;
        init();
      }

      Server(int port) {
        mPort = port;
        init();
      };

      ~Server() {
      };

      void run() {
        mServer.listen(mPort);
        mServer.start_accept();
        mServer.run();
      };

    protected:
      void init() {
        mServer.set_error_channels(websocketpp::log::elevel::all);
        mServer.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
        mServer.init_asio();
      }
      websocketpp::server<websocketpp::config::asio> mServer;
      int mPort;
    }
  }
}
