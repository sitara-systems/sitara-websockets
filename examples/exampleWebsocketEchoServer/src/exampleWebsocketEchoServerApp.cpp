#include "Websockets.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

class exampleWebsocketEchoServerApp : public ci::app::App {
   public:
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void update() override;
    void draw() override;

    std::shared_ptr<sitara::websocket::Server> mServer;
    int mConnectionId;
};

void exampleWebsocketEchoServerApp::setup() {
    ci::app::setFrameRate(1);
    mServer = std::make_shared<sitara::websocket::Server>(9002);
    mServer->addOnReceiveFn(
        [&](websocketpp::connection_hdl handle,
            websocketpp::server<websocketpp::config::asio>::message_ptr
                message) {
            std::printf("Received message : %s\n",
                        message->get_payload().c_str());
            auto c = mServer->getConnection(handle);
            mServer->send(c->getId(), message);
        });
    mServer->run();
}

void exampleWebsocketEchoServerApp::mouseDown(MouseEvent event) {}

void exampleWebsocketEchoServerApp::update() {}

void exampleWebsocketEchoServerApp::draw() {
    ci::gl::clear(Color(0, 0, 0));
}

CINDER_APP(exampleWebsocketEchoServerApp,
           ci::app::RendererGl,
           [=](cinder::app::App::Settings* settings) {
               settings->setConsoleWindowEnabled();
           })
