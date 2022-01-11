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
