#ifndef _WEBSOCKETPP_CPP11_STL_

#define _WEBSOCKETPP_CPP11_STL_
#endif

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include <iostream>
#include "Websockets.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class exampleClientApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	std::shared_ptr<midnight::websocket::Client> mClient;
};

void exampleClientApp::setup() {
	// Create a client endpoint
	mClient = std::make_shared<midnight::websocket::Client>();
	std::string uri = "ws://localhost:9002";
	mClient->connect(uri);
}

void exampleClientApp::mouseDown( MouseEvent event )
{
}

void exampleClientApp::update() {
	if (ci::app::getElapsedFrames() % 5 == 0) {
		mClient->send(0, "Hello World\n");
	}
}

void exampleClientApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( exampleClientApp, RendererGl, [=](cinder::app::App::Settings* settings) {settings->setConsoleWindowEnabled(); })
