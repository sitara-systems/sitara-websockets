#pragma once

#include <random>

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
