#pragma once

#include <memory>
#include <string>
#include <vector>
#include <websocketpp/client.hpp>
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

namespace sitara {
namespace websocket {
enum ConnectionStatus { CONNECTING, OPEN, CLOSED, FAILED };

class Connection {
   public:
    Connection(int id, websocketpp::connection_hdl handle, std::string uri) {
        mId = id;
        mHandle = handle;
        mStatus = ConnectionStatus::CONNECTING;
        mUri = uri;
        mEndpoint = "N/A";
        mOnOpenFn = nullptr;
        mOnFailFn = nullptr;
        mOnCloseFn = nullptr;
        mOnReceiveFn = nullptr;
    }

    ~Connection(){};

    void setStatus(ConnectionStatus status) { mStatus = status; }

    ConnectionStatus getStatus() { return mStatus; }

    void recordMessage(
        websocketpp::client<websocketpp::config::asio_client>::message_ptr
            message) {
        if (message->get_opcode() == websocketpp::frame::opcode::text) {
            mMessages.push_back(message->get_payload());
        } else {
            mMessages.push_back(
                websocketpp::utility::to_hex(message->get_payload()));
        }
    }

    void setHandle(websocketpp::connection_hdl handle) { mHandle = handle; }

    websocketpp::connection_hdl getHandle() { return mHandle; }

    void setId(int id) { mId = id; }

    int getId() { return mId; }

    void setEndpoint(std::string endpoint) { mEndpoint = endpoint; }

    std::string getEndpoint() { return mEndpoint; }

    void setError(std::string error) { mErrorReason = error; }

    std::string getError() { return mErrorReason; }

    void printStatus() {
        std::printf(
            " URI: %s\n Status: %s\n Remote Endpoint: %s\n Error/Close Reason: "
            "%s\n Messages Processed: %d\n",
            mUri.c_str(), getStatusString(mStatus).c_str(), mEndpoint.c_str(),
            mErrorReason.c_str(), mMessages.size());
    }

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

    void setOnOpenFn(
        std::function<
            void(std::shared_ptr<Connection> connection,
                 websocketpp::client<websocketpp::config::asio_client>* client,
                 websocketpp::connection_hdl handle)> response) {
        mOnOpenFn = response;
    }

    void callOnOpenFn(
        std::shared_ptr<Connection> connection,
        websocketpp::client<websocketpp::config::asio_client>* client,
        websocketpp::connection_hdl handle) {
        if (mOnOpenFn) {
            mOnOpenFn(connection, client, handle);
        }
    }

    void setOnFailFn(
        std::function<
            void(std::shared_ptr<Connection> connection,
                 websocketpp::client<websocketpp::config::asio_client>* client,
                 websocketpp::connection_hdl handle)> response) {
        mOnFailFn = response;
    }

    void callOnFailFn(
        std::shared_ptr<Connection> connection,
        websocketpp::client<websocketpp::config::asio_client>* client,
        websocketpp::connection_hdl handle) {
        if (mOnFailFn) {
            mOnFailFn(connection, client, handle);
        }
    }

    void setOnCloseFn(
        std::function<
            void(std::shared_ptr<Connection> connection,
                 websocketpp::client<websocketpp::config::asio_client>* client,
                 websocketpp::connection_hdl handle)> response) {
        mOnCloseFn = response;
    }

    void callOnCloseFn(
        std::shared_ptr<Connection> connection,
        websocketpp::client<websocketpp::config::asio_client>* client,
        websocketpp::connection_hdl handle) {
        if (mOnCloseFn) {
            mOnCloseFn(connection, client, handle);
        }
    }

    void setOnReceiveFn(
        std::function<void(
            websocketpp::connection_hdl handle,
            websocketpp::client<websocketpp::config::asio_client>::message_ptr
                message)> response) {
        mOnReceiveFn = response;
    }

    void callOnReceiveFn(
        websocketpp::connection_hdl handle,
        websocketpp::client<websocketpp::config::asio_client>::message_ptr
            message) {
        if (mOnReceiveFn) {
            mOnReceiveFn(handle, message);
        }
    }

   protected:
    int mId;
    websocketpp::connection_hdl mHandle;
    ConnectionStatus mStatus;
    std::string mUri;
    std::string mEndpoint;
    std::string mErrorReason;
    std::vector<std::string> mMessages;
    std::function<void(
        std::shared_ptr<Connection> connection,
        websocketpp::client<websocketpp::config::asio_client>* client,
        websocketpp::connection_hdl handle)>
        mOnOpenFn;
    std::function<void(
        std::shared_ptr<Connection> connection,
        websocketpp::client<websocketpp::config::asio_client>* client,
        websocketpp::connection_hdl handle)>
        mOnFailFn;
    std::function<void(
        std::shared_ptr<Connection> connection,
        websocketpp::client<websocketpp::config::asio_client>* client,
        websocketpp::connection_hdl handle)>
        mOnCloseFn;
    std::function<void(
        websocketpp::connection_hdl handle,
        websocketpp::client<websocketpp::config::asio_client>::message_ptr
            message)>
        mOnReceiveFn;
};
}  // namespace websocket
}  // namespace sitara
