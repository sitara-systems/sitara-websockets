#include "WebSockets.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

class exampleWebsocketClientApp : public ci::app::App {
   public:
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void update() override;
    void draw() override;

    std::shared_ptr<sitara::websocket::Client> mClient;
    int mConnectionId;
};

void exampleWebsocketClientApp::setup() {
    ci::app::setFrameRate(1);
    // Create a client endpoint
    mClient = std::make_shared<sitara::websocket::Client>();
    // std::string uri = "ws://localhost:3000/websocket";
    std::string uri = "ws://echo.websocket.org/";
    mConnectionId = mClient->connect(uri);
    std::shared_ptr<sitara::websocket::Connection> connection =
        mClient->getConnection(mConnectionId);

    connection->setOnReceiveFn(
        [&](websocketpp::connection_hdl handle,
            websocketpp::client<websocketpp::config::asio_client>::message_ptr
                message) {
            std::printf("Received message!\n");
            std::printf("Connection Status:\n");
            auto c = mClient->getConnection(handle);
            c->printStatus();
            std::printf("Message : %s\n\n", message->get_payload().c_str());
        });
}

void exampleWebsocketClientApp::mouseDown(MouseEvent event) {}

void exampleWebsocketClientApp::update() {
    if (static_cast<int>(ci::app::getElapsedSeconds()) % 2 == 0) {
        std::shared_ptr<sitara::websocket::Connection> connection =
            mClient->getConnection(mConnectionId);
        if (connection->getStatus() == sitara::websocket::OPEN) {
            std::printf("Sending message...\n");
            std::string msg =
                "Hello World " + std::to_string(ci::app::getElapsedSeconds());
            mClient->send(mConnectionId, msg);
        }
    }
}

void exampleWebsocketClientApp::draw() {
    ci::gl::clear(Color(0, 0, 0));
}

CINDER_APP(exampleWebsocketClientApp,
           ci::app::RendererGl,
           [=](cinder::app::App::Settings* settings) {
               settings->setConsoleWindowEnabled();
           })
