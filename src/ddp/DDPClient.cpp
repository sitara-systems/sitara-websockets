#include "ddp/DDPClient.h"

using namespace sitara::websocket;

DDPClient::DDPClient() : Client() {
    mConnectionId = -1;
    mIsConnected = false;
    mOnAddedFn = nullptr;
    mOnChangedFn = nullptr;
    mOnRemovedFn = nullptr;
}

DDPClient::~DDPClient() {
    Client::~Client();
}

int DDPClient::connect(const std::string& uri) {
    mConnectionId = Client::connect(uri);
    std::shared_ptr<sitara::websocket::Connection> connection =
        getConnection(mConnectionId);

    connection->setOnOpenFn(
        [&](std::shared_ptr<Connection> connection,
            websocketpp::client<websocketpp::config::asio_client>* client,
            websocketpp::connection_hdl handle) {
            Json::Value openMsg;
            openMsg["msg"] = "connect";
            if (!mSession.empty()) {
                openMsg["session"] = mSession;
            }
            openMsg["version"] = "1";
            openMsg["support"] = Json::arrayValue;
            openMsg["support"].append("1");

            send(mConnectionId, mJsonWriter.write(openMsg));
        });

    connection->setOnReceiveFn(
        [&](websocketpp::connection_hdl handle,
            websocketpp::client<websocketpp::config::asio_client>::message_ptr
                message) {
            // std::printf("Received Message : %s\n\n",
            // message->get_payload().c_str());

            Json::Value receivedMsg;
            mJsonReader.parse(message->get_payload(), receivedMsg);

            if (receivedMsg["msg"].isNull()) {
                std::cout << "Received null message" << std::endl;
            } else if (receivedMsg["msg"].asString() == "connected") {
                std::cout << "Successfully connected to DDP Server."
                          << std::endl;
                mIsConnected = true;
            } else if (receivedMsg["msg"].asString() == "failed") {
                if (receivedMsg["version"].asString() != "1") {
                    std::cout
                        << "Server requires an incompatible version of DDP ("
                        << receivedMsg["version"].asString() << ")"
                        << std::endl;
                    std::cout << "DDPClient will not try to reconnect."
                              << std::endl;
                    close(mConnectionId,
                          websocketpp::close::status::protocol_error);
                } else {
                    std::cout << "DDPClient failed to connect to DDP Server.  "
                                 "Protocol number (1) is correct; please "
                                 "investigate error further."
                              << std::endl;
                }
                mIsConnected = false;
            } else if (receivedMsg["msg"].asString() == "ping") {
                Json::Value response;
                response["msg"] = "pong";
                if (!receivedMsg["id"].asString().empty()) {
                    response["id"] = receivedMsg["id"].asString();
                }

                send(mConnectionId, mJsonWriter.write(response));
            } else if (receivedMsg["msg"].asString() == "added") {
                if (mOnAddedFn) {
                    mOnAddedFn(receivedMsg["collection"].asString(),
                               receivedMsg["id"].asString(),
                               receivedMsg["fields"]);
                }
            } else if (receivedMsg["msg"].asString() == "changed") {
                if (mOnChangedFn) {
                    mOnChangedFn(receivedMsg["collection"].asString(),
                                 receivedMsg["id"].asString(),
                                 receivedMsg["fields"]);
                }
            } else if (receivedMsg["msg"].asString() == "removed") {
                if (mOnRemovedFn) {
                    mOnRemovedFn(receivedMsg["collection"].asString(),
                                 receivedMsg["id"].asString());
                }
            } else if (receivedMsg["msg"].asString() == "ready") {
                for (Json::Value::ArrayIndex i = 0;
                     i != receivedMsg["subs"].size(); i++) {
                    mSubscriptionStatus[receivedMsg["subs"][i].asString()] =
                        ddp::SubscriptionStatus::READY;
                }
            } else if (receivedMsg["msg"].asString() == "error") {
                std::cout << "Received Error!" << std::endl;
                std::cout << message->get_payload() << std::endl;
            } else if (receivedMsg["msg"].asString() == "nosub") {
                mSubscriptionStatus[receivedMsg["id"].asString()] =
                    ddp::SubscriptionStatus::UNSUBSCRIBED;
            }
        });

    return mConnectionId;
}

bool DDPClient::isConnected() {
    return mIsConnected;
}

ddp::SubscriptionStatus DDPClient::getSubscriptionStatus(
    const std::string& subscriptionName) {
    if (mSubscriptionMap.find(subscriptionName) != mSubscriptionMap.end()) {
        std::string subscriptionId = mSubscriptionMap[subscriptionName];
        return mSubscriptionStatus[subscriptionId];
    } else {
        // not subscribed yet
        return ddp::SubscriptionStatus::UNSUBSCRIBED;
    }
}

std::string DDPClient::subscribe(const std::string& name) {
    std::string subscriptionId = generateId();

    Json::Value subMsg;
    subMsg["msg"] = "sub";
    if (!mSession.empty()) {
        subMsg["session"] = mSession;
    }
    subMsg["id"] = subscriptionId;
    subMsg["name"] = name;

    send(mConnectionId, mJsonWriter.write(subMsg));

    mSubscriptionMap.insert(std::make_pair(name, subscriptionId));
    mSubscriptionStatus.insert(
        std::make_pair(subscriptionId, ddp::SubscriptionStatus::CONNECTING));

    return subscriptionId;
}

void DDPClient::unsubscribe(const std::string& subscriptionId) {
    Json::Value unsubMsg;
    unsubMsg["msg"] = "unsub";
    unsubMsg["id"] = subscriptionId;

    send(mConnectionId, mJsonWriter.write(unsubMsg));
}

void DDPClient::setOnAddedFn(std::function<void(std::string collection,
                                                std::string documentId,
                                                Json::Value fields)> fn) {
    mOnAddedFn = fn;
}

void DDPClient::setOnChangedFn(std::function<void(std::string collection,
                                                  std::string documentId,
                                                  Json::Value fields)> fn) {
    mOnChangedFn = fn;
}

void DDPClient::setOnRemovedFn(
    std::function<void(std::string collection, std::string documentId)> fn) {
    mOnRemovedFn = fn;
}

unsigned int DDPClient::generateRandomCharacter() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

std::string DDPClient::generateId() {
    std::stringstream ss;
    for (auto i = 0; i < 16; i++) {
        const auto rc = generateRandomCharacter();
        std::stringstream hexstream;
        hexstream << std::hex << rc;
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

std::string DDPClient::toString(const Json::Value& json) {
    return mJsonWriter.write(json);
}