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
const int FRAMERATE = 60;

/*//////////////////////////////////*/
    // いくつかのグローバル変数//

patternDetect *blink1;
patternDetect *blink2;
patternDetect *glider1;
patternDetect *glider2;
patternDetect *glider3;

vector<resPattern> datas;
vector<resPattern>::iterator resData;

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
      oscSending(datas);
    }
}

/*今は参照渡し風に書いている　複数のインスタンスを渡してバグが生まれたら対応*/
void gameOfLife::oscSending(vector<resPattern> &datas) {
  for(resData = datas.begin(); resData != datas.end(); ++resData) {
    std::stringstream result_x, result_y;
    std::copy(&*resData->x.begin(), &*resData->x.end(), std::ostream_iterator<int>(result_x, ","));
    std::copy(&*resData->y.begin(), &*resData->y.end(), std::ostream_iterator<int>(result_y, ","));
/*
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
 */
  };
  

}


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
              //              myImage.ofImage_::draw((float)((i + resData->x.at(h)) * cellWidth), (float)((j + resData->y.at(h)) * cellHeight), cellWidth*3.0, cellHeight*3.0);
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
      
      default:
      break;
  }
}
