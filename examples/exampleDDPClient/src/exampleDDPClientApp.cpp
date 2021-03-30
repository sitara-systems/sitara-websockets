#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "WebSockets.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class exampleDDPClientApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;

	gl::Texture2dRef mSitaraSystemsLogo;
	ci::vec2 mLogoPosition;
	bool mAppReady;
	std::shared_ptr<sitara::websocket::DDPClient> mClient;
	int mConnectionId;
};

void exampleDDPClientApp::setup() {
	mConnectionId = -1;
	mAppReady = false;
	mSitaraSystemsLogo = ci::gl::Texture2d::create(loadImage(loadAsset("images/logo/sitara_systems_logo.png")));
	mLogoPosition = ci::app::getWindowCenter() - 0.5f*ci::vec2(mSitaraSystemsLogo->getWidth(), mSitaraSystemsLogo->getHeight());

	// Create a client endpoint
	mClient = std::make_shared<sitara::websocket::DDPClient>();
	std::string uri = "ws://localhost:3000/websocket";
	mConnectionId = mClient->connect(uri);

	mClient->setOnAddedFn([&](std::string collection, std::string documentId, Json::Value fields) {
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
  }
  else {
	  // app is ready, regular logic goes here
	  if (mClient->getSubscriptionStatus("allVisits") == sitara::websocket::ddp::SubscriptionStatus::UNSUBSCRIBED) {
		  mClient->subscribe("allVisits");
	  }
  }
}

void exampleDDPClientApp::draw() {
  if (!mAppReady) {
		gl::clear(Color(0, 0, 0));
		gl::pushMatrices();
		gl::translate(mLogoPosition);
		gl::draw(mSitaraSystemsLogo);
		gl::popMatrices();
	}
	else {
		// app goes here!
	}
}

CINDER_APP(exampleDDPClientApp, RendererGl, [=](cinder::app::App::Settings* settings) {
	settings->setTitle("Sitara Systems Template Application");
	settings->setWindowSize(1920, 1080);
	settings->setConsoleWindowEnabled();
	settings->setHighDensityDisplayEnabled(false);
	});
