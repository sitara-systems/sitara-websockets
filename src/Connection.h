#pragma once

#include <websocketpp/config/asio_no_tls_client.hpp>
#include<websocketpp/client.hpp>
#include <websocketpp/common/connection_hdl.hpp>

namespace midnight {
	namespace websocket {
		enum ConnectionStatus {
			CONNECTING,
			OPEN,
			CLOSED,
			FAILED
		};

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

			void setStatus(ConnectionStatus status) {
				mStatus = status;
			}

			ConnectionStatus getStatus() {
				return mStatus;
			}

			void recordMessage(websocketpp::client<websocketpp::config::asio_client>::message_ptr message) {
				if (message->get_opcode() == websocketpp::frame::opcode::text) {
					mMessages.push_back(message->get_payload());
				}
				else {
					mMessages.push_back(websocketpp::utility::to_hex(message->get_payload()));
				}
			}

			void setHandle(websocketpp::connection_hdl handle) {
				mHandle = handle;
			}

			websocketpp::connection_hdl getHandle() {
				return mHandle;
			}

			void setId(int id) {
				mId = id;
			}

			int getId() {
				return mId;
			}

			void setServer(std::string server) {
				mServer = server;
			}

			std::string getServer() {
				return mServer;
			}

			void setError(std::string error) {
				mErrorReason = error;
			}

			std::string getError() {
				return mErrorReason;
			}

			void statusUpdate() {
				std::printf("URI: %s\n Status: %s\n Remote Server: %s\n Error/Close Reason: %s\n Messages Processed: %d\n",
					mUri.c_str(), getStatusString(mStatus).c_str(), mServer.c_str(), mErrorReason.c_str(), mMessages.size());
			};

			std::string getStatusString(ConnectionStatus status) {
				switch (status) {
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

			void addOnReceiveFn(std::function<void(std::string message)> response) {
				mOnReceiveFns.push_back(response);
			};

			void callOnReceiveFns(websocketpp::client<websocketpp::config::asio_client>::message_ptr message) {
				for (auto callback : mOnReceiveFns) {
					callback(message->get_payload());
				}
			};

		protected:
			int mId;
			websocketpp::connection_hdl mHandle;
			ConnectionStatus mStatus;
			std::string mUri;
			std::string mServer;
			std::string mErrorReason;
			std::vector<std::string> mMessages;
			std::vector<std::function<void(std::string message)> > mOnReceiveFns;

		};
	}
}
