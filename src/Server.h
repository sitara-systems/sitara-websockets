#pragma once

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/server.hpp>
#include <functional>
#include <memory>
#include <system_error>

#include "Connection.h"
#include "Endpoint.h"

namespace sitara {
	namespace websocket {
		class Server : public Endpoint {
		public:
			Server() : Endpoint() {
				mPort = 9002;
				init();
			};

			Server(int port) : Endpoint() {
				mPort = port;
				init();
			};

			~Server() {
				if (!mServer.stopped()) {
					mServer.stop_listening();
					mServer.stop();
				}
			};

			void run() {
				mServer.listen(mPort);
				mServer.start_accept();
				mServer.run();
			};

			void stop() {
				std::error_code errorCode;
				mServer.stop_listening(errorCode);
				if (errorCode) {
					std::printf("Failed to stop listening: %s\n", errorCode.message().c_str());
					return;
				}
				for (std::map<int, std::shared_ptr<Connection>>::const_iterator it = mConnectionList.begin(); it != mConnectionList.end(); ++it) {
					if (it->second->getStatus() != ConnectionStatus::OPEN) {
						// Only close open connections
						continue;
					}
					std::printf("Closing connection %d\n", it->second->getId());
					std::error_code errorCode;
				}

				mServer.stop();
			}

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
			};

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
			};

			bool sendClose(int id) {
				websocketpp::connection_hdl handle;
				std::error_code errorCode;
				std::shared_ptr<Connection> connection = getConnection(id);
				if (connection == nullptr) {
					return false;
				}
				handle = connection->getHandle();
				std::string data = "Terminating connection...";
				mServer.close(handle, websocketpp::close::status::normal, data, errorCode);
				if (errorCode) {
					return false;
				}
				mConnectionList.erase(id);
				return true;
			}

		protected:
			void init() {
				mServer.set_error_channels(websocketpp::log::elevel::all);
				mServer.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
				mServer.init_asio();

				mServer.set_open_handler([&](websocketpp::connection_hdl handle) {
					onOpen(this, &mServer, handle);
				});

				mServer.set_fail_handler([&](websocketpp::connection_hdl handle) {
					onFail(this, &mServer, handle);
				});

				mServer.set_close_handler([&](websocketpp::connection_hdl handle) {
					onClose(this, &mServer, handle);
				});

				mServer.set_message_handler([&](websocketpp::connection_hdl handle, websocketpp::server<websocketpp::config::asio>::message_ptr message) {
					onReceive(this, &mServer, handle, message);
				});

				mServer.set_socket_init_handler([&](websocketpp::connection_hdl handle, asio::ip::tcp::socket& socket) {
					onSocketInit(this, &mServer, handle, socket);
				});

				/*
				The library recommended using std::bind for binding function arguments, but this seems to cause errors.  Lambdas seem more flexible.

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

				mServer.set_socket_init_handler(std::bind(
					&Server::onSocketInit,
					this,
					&mServer,
					std::placeholders::_1,
					std::placeholders::_2
				));
				*/
			};

			// these handlers should probably use a std::shared_ptr<Server>, but a regular pointer will do for now...
			static void onOpen(Server* server, websocketpp::server<websocketpp::config::asio>* wsServer, websocketpp::connection_hdl handle) {
				std::printf("Server opened!\n");
				int newId = server->mNextId++;
				std::shared_ptr<Connection> newConnection(new Connection(newId, handle, ""));
			};

			static void onFail(Server* server, websocketpp::server<websocketpp::config::asio>* wsServer, websocketpp::connection_hdl handle) {
				websocketpp::server<websocketpp::config::asio>::connection_ptr connection = wsServer->get_con_from_hdl(handle);
				websocketpp::lib::error_code errorCode = connection->get_ec();
			};

			static void onClose(Server* server, websocketpp::server<websocketpp::config::asio>* wsServer, websocketpp::connection_hdl handle) {
				std::printf("Server closed.\n");
			};

			static void onReceive(Server* server, websocketpp::server<websocketpp::config::asio>* wsServer, websocketpp::connection_hdl handle, websocketpp::server<websocketpp::config::asio>::message_ptr message) {
				std::printf("received message %s", message->get_payload().c_str());
			};

			static void onSocketInit(Server* server, websocketpp::server<websocketpp::config::asio>* wsServer, websocketpp::connection_hdl handle, asio::ip::tcp::socket& socket) {
				int newId = server->mNextId++;
				websocketpp::server<websocketpp::config::asio>::connection_ptr connection = wsServer->get_con_from_hdl(handle);
				// not sure get_query() is actually right; should be able to pull the URI from the uri_ptr object somehow.
				//std::shared_ptr<Connection> newConnection(new Connection(newId, handle, connection->get_uri()->get_query()));
				std::shared_ptr<Connection> newConnection(new Connection(newId, handle, ""));
				server->mConnectionList[newId] = newConnection;
			};

			websocketpp::server<websocketpp::config::asio> mServer;
			int mPort;
		};
	};
};