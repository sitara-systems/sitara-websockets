#pragma once

#include <functional>
#include <memory>
#include <system_error>
#include <thread>
#include <websocketpp/config/asio_no_tls_client.hpp>

#include "Connection.h"

namespace sitara {
namespace websocket {
class Endpoint {
   public:
    Endpoint() : mNextId(0){};

    ~Endpoint(){};

    void removeConnection(int id) {
        std::shared_ptr<Connection> connection = getConnection(id);
        removeConnection(connection);
    }

    void removeConnection(websocketpp::connection_hdl handle) {
        std::shared_ptr<Connection> connection = getConnection(handle);
        removeConnection(connection);
    }

    void removeConnection(std::shared_ptr<Connection> connection) {
        websocketpp::connection_hdl handle = connection->getHandle();
        int id = connection->getId();
        mConnectionList.erase(id);
        mHandleMap.erase(handle);
    }

    std::shared_ptr<Connection> getConnection(int id) {
        std::map<int, std::shared_ptr<Connection>>::iterator connection =
            mConnectionList.find(id);
        if (connection == mConnectionList.end()) {
            std::printf("No Connection Found with id %d\n", id);
            return nullptr;
        } else {
            return connection->second;
        }
    }

    std::shared_ptr<Connection> getConnection(
        websocketpp::connection_hdl handle) {
        std::map<websocketpp::connection_hdl,
                 std::shared_ptr<Connection>>::iterator connection =
            mHandleMap.find(handle);
        if (connection == mHandleMap.end()) {
            std::printf("No Connection Found with matching handle\n");
            return nullptr;
        } else {
            return connection->second;
        }
    }

   protected:
    int mNextId;
    std::map<int, std::shared_ptr<Connection>> mConnectionList;
    std::map<websocketpp::connection_hdl,
             std::shared_ptr<Connection>,
             std::owner_less<websocketpp::connection_hdl>>
        mHandleMap;
};
}  // namespace websocket
}  // namespace sitara
