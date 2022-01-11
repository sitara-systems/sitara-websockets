// Copyright (c) 2022 Nathan S Lachenmyer
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <system_error>
#include <thread>
#include <websocketpp/config/asio_no_tls_client.hpp>

#include "Connection.h"

namespace sitara {
namespace websocket {
class Endpoint {
   public:
    Endpoint() : mNextId(0){}

    ~Endpoint(){}

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
