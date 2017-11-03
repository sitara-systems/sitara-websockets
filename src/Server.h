#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <functional>
#include <memory>
#include <system_error>

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

      void send(std::string message) {
        std::error_code errorCode;
        mServer.send(mHandle, message, websocketpp::frame::opcode::text, errorCode);
        if (errorCode) {
          std::printf("Error sending message: %s\n", errorCode.message().c_str());
          return;
        }
        recordMessage(message);
      }

      void send(void const * message, size_t length) {
        std::error_code errorCode;
        mServer.send(mHandle, message, length, websocketpp::frame::opcode::binary, errorCode);
        if (errorCode) {
          std::printf("Error sending message: %s\n", errorCode.message().c_str());
          return;
        }
		//not sure this will work
        //recordMessage(message);
      }

	  void addOnReceiveFn(std::function<void(std::string message)> response) {
		  mOnReceiveFns.push_back(response);
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

        void recordMessage(std::string message) {
          mMessages.push_back(message);
        }

        void recordMessage(websocketpp::server<websocketpp::config::asio>::message_ptr message) {
          if(message->get_opcode() == websocketpp::frame::opcode::text) {
            mMessages.push_back(message->get_payload());
          }
          else {
            mMessages.push_back(websocketpp::utility::to_hex(message->get_payload()));
          }
        }	

      void onOpen(websocketpp::server<websocketpp::config::asio> server, websocketpp::connection_hdl handle) {
        std::printf("Server opened!\n");
        mHandle = handle;
      };

      void onFail(websocketpp::server<websocketpp::config::asio> server, websocketpp::connection_hdl handle) {
        mHandle = handle;
      };

      void onClose(websocketpp::server<websocketpp::config::asio> server, websocketpp::connection_hdl handle) {
        std::printf("Server closed.\n");
      };

	  void onReceive(websocketpp::connection_hdl handle, websocketpp::server<websocketpp::config::asio>::message_ptr message) {
		  for (auto callback : mOnReceiveFns) {
			  callback(message->get_payload());
		  }
		  recordMessage(message);
		  std::printf("received message %s", message->get_payload().c_str());
	  };

      websocketpp::connection_hdl mHandle;
      websocketpp::server<websocketpp::config::asio> mServer;
      int mPort;
      std::vector<std::function<void(std::string message)> > mOnReceiveFns;
      std::vector<std::string> mMessages;
	};
  }
}
