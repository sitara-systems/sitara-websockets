#pragma once

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <functional>
#include <memory>
#include <thread>
#include <system_error>
#include "Connection.h"

namespace midnight {
	namespace websocket {
		class Endpoint {
		public:
			Endpoint() {
				mNextId = 0;
			};

			~Endpoint() {
			};

			std::shared_ptr<Connection> getConnection(int id) {
				std::map<int, std::shared_ptr<Connection>>::iterator connection = mConnectionList.find(id);
				if (connection == mConnectionList.end()) {
					std::printf("No Connection Found with id %d\n", id);
					return nullptr;
				}
				else {
					return connection->second;
				}
			}

		protected:
			int mNextId;
			std::map<int, std::shared_ptr<Connection>> mConnectionList;
		};
	}
}
