#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	// enable depth->video image calibration
	kinect.setRegistration(true);
    
	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)
	
	kinect.open();		// opens first available kinect
	//kinect.open(1);	// open a kinect by id, starting with 0 (sorted by serial # lexicographically))
	//kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #
	
	// print the intrinsic IR sensor values
	if(kinect.isConnected()) {
		ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage01.allocate(kinect.width, kinect.height);
	grayImage02.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	
	//nearThreshold = 230;
	//farThreshold = 70;
	nearThreshold01 = 250;
	farThreshold01 = 240;
	nearThreshold02 = 240;
	farThreshold02 = 230;
	bThreshWithOpenCV = true;
	
	//ofSetFrameRate(60);
	ofSetFrameRate(30);
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);
}

//--------------------------------------------------------------
void ofApp::update() {
	
	ofBackground(100, 100, 100);
	
	kinect.update();
    
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		
		// load grayscale depth image from the kinect source
		grayImage01.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds
		if(bThreshWithOpenCV) {
			grayThreshNear = grayImage01;
			grayThreshFar = grayImage01;
			grayThreshNear.threshold(nearThreshold01, true);
			grayThreshFar.threshold(farThreshold01);
			cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage01.getCvImage(), NULL);
		} else {
			
			// or we do it ourselves - show people how they can work with the pixels
			unsigned char * pix = grayImage01.getPixels();
			
			int numPixels = grayImage01.getWidth() * grayImage01.getHeight();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] < nearThreshold01 && pix[i] > farThreshold01) {
					pix[i] = 255;
					//pix[i] = 25;
				} else {
					pix[i] = 0;
					//pix[i] = 23;
				}
			}
		}
		// update the cv images
		grayImage01.flagImageChanged();
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder01.findContours(grayImage01, 10, (kinect.width*kinect.height)/2, 20, false);
    }
    
    if(kinect.isFrameNew()) {
        
        // load grayscale depth image from the kinect source
        grayImage02.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
        
        // we do two thresholds - one for the far plane and one for the near plane
        // we then do a cvAnd to get the pixels which are a union of the two thresholds
        if(bThreshWithOpenCV) {
            grayThreshNear = grayImage02;
            grayThreshFar = grayImage02;
            grayThreshNear.threshold(nearThreshold02, true);
            grayThreshFar.threshold(farThreshold02);
            cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage02.getCvImage(), NULL);
        } else {
            
            // or we do it ourselves - show people how they can work with the pixels
            unsigned char * pix = grayImage02.getPixels();
            
            int numPixels = grayImage02.getWidth() * grayImage02.getHeight();
            for(int i = 0; i < numPixels; i++) {
                if(pix[i] < nearThreshold02 && pix[i] > farThreshold02) {
                    pix[i] = 255;
                    //pix[i] = 25;
                } else {
                    pix[i] = 0;
                    //pix[i] = 23;
                }
            }
        }
        
		// update the cv images
        grayImage02.flagImageChanged();
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
        contourFinder02.findContours(grayImage02, 10, (kinect.width*kinect.height)/2, 20, false);
	}
	
}

//--------------------------------------------------------------
void ofApp::draw() {
	
	ofSetColor(255, 255, 255);
	
    //kinect.draw(0, 0, kinect.width, kinect.height);
    //kinect.drawDepth(kinect.width, 0, kinect.width, kinect.height);
    
    // draw from the live kinect
    kinect.drawDepth(10 + 1024, 10, 400, 300);
    kinect.draw(420 + 1024, 10, 400, 300);
    
    ofSetColor(255, 0, 0, 50);
    grayImage01.draw(10 + 1024, 320, 400, 300);
    ofSetColor(255, 255, 0, 100);
    contourFinder01.draw(10 + 1024, 320, 400, 300);
    
    ofSetColor(0, 0, 255, 50);
    //grayImage02.draw(420, 320, 400, 300);
    grayImage02.draw(10 + 1024, 320, 400, 300);
    ofSetColor(0, 255, 255, 100);
    contourFinder02.draw(10 + 1024, 320, 400, 300);
	
	// draw instructions
	ofSetColor(255, 255, 255);
	//ofSetColor(255, 255, 0);
	stringstream reportStream;
    
    if(kinect.hasAccelControl()) {
        reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
        << ofToString(kinect.getMksAccel().y, 2) << " / "
        << ofToString(kinect.getMksAccel().z, 2) << endl;
    } else {
        reportStream << "Note: this is a newer Xbox Kinect or Kinect For Windows device," << endl
		<< "motor / led / accel controls are not currently supported" << endl << endl;
    }
    
	reportStream << "press p to switch between images and point cloud, rotate the point cloud with the mouse" << endl
	<< "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
	<< "set near threshold " << nearThreshold01 << " (press: + -)" << endl
	<< "set far threshold " << farThreshold01 << " (press: < >) num blobs found " << contourFinder01.nBlobs
	<< ", fps: " << ofGetFrameRate() << endl
	<< "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl;
    
    if(kinect.hasCamTiltControl()) {
    	reportStream << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl
        << "press 1-5 & 0 to change the led mode" << endl;
    }
    
	ofDrawBitmapString(reportStream.str(), 20, 652);
    
}
//--------------------------------------------------------------
void ofApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
	switch (key) {
		case ' ':
			//bThreshWithOpenCV = !bThreshWithOpenCV;
			break;
			
		case '>':
		case '.':
			farThreshold01 ++;
			if (farThreshold01 > 255) farThreshold01 = 255;
			break;
			
		case '<':
		case ',':
			farThreshold01 --;
			if (farThreshold01 < 0) farThreshold01 = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold01 ++;
			if (nearThreshold01 > 255) nearThreshold01 = 255;
			break;
			
		case '-':
			nearThreshold01 --;
			if (nearThreshold01 < 0) nearThreshold01 = 0;
			break;
			
		case 'w':
			//kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
			
		case 'o':
			kinect.setCameraTiltAngle(angle); // go back to prev tilt
			kinect.open();
			break;
			
		case 'c':
			//kinect.setCameraTiltAngle(0); // zero the tilt
			//kinect.close();
			break;
			
		case '1':
			kinect.setLed(ofxKinect::LED_GREEN);
			break;
			
		case '2':
			kinect.setLed(ofxKinect::LED_YELLOW);
			break;
			
		case '3':
			kinect.setLed(ofxKinect::LED_RED);
			break;
			
		case '4':
			kinect.setLed(ofxKinect::LED_BLINK_GREEN);
			break;
			
		case '5':
			kinect.setLed(ofxKinect::LED_BLINK_YELLOW_RED);
			break;
			
		case '0':
			kinect.setLed(ofxKinect::LED_OFF);
			break;
			
		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;
			
		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{}
