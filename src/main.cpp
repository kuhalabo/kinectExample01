#include "ofApp.h"
#include "gameOfLife.h"
#include "ofAppGlutWindow.h"

int main() {
    //ofSetupOpenGL(1024, 768, OF_WINDOW);

    ofAppGlutWindow window;
    ofSetupOpenGL(&window, 640*2, 480, OF_WINDOW);
    //ofSetupOpenGL(&window, 1024*2, 768, OF_WINDOW);

	//ofRunApp(new ofApp());
	ofRunApp(new gameOfLife());
}
