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

#include "WebSockets.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

class exampleDDPClientApp : public ci::app::App {
   public:
    void setup() override;
    void update() override;
    void draw() override;

    ci::gl::Texture2dRef mSitaraSystemsLogo;
    ci::vec2 mLogoPosition;
    bool mAppReady;
    std::shared_ptr<sitara::websocket::DDPClient> mClient;
    int mConnectionId;
};

void exampleDDPClientApp::setup() {
    mConnectionId = -1;
    mAppReady = false;
    mSitaraSystemsLogo = ci::gl::Texture2d::create(
        loadImage(loadAsset("images/logo/sitara_systems_logo.png")));
    mLogoPosition = ci::app::getWindowCenter() -
                    0.5f * ci::vec2(mSitaraSystemsLogo->getWidth(),
                                    mSitaraSystemsLogo->getHeight());

    // Create a client endpoint
    mClient = std::make_shared<sitara::websocket::DDPClient>();
    std::string uri = "ws://localhost:3000/websocket";
    mConnectionId = mClient->connect(uri);

    mClient->setOnAddedFn([&](std::string collection, std::string documentId,
                              Json::Value fields) {
        std::cout << "Received document added : " << std::endl;
        std::cout << "    Collection : " << collection << std::endl;
        std::cout << "    Document Id : " << documentId << std::endl;
        std::cout << "    Data : " << mClient->toString(fields) << std::endl;
    });
}

void exampleDDPClientApp::update() {
    if (!mAppReady) {
        if (mClient->isConnected()) {
            mAppReady = true;
        }
    } else {
        // app is ready, regular logic goes here
        if (mClient->getSubscriptionStatus("allVisits") ==
            sitara::websocket::ddp::SubscriptionStatus::UNSUBSCRIBED) {
            mClient->subscribe("allVisits");
        }
    }
}

void exampleDDPClientApp::draw() {
    if (!mAppReady) {
        ci::gl::clear(Color(0, 0, 0));
        ci::gl::pushMatrices();
        ci::gl::translate(mLogoPosition);
        ci::gl::draw(mSitaraSystemsLogo);
        ci::gl::popMatrices();
    } else {
        // app goes here!
    }
}

CINDER_APP(exampleDDPClientApp,
           ci::app::RendererGl,
           [=](cinder::app::App::Settings* settings) {
               settings->setTitle("Sitara Systems Template Application");
               settings->setWindowSize(1920, 1080);
               settings->setConsoleWindowEnabled();
               settings->setHighDensityDisplayEnabled(false);
           });
