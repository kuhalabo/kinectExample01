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
const int HEIGHT = 600;
//const int WIDTH = 1024;
//const int HEIGHT = 728;
const int CELLSIZE = 6;
const int FULLSCREEN_CELLSIZE = 8;
const int TICK_INTERVAL = 6;
const int FRAMERATE = 60;

///////////////////////////////////
// いくつかのグローバル変数//

patternDetect *blink1;
patternDetect *blink2;
patternDetect *glider1;
patternDetect *glider2;
patternDetect *glider3;
patternDetect *glider4;
patternDetect *line5;

vector<resPattern> datas;
vector<resPattern>::iterator resData;
///////////////////////////////////

ofImage myImage;

//------------------------
// kinect
const int SCREENRATE = 1;
// fullscreen width, height
int wfull = 640;
int hfull = 480;
//int wfull = 800;
//int hfull = 600;
int depth_min = 220;
int alphaGray = 30;
int alphaSpring = 200;
//------------------------


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
  
  audioSetup();
    
    //  sender.setup(HOST, PORT);
    
    //-----------------------------------
    // kinect
    kinectSetup();
    //------------------------------------
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
        datas.push_back(glider4->detection(grid, rows, cols));
      
        datas.push_back(line5->detection(grid, rows, cols));
        audioTick = true;
        //        oscSending(datas);
    }
    //---------------------------
    // kinect
    kinectUpdate();
    //---------------------------
    
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

    ofBackground(0, 0, 0, 0);
    //  ofEnableBlendMode(OF_BLENDMODE_ADD);
	for (int i=0; i<cols; i++) {
		for (int j=0; j<rows; j++) {
            cell thisCell = grid[i][j];
			ofSetColor(200, 200, 200);
			//ofSetColor(0, 255, 255);
			ofNoFill();
            //			ofRect(i*cellWidth, j*cellHeight, cellWidth, cellHeight);
            if (thisCell.currState == true) {
				//ofSetColor(thisCell.color.r, thisCell.color.g, thisCell.color.b, 30); // dark cell
				ofSetColor(thisCell.color.r, thisCell.color.g, thisCell.color.b, 130); // bright cell
				ofFill();
//                myImage.ofImage_::draw((float)(i*cellWidth), (float)(j*cellHeight), cellWidth*2.0, cellHeight*2.0);
                ofRect(i*cellWidth, j*cellHeight, cellWidth, cellHeight);
				ofNoFill();
			}
		}
	}

    
    if (ofGetFrameNum() % TICK_INTERVAL == 0 && active) {
      ofEnableBlendMode(OF_BLENDMODE_ALPHA);
      
      /*パターン検出インスタンスの実行メソッド*/
      drawingResPatterns(datas);
      
      /*レスデータはここでクリアする*/
      datas.clear();
    }

    //------------------------------------
    // kinect
    kinectDraw();
    //------------------------------------

}

/*::::::::::::::::::::::::
 // パターン発見時の描画まわり
 ::::::::::::::::::::::::*/
void gameOfLife::drawingResPatterns(vector<resPattern> &datas) {
  for(resData = datas.begin(); resData != datas.end(); ++resData) {
    resData->x.size();
    if (resData->x.size() != 0) {
      for (int h=0; h < resData->x.size(); h++) {
        for (int i=0; i< resData->mPattern.patternGrid[0]; i++) {
          for (int j=0; j < resData->mPattern.patternGrid[1]; j++) {
            if (resData->mPattern.pattern[resData->mPattern.patternGrid[0] * j + i ] == 1) {
              
              /*検出描画チェックログ よくつかう*/
              //              cout << resData->mPattern.name << endl;
              
              ofSetColor(resData->mPattern.color.r, resData->mPattern.color.g, resData->mPattern.color.b, 200);
              ofFill();
              myImage.ofImage_::draw((float)((i + resData->x.at(h)) * cellWidth) - cellWidth / 2, (float)((j + resData->y.at(h)) * cellHeight) - cellHeight / 2, cellWidth*3.0, cellHeight*3.0);
              //              ofRect( (i + resData->x.at(h)) * cellWidth, (j + resData->y.at(h)) * cellHeight, cellWidth, cellHeight);
              //              ofNoFill();
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
    
  int grid1[] = {1, 7};
  int grid2[] = {3, 3};
  int pat1[] = {0, 0, 0, 1, 1, 1, 0, 0, 0};
  int pat2[] = {0, 1, 0, 0, 1, 0, 0, 1, 0};
  int pat3[] = {0, 1, 1, 1, 1, 1, 0};
  
  int patGlider1[] = {0, 1, 0, 0, 0, 1, 1, 1, 1};
  int patGlider2[] = {0, 0, 1, 1, 0, 1, 0, 1, 1};
  int patGlider3[] = {1, 0, 0, 0, 1, 1, 1, 1, 0};
  int patGlider4[] = {0, 0, 1, 1, 1, 0, 0, 1, 1};
  
  blink1 = new patternDetect("blink1", grid2, pat1, ofColor::cyan);
  blink2 = new patternDetect("blink2", grid2, pat2, ofColor::cyan);
  
  glider1 = new patternDetect("glider1", grid2, patGlider1, ofColor::cyan);
  glider2 = new patternDetect("glider2", grid2, patGlider2, ofColor::cyan);
  glider3 = new patternDetect("glider3", grid2, patGlider3, ofColor::cyan);
  glider4 = new patternDetect("glider4", grid2, patGlider4, ofColor::cyan);
  
  line5 = new patternDetect("line5", grid2, pat3, ofColor::cyan);
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
        wfull = ofGetScreenWidth();
        hfull = ofGetScreenHeight();
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
            
            //-------------------------
            // for kinect by kuha
        case '1':
            patterns::glider01(grid, 100, 20);
            break;
        case '2':
            patterns::glider02(grid, 100, 10);
            break;
        case 'b':
            patterns::blinker01(grid, 50, 20);
            break;
        case 'D': // 0:far -> 255:near
            depth_min = min(depth_min + 1, 255);
            nearThreshold01 = depth_min + 20;
            farThreshold01 = depth_min + 10;
            nearThreshold02 = depth_min + 10;
            farThreshold02 = depth_min;
            break;
        case 'd':
            depth_min = max(depth_min - 1, 0);
            nearThreshold01 = depth_min + 20;
            farThreshold01 = depth_min + 10;
            nearThreshold02 = depth_min + 10;
            farThreshold02 = depth_min;
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
            //-------------------------
            
        default:
            break;
    }
}

//--------------------------------------------------------------
// for kinect setting
//------------------------------------
// kinect setup
void gameOfLife::kinectSetup() {
    // Mix color setting
    ofEnableSmoothing();
    //    glEnable(GL_BLEND);
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    //ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
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
	grayImage03.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
    
    /*
     //nearThreshold = 230;
     //farThreshold = 70;
     nearThreshold01 = 250;
     farThreshold01 = 240;
     nearThreshold02 = 240;
     farThreshold02 = 230;
     */
	nearThreshold01 = depth_min + 30;
	farThreshold01 = depth_min + 20;
	nearThreshold02 = depth_min + 20;
	farThreshold02 = depth_min + 10;
	nearThreshold03 = depth_min + 10;
	farThreshold03 = depth_min;
	bThreshWithOpenCV = true;
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);
    
}
//------------------------------------
// kinect update
void gameOfLife::kinectUpdate() {
	//ofBackground(100, 100, 100);
	//ofBackground(255, 255, 100);
	
	kinect.update();
    
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		
        //左右反転
        //colorImg.mirror(false, true);
        
        // 01 layer
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
        
        // 02 layer
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
        
        // 03 layer
        // load grayscale depth image from the kinect source
        grayImage03.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
        
        // we do two thresholds - one for the far plane and one for the near plane
        // we then do a cvAnd to get the pixels which are a union of the two thresholds
        if(bThreshWithOpenCV) {
            grayThreshNear = grayImage03;
            grayThreshFar = grayImage03;
            grayThreshNear.threshold(nearThreshold03, true);
            grayThreshFar.threshold(farThreshold03);
            cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage03.getCvImage(), NULL);
        } else {
            
            // or we do it ourselves - show people how they can work with the pixels
            unsigned char * pix = grayImage03.getPixels();
            
            int numPixels = grayImage03.getWidth() * grayImage03.getHeight();
            for(int i = 0; i < numPixels; i++) {
                if(pix[i] < nearThreshold03 && pix[i] > farThreshold03) {
                    pix[i] = 255;
                    //pix[i] = 25;
                } else {
                    pix[i] = 0;
                    //pix[i] = 23;
                }
            }
        }
        
		// update the cv images
        grayImage03.flagImageChanged();
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
        contourFinder03.findContours(grayImage03, 10, (kinect.width*kinect.height)/2, 20, false);
        
    }
}
//---------------------
// kinect draw
void gameOfLife::kinectDraw() {
	ofSetColor(255, 255, 255);
    //    ofBackground(0, 0, 0, 0);
	
    //kinect.draw(0, 0, kinect.width, kinect.height);
    //kinect.drawDepth(kinect.width, 0, kinect.width, kinect.height);
    
    // draw from the live kinect
    if (!fullScreen) {
        kinect.drawDepth(10 + 256 * SCREENRATE, 10, 400, 300);
        kinect.draw(420 + 256 * SCREENRATE, 10, 400, 300);
        
        ofSetColor(255, 0, 0, 100);
        grayImage01.draw(10 + 256 * SCREENRATE, 320, 400, 300);
        //ofSetColor(255, 255, 0, 100);
        //contourFinder01.draw(10 + 256 * SCREENRATE, 320, 400, 300);
        
        ofSetColor(0, 255, 0, 100);
        grayImage02.draw(10 + 256 * SCREENRATE, 320, 400, 300);
        //ofSetColor(0, 255, 255, 100);
        //contourFinder02.draw(10 + 256 * SCREENRATE, 320, 400, 300);
        
        ofSetColor(0, 0, 255, 100);
        grayImage03.draw(10 + 256 * SCREENRATE, 320, 400, 300);
        //ofSetColor(0, 0, 255, 100);
        //contourFinder03.draw(10 + 256 * SCREENRATE, 320, 400, 300);
        
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
            //ofDrawBitmapString(reportStream.str(), 20, 652);
            ofDrawBitmapString(reportStream.str(), 20, 400);
        }
    }
    else{
        int xCell, yCell;
        
        ofSetColor(255, 0, 0, alphaGray);
        grayImage01.draw(0, 0, wfull, hfull);
        //        ofSetColor(255, 0, 0, 100);
        //        contourFinder01.draw(0, 0, wfull, hfull);
        // draw Centorid of contour01
        int centroX01;
        int centroY01;
        ofSetColor(255, 0, 255, alphaSpring);
        ofSetLineWidth(4);
        for( int i = 0; i < 3; i++){
            centroX01 = contourFinder01.blobs[i].centroid.x * wfull / WIDTH * 1.25;
            centroY01 = contourFinder01.blobs[i].centroid.y * hfull / HEIGHT * 1.25;
            if(centroX01 > 0 && centroX01 < wfull && centroY01 > 0 && centroY01 < hfull){
                xCell = centroX01 * cols / wfull;
                yCell = centroY01 * rows / hfull;
                ofCircle(centroX01, centroY01, 30); // Centroid draw
                if (ofGetFrameNum() % (TICK_INTERVAL * 6) == 0 && active) {
                    //patterns::blinker01(grid, xCell, yCell);
                    patterns::blinker01(grid, xCell, yCell);
                }
            }
        }
        ofSetLineWidth(1);
        
        ofSetColor(0, 255, 0, alphaGray);
        grayImage02.draw(0, 0, wfull, hfull);
        //        ofSetColor(0, 255, 0, 50);
        //        contourFinder02.draw(0, 0, wfull, hfull);
        // draw Centorid of contour02
        int centroX02;
        int centroY02;
        ofSetColor(255, 255, 0, alphaSpring);
        ofSetLineWidth(4);
        for( int i = 0; i < 3; i++){
            centroX02 = contourFinder02.blobs[i].centroid.x * wfull / WIDTH * 1.25;
            centroY02 = contourFinder02.blobs[i].centroid.y * hfull / HEIGHT * 1.25;
            if(centroX02 > 0 && centroX02 < wfull && centroY02 > 0 && centroY02 < hfull){
                xCell = centroX02 * cols / wfull;
                yCell = centroY02 * rows / hfull;
                ofCircle(centroX02, centroY02, 30); // Centroid draw
                if (ofGetFrameNum() % (TICK_INTERVAL * 6) == 0 && active) {
                    //patterns::blinker01(grid, xCell, yCell);
                    patterns::glider01(grid, xCell, yCell);
                }
            }
        }
        ofSetLineWidth(1);
        
        ofSetColor(0, 0, 255, alphaGray);
        grayImage03.draw(0, 0, wfull, hfull);
        //        ofSetColor(0, 255, 0, 50);
        //        contourFinder02.draw(0, 0, wfull, hfull);
        // draw Centorid of contour02
        int centroX03;
        int centroY03;
        ofSetColor(0, 255, 255, alphaSpring);
        ofSetLineWidth(4);
        for( int i = 0; i < 3; i++){
            centroX03 = contourFinder03.blobs[i].centroid.x * wfull / WIDTH * 1.25;
            centroY03 = contourFinder03.blobs[i].centroid.y * hfull / HEIGHT * 1.25;
            if(centroX03 > 0 && centroX03 < wfull && centroY03 > 0 && centroY03 < hfull){
                xCell = centroX03 * cols / wfull;
                yCell = centroY03 * rows / hfull;
                ofCircle(centroX03, centroY03, 30); // Centroid draw
                if (ofGetFrameNum() % (TICK_INTERVAL * 6) == 0 && active) {
                    //patterns::blinker01(grid, xCell, yCell);
                    patterns::glider01(grid, xCell, yCell);
                }
            }
        }
        ofSetLineWidth(1);
        
        stringstream reportScreen;
        reportScreen << "wfull=" << wfull << ",hfull=" << hfull << ", cols=" << cols << ",rows=" << rows << endl
        << "centroX02=" << centroX02 << ",centroY02=" << centroY02 << endl
        << "xCell=" << xCell << ",yCell=" << yCell << endl
        << "depth_min=" << depth_min << ", frame number=" << ofGetFrameNum() << endl
        << endl;
        ofSetColor(255, 255, 255);
        ofDrawBitmapString(reportScreen.str(), 20, 100);
        
    }
}
//--------------------------------------------------------------
// kinect exit
void gameOfLife::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}


//--------------------------------------------------------------



/*****************************
 * オーディオ出力
 *****************************/

void gameOfLife::audioSetup() {
  
  sampleRate 			= 44100; /* Sampling Rate */
	initialBufferSize	= 512;	/* Buffer Size. you have to fill this buffer with sound*/
	
	ofSoundStreamSetup(2,0,this, sampleRate, initialBufferSize, 4);
  
  lAudio.assign(initialBufferSize, 0.0);
	rAudio.assign(initialBufferSize, 0.0);
  
  /*frequencyのマッピング*/
  freqMap["blink1"] = 493.883301;
  freqMap["blink2"] = 329.627563;
  
  freqMap["glider1"] = 277.182617;
  freqMap["glider2"] = 369.994415;
  freqMap["glider3"] = 415.304688;
  freqMap["glider4"] = 440.0;
  
  freqMap["line5"] = 554.365234;
  
  mode = 0;
  
}

void gameOfLife::audioOut(float *output, int bufferSize, int nChannels) {
  float currentTone[polyNum];
  float currentAdditiveTone[polyNum];
  int currentX[polyNum]; // 発音数に応じて得れる要素数を制限する
  int currentY[polyNum]; // 発音数に応じて得れる要素数を制限する
  
  for (int i = 0; i < bufferSize; i++) {
    //    for(resData = datas.begin(); resData != datas.end(); ++resData) {
    //      float career = patTofreq(resData->mPattern.name);
    //      for(int i = 0; i < resData->x.size(); i++) {
    //        wave = osc.sinewave(career) * resData->y[i] * 0.01;
    //        float pan = 1 / resData->x[i];
    //        mymix.stereo(wave, outputs, pan);
    //      }
    
    //
    //      lAudio[i] = output[i * nChannels] = outputs[0];
    //      rAudio[i] = output[i * nChannels + 1] = outputs[1];
    //    }
    
    if (audioTick == true) {
      wave = 0;
      
      for(int j = 0; j < polyNum; j ++ ) {
        ADSR[j].trigger(0, adsrEnv[0]);
        currentTone[j] = 0;
      }
      
      for (int k = 0; k < 7; k ++ ) {
        for(int l = 0; l < datas[k].x.size(); l ++ ) {
          float career = patTofreq(datas[k].mPattern.name);
          
          /*決めウチのオシレーター数を超えないようのif文*/
          if ((k * l) + l < polyNum) {
            
            currentTone[(k * l) + l] = career;
            // ちょっと後で原因究明したいが、いったん変な値を汚く間引く
            currentX[(k * l) + l] = datas[k].x[l] > 0 && datas[k].x[l] < 2000 ? datas[k].x[l] : 0;
            currentY[(k * l) + l] = datas[k].y[l] > 0 && datas[k].y[l] < 2000 ? datas[k].y[l] : 0;
          }
        }
      }
      
      //    float career = patTofreq(resData->mPattern.name);
      addOscCOunter ++;
      audioTick = false;
    }
    
    if (addOscCOunter % 43 == 42) {
      for (int i = 0; i < polyNum; i ++ ) {
        currentAdditiveTone[i] = currentTone[i] + (rand() % 10) ;
      }
      addOscCOunter = 0;
    }
    
    for(int m = 0; m < polyNum; m ++ ) {
      ADSRout = ADSR[m].line(6, adsrEnv);
      if (currentTone[m] != 0) {
        wave += oscbank[m].square(currentTone[m]);
        mymix.stereo(wave * ADSRout, outputs, ( 1 / ((float)(currentX[m]) ) / (float)(ofGetWidth())) );
      }
      
      if (currentAdditiveTone[m] != 0) {
        wave2 += addOsc[m].sinewave(currentAdditiveTone[m]) * 0.001;
      }
    }
    
    //    mymix.stereo(wave2, outputs, 0.5);
    lAudio[i] = output[i * nChannels] = outputs[0] + wave2;
    rAudio[i] = output[i * nChannels + 1] = outputs[1] +wave2;
  }
}

float gameOfLife::patTofreq(string patName) {
  if (freqMap.find(patName) == freqMap.end()) {
    // not found
    
    return 0.0;
  } else {
    // found
    
    return freqMap[patName];
  }
}
