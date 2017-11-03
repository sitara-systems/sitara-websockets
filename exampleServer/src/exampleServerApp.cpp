#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include <iostream>
#include "Server.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class exampleServerApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	std::shared_ptr<midnight::websocket::Server> mServer;
};

void exampleServerApp::setup() {
	mServer = std::make_shared<midnight::websocket::Server>(9002);
	//mServer->addOnReceiveFn([](std::string message) {
	//	std::printf("received message %s", message.c_str());
	//});
	mServer->run();
}

void exampleServerApp::mouseDown( MouseEvent event )
{
}

void exampleServerApp::update()
{
}

void exampleServerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( exampleServerApp, RendererGl, [=](cinder::app::App::Settings* settings) {settings->setConsoleWindowEnabled(); })
