/*
 *  gameOfLife.h
 *  conway
 *
 *  Created by Chris Roby on 3/26/10.
 *  Copyright (c) 2012 Chris Roby, 70bpm, LLC. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
//#include "ofxOsc.h"
// kinect setting
#include "ofxOpenCv.h"
#include "ofxKinect.h"

#define HOST "localhost" //送信先ホストのIPを設定
#define PORT 8000 //送信先のポート番号を設定

struct cell {
	bool currState;
	bool nextState;
	int activeNeighbors;
	ofColor color;
};


struct matchPattern {
    string name;
    int patternGrid[2]; // ポインタ指定するとこけなかった 謎
    int *pattern;
};

struct resPattern {
    string patternName;
	vector<int> x;
    vector<int> y;
};

class gameOfLife : public ofBaseApp {
    
public:
    void setup();
    void init(int width, int height, int cellsize);
    
    void tick();
    void update();
    void draw();
    void clear();

    void exit(); // kinect exit

    void patternMapping();
//    void oscSending(vector<resPattern> &datas);
    void drawingResPatterns(vector<resPattern> &datas, matchPattern &mPattern, ofColor paramsColor);
    void pause();
    
    void keyPressed(int key);
    void mousePressed(int x, int y, int button);
    int rows, cols;
    
    bool active;

//--------------------------
// kinect setting
//    void windowResized(int w, int h);
	
    //    ofxKinect kinect; //Kinect instance
	ofImage kinectImage; // Kinect camera image
    ofImage depthImage; // Kinect depth image
    ofImage threshImage;
	ofxKinect kinect;
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage grayImage01; // grayscale depth image
	ofxCvGrayscaleImage grayImage02; // grayscale depth image
	ofxCvGrayscaleImage grayImage03; // grayscale depth image
    
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
	
	ofxCvContourFinder contourFinder01;
	ofxCvContourFinder contourFinder02;
	ofxCvContourFinder contourFinder03;
	
	bool bThreshWithOpenCV;
	
	int nearThreshold01,nearThreshold02,nearThreshold03;
	int farThreshold01,farThreshold02,farThreshold03;
	int angle;
    
//--------------------------

private:
    cell **grid;
    float cellWidth, cellHeight;
    bool fullScreen, highlight;
    
    int getNumActiveNeighbors(int colIndex, int rowIndex);
    int currState(int colIndex, int rowIndex);
    
    
    void makeNextStateCurrent();
    void goFullScreen();
    
    //  ofxOscSender sender;
};
