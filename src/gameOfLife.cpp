/*
 *  gameOfLife.cpp
 *  conway
 *
 *  Created by Chris Roby on 3/26/10.
 *  Copyright (c) 2012 Chris Roby. All rights reserved.
 *
 */

#include "gameOfLife.h"
#include "patterns.h"
#include "patternDetect.h"


const int WIDTH = 800;
const int HEIGHT = 500;
const int CELLSIZE = 6;
const int FULLSCREEN_CELLSIZE = 8;
const int TICK_INTERVAL = 6;
const int FRAMERATE = 30;

/*//////////////////////////////////*/
// いくつかのグローバル変数//

patternDetect *blink1;
patternDetect *blink2;
patternDetect *glider1;
patternDetect *glider2;
patternDetect *glider3;

vector<resPattern> datas;
vector<resPattern>::iterator resData;

// by kuha for screen size 256 * x
const int SCREENRATE = 2;

/*//////////////////////////////////*/

ofImage myImage;

void gameOfLife::setup() {
    fullScreen = false;
    highlight = false;
    active = false;
    
    ofSetFullscreen(false);
    ofSetWindowShape(WIDTH, HEIGHT);
    
    init(WIDTH, HEIGHT, CELLSIZE);
    
	ofBackground(ofColor::white);
	ofSetBackgroundAuto(true);
	ofSetWindowTitle("Conway's Game of Life");
	ofSetFrameRate(FRAMERATE);
    
    myImage.loadImage("circleAlpha.tif");
    
    //  sender.setup(HOST, PORT);
    
    //------------------------------------
    // kinect setting
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
	//ofSetFrameRate(30);
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);
    
}

void gameOfLife::init(int width, int height, int cellSize) {
	cols = width/cellSize;
	rows = height/cellSize;
	
	if (width % cellSize != 0 || (height & cellSize) != 0) {
		float ratio = width/height;
		cellWidth = cellSize * ratio;
		cellHeight = cellSize;
	} else {
		cellWidth = cellSize;
		cellHeight = cellSize;
	}
    
	// set up grid
	clear();
    patternMapping();
}

void gameOfLife::update() {
    if (ofGetFrameNum() % TICK_INTERVAL == 0 && active) {
        tick();
        datas.push_back(blink1->detection(grid, rows, cols));
        datas.push_back(blink2->detection(grid, rows, cols));
        datas.push_back(glider1->detection(grid, rows, cols));
        datas.push_back(glider2->detection(grid, rows, cols));
        datas.push_back(glider3->detection(grid, rows, cols));
//        oscSending(datas);
    }
//----------------------------
// kinect update
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

/*今は参照渡し風に書いている　複数のインスタンスを渡してバグが生まれたら対応*/
/*
void gameOfLife::oscSending(vector<resPattern> &datas) {
    for(resData = datas.begin(); resData != datas.end(); ++resData) {
        std::stringstream result_x, result_y;
        std::copy(&*resData->x.begin(), &*resData->x.end(), std::ostream_iterator<int>(result_x, ","));
        std::copy(&*resData->y.begin(), &*resData->y.end(), std::ostream_iterator<int>(result_y, ","));
         ofxOscMessage mx, my;
         string textName = "/";
         textName += resData->patternName;
         textName += "/";
         string textX = textName + "x";
         string textY = textName + "y";
         
         mx.setAddress(textX);
         mx.addStringArg( result_x.str() );
         
         my.setAddress( textY );
         my.addStringArg( result_y.str() );
         
         //メッセージを送信
         sender.sendMessage( mx );
         sender.sendMessage( my );
    };
}
*/

void gameOfLife::tick() {
	// get active neighbors for each cell
    
	for (int i=0; i<cols; i++) {
        for (int j=0; j<rows; j++) {
            cell *thisCell = &grid[i][j];
            thisCell->activeNeighbors = getNumActiveNeighbors(i, j);
            bool currState = thisCell->currState;
            int activeNeighbors = thisCell->activeNeighbors;
            if (currState == true && activeNeighbors < 2) {
                thisCell->nextState = false;
            } else if (currState == true && activeNeighbors > 3) {
                thisCell->nextState = false;
            } else if (currState == true && activeNeighbors > 1 && activeNeighbors < 4) {
                thisCell->nextState = true;
                thisCell->color = ofColor::white;
            } else if (currState == false && activeNeighbors == 3) {
                thisCell->nextState = true;
                thisCell->color = highlight ? ofColor::green : ofColor::white;
            }
        }
	}
	makeNextStateCurrent();
}

void gameOfLife::makeNextStateCurrent() {
	for (int i=0; i<cols; i++) {
		for (int j=0; j<rows; j++) {
			grid[i][j].currState = grid[i][j].nextState;
		}
	}
}

void gameOfLife::draw() {
    ofBackground(0, 0, 0);
    //  ofEnableBlendMode(OF_BLENDMODE_ADD);
	for (int i=0; i<cols; i++) {
		for (int j=0; j<rows; j++) {
            cell thisCell = grid[i][j];
			ofSetColor(200, 200, 200);
			ofNoFill();
            //			ofRect(i*cellWidth, j*cellHeight, cellWidth, cellHeight);
            if (thisCell.currState == true) {
				ofSetColor(thisCell.color.r, thisCell.color.g, thisCell.color.b, 30);
				ofFill();
                myImage.ofImage_::draw((float)(i*cellWidth), (float)(j*cellHeight), cellWidth*2.0, cellHeight*2.0);
                ofRect(i*cellWidth, j*cellHeight, cellWidth, cellHeight);
				ofNoFill();
			}
		}
	}
    /*パターン検出インスタンスの実行メソッド*/
    drawingResPatterns(datas, blink1->mPattern, ofColor::green);
    drawingResPatterns(datas, blink2->mPattern, ofColor::blue);
    //  drawingResPatterns(datas, glider1->mPattern, ofColor::red);
    //  drawingResPatterns(datas, glider2->mPattern, ofColor::orange);
    //  drawingResPatterns(datas, glider3->mPattern, ofColor::yellowGreen);
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
    if (ofGetFrameNum() % TICK_INTERVAL == 0 && active) {
        /*レスデータはここでクリアする*/
        datas.clear();
    }
    
    //------------------------------------
    // kinect draw
	ofSetColor(255, 255, 255);
	
    //kinect.draw(0, 0, kinect.width, kinect.height);
    //kinect.drawDepth(kinect.width, 0, kinect.width, kinect.height);
    
    // draw from the live kinect
    kinect.drawDepth(10 + 256 * SCREENRATE, 10, 400, 300);
    kinect.draw(420 + 256 * SCREENRATE, 10, 400, 300);
    
    ofSetColor(255, 0, 0, 50);
    grayImage01.draw(10 + 256 * SCREENRATE, 320, 400, 300);
    ofSetColor(255, 255, 0, 100);
    contourFinder01.draw(10 + 256 * SCREENRATE, 320, 400, 300);
    
    ofSetColor(0, 0, 255, 50);
    //grayImage02.draw(420, 320, 400, 300);
    grayImage02.draw(10 + 256 * SCREENRATE, 320, 400, 300);
    ofSetColor(0, 255, 255, 100);
    contourFinder02.draw(10 + 256 * SCREENRATE, 320, 400, 300);
	
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

/*::::::::::::::::::::::::
 // パターン発見時の描画まわり
 ::::::::::::::::::::::::*/
void gameOfLife::drawingResPatterns(vector<resPattern> &datas, matchPattern &mPattern, ofColor paramsColor) {
    for(resData = datas.begin(); resData != datas.end(); ++resData) {
        resData->x.size();
        if (resData->x.size() != 0) {
            for (int h=0; h < resData->x.size(); h++) {
                for (int i=0; i< mPattern.patternGrid[0]; i++) {
                    for (int j=0; j < mPattern.patternGrid[1]; j++) {
                        
                        
                        if (mPattern.pattern[mPattern.patternGrid[0] * i + j ] == 1) {
                            ofSetColor(paramsColor.r, paramsColor.g, paramsColor.b, 100);
                            ofFill();
                                          myImage.ofImage_::draw((float)((i + resData->x.at(h)) * cellWidth), (float)((j + resData->y.at(h)) * cellHeight), cellWidth*3.0, cellHeight*3.0);
                            ofRect( (i + resData->x.at(h)) * cellWidth, (j + resData->y.at(h)) * cellHeight, cellWidth, cellHeight);
                            ofNoFill();
                        }
                    }
                }
            }
        }
    }
}


void gameOfLife::clear() {
	grid = new cell *[cols];
	for (int i=0; i<cols; i++) {
		grid[i] = new cell[rows];
		for (int j=0; j<rows; j++) {
            cell *thisCell = &grid[i][j];
			thisCell->currState = false;
			thisCell->nextState = false;
			thisCell->color = ofColor::black;
		}
	}
}

/*****************************
 * 検出パターンを初期化するメソッド
 *****************************/
void gameOfLife::patternMapping() {
    
    int grid1[] = {1, 5};
    int grid2[] = {3, 3};
    int pat1[] = {0, 0, 0, 1, 1, 1, 0, 0, 0};
    int pat2[] = {0, 1, 0, 0, 1, 0, 0, 1, 0};
    
    int patGlider1[] = {0, 1, 0, 0, 0, 1, 1, 1, 1};
    int patGlider2[] = {0, 0, 1, 1, 0, 1, 0, 1, 1};
    int patGlider3[] = {1, 0, 0, 0, 1, 1, 1, 1, 0};
    blink1 = new patternDetect("blink1", grid2, pat1);
    blink2 = new patternDetect("blink2", grid2, pat2);
    glider1 = new patternDetect("glider1", grid2, patGlider1);
    glider2 = new patternDetect("glider2", grid2, patGlider2);
    glider3 = new patternDetect("glider3", grid2, patGlider3);
}


/**
 * Ensure it is a valid col/row combo (on grid) and
 * that this cell's currState is true
 */
int gameOfLife::currState(int col, int row) {
    return (col >= 0 && row >= 0 &&
            col < cols && row < rows &&
            grid[col][row].currState == true) ? 1 : 0;
}

/**
 * Checks for the number of neighbors that are in an active state
 */
int gameOfLife::getNumActiveNeighbors(int colIndex, int rowIndex) {
    int ret = 0;
    
    int prevCol = colIndex-1;
    int nextCol = colIndex+1;
    int prevRow = rowIndex-1;
    int nextRow = rowIndex+1;
    
    // by kuha for torus topology
    if(prevCol < 0) prevCol = cols - 1;
    if(nextCol >= cols) nextCol = 0;
    if(prevRow < 0) prevRow = rows - 1;
    if(nextRow >= rows) nextRow = 0;
    
    ret += currState(prevCol, prevRow);
    ret += currState(prevCol, rowIndex);
    ret += currState(prevCol, nextRow);
    
    ret += currState(colIndex, prevRow);
    ret += currState(colIndex, nextRow);
    
    ret += currState(nextCol, prevRow);
    ret += currState(nextCol, rowIndex);
    ret += currState(nextCol, nextRow);
    
    return ret;
}


void gameOfLife::goFullScreen() {
    active = false;
    ofToggleFullscreen();
    fullScreen = !fullScreen;
    if (fullScreen) {
        init(ofGetScreenWidth(), ofGetScreenHeight(), FULLSCREEN_CELLSIZE);
    } else {
        init(WIDTH, HEIGHT, CELLSIZE);
    }
}

void gameOfLife::pause() {
    active = false;
    clear();
}

void gameOfLife::mousePressed(int x, int y, int button) {
    int xcell = x/cellWidth;
	int ycell = y/cellHeight;
	grid[xcell][ycell].currState = !grid[xcell][ycell].currState;
}

void gameOfLife::keyPressed(int key) {
    switch (key) {
        case ' ':
            active = !active;
            break;
        case 'c':
            clear();
            break;
        case 'f':
            goFullScreen();
            break;
        case 'h':
            highlight = !highlight;
            break;
        case 'g':
            //      pause();
            patterns::gliderGun(grid, 2, 2);
            break;
        case 'p':
            //      pause();
            patterns::pufferTrain(grid, cols/2 - 10, rows - 30);
            break;
            
            // by kuha
        case '1':
            patterns::glider01(grid, 100, 20);
            break;
        case '2':
            patterns::glider02(grid, 100, 10);
            break;
        case 'b':
            patterns::blinker01(grid, 50, 20);
            break;
            
            //-------------------------
            // kinect
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
            
            
            
            
        default:
            break;
    }
}

//--------------------------------------------------------------
// for kinect setting

void gameOfLife::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}

//--------------------------------------------------------------
