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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <map>
#include <random>
#include <string>

#include "../Client.h"
#include "jsoncpp/json.h"

namespace sitara {
namespace websocket {
namespace ddp {
enum SubscriptionStatus { READY, CONNECTING, UNSUBSCRIBED };
}

class DDPClient : public Client {
   public:
    DDPClient();
    ~DDPClient();
    int connect(const std::string& uri);
    bool isConnected();
    ddp::SubscriptionStatus getSubscriptionStatus(
        const std::string& subscriptionName);
    std::string subscribe(const std::string& name);
    void unsubscribe(const std::string& subscriptionId);
    void setOnAddedFn(std::function<void(std::string collection,
                                         std::string documentId,
                                         Json::Value fields)> fn);
    void setOnChangedFn(std::function<void(std::string collection,
                                           std::string documentId,
                                           Json::Value fields)> fn);
    void setOnRemovedFn(
        std::function<void(std::string collection, std::string documentId)> fn);
    std::string toString(const Json::Value& json);

   protected:
    unsigned int generateRandomCharacter();
    std::string generateId();
    int mConnectionId;
    Json::Reader mJsonReader;
    Json::StyledWriter mJsonWriter;
    bool mIsConnected;
    bool mIsReady;
    std::string mSession;
    std::map<std::string, std::string> mSubscriptionMap;
    std::map<std::string, ddp::SubscriptionStatus> mSubscriptionStatus;
    std::function<void(std::string collection,
                       std::string documentId,
                       Json::Value fields)>
        mOnAddedFn;
    std::function<void(std::string collection,
                       std::string documentId,
                       Json::Value fields)>
        mOnChangedFn;
    std::function<void(std::string collection, std::string documentId)>
        mOnRemovedFn;
};
}  // namespace websocket
}  // namespace sitara
