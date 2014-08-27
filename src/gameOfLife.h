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
#include "ofxMaxim.h"

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
  ofColor color;
  int patternGrid[2]; // ポインタ指定するとこけなかった 謎
  int *pattern;
};

struct resPattern {
  matchPattern mPattern;
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
    void drawingResPatterns(vector<resPattern> &datas);
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
    
    void kinectSetup();
    void kinectUpdate();
    void kinectDraw();
//--------------------------
	
	bool bThreshWithOpenCV;
	
	int nearThreshold01,nearThreshold02,nearThreshold03;
	int farThreshold01,farThreshold02,farThreshold03;
	int angle;
    
//--------------------------
  
  
  /************************/
  // ofxMaximまわり変数宣言 //
  /***********************/
  void audioRequested(float * output, int bufferSize, int nChannels);
  void drawBuffer();
  void audioSetup();
  float patTofreq(string patName);
  int initialBufferSize;
  int sampleRate;
  int mode;
  double wave, wave2, sample, ADSRout, outputs[2], addoutputs[2];
  ofxMaxiMix mymix;
  ofxMaxiMix mymixAdd;
  ofxMaxiOsc osc;
  double adsrEnv[6]={0, 10, 0.005, 10, 0, 20};
  double adsrAddEnv[6]={0, 100, 0.005, 3000, 0, 2000};
  //  vector <ofxMaxiOsc> oscbank;
  vector <float> lAudio;
  vector <float> rAudio;
  
  std::map<std::string, float> freqMap;
  bool audioTick = false;
  int polyNum = 30;
  ofxMaxiOsc oscbank[30];
  ofxMaxiOsc addOsc[30];
  ofxMaxiEnvelope ADSR[30];
  ofxMaxiEnvelope ADSRADD;
  ofxMaxiFilter vcFilter[30];
  int addOscCOunter = 0;

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
