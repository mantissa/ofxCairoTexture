
// include all of the header files

#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( ){

	// create an opengl window
    ofAppGlutWindow window;
	
	// setup the window at a given size in normal (non-fullcreen mode )
	// use OF_FULLSCREEN if you want to change the window mode
	ofSetupOpenGL(&window, 1080, 720, OF_WINDOW);			

	// create an instance of the testApp and run it as our main application
	ofRunApp( new testApp());
}
