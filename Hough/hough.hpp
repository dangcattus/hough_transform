//
//  hough.hpp
//  Hough
//
//  Created by Maxx Sokal on 2/7/17.
//  Copyright © 2017 Maxx Sokal. All rights reserved.
//

#ifndef hough_hpp
#define hough_hpp

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <math.h>
#include <vector>


class GHT
{
public:
    void findObject(cv::Mat im);
    void printRTable();
private:
    struct Rentry {
       // float radius; //length (r)
      //  cv::Point radius; //store pounts (row,col)
        float radius;
        float alpha;  // angle (a)
        float x,y;
        
        Rentry(float r, float a) : radius(r), alpha(a) {}
        Rentry(float xx, float yy, float r, float a) : x(xx),y(yy),radius(r), alpha(a) {}
    };
    
    
    //Matrices used
    cv::Mat accum;
    cv::Mat imOrig;
    cv::Mat imEdge;
    cv::Mat imMag;
    cv::Mat imGray;
    cv::Mat imGrad;
    std::vector< std::vector<struct Rentry>> rtable;
    int rowc; //when using .at its row, convert to y
    int colc; //using .at its col, convert to x
    int dmax;
    
    
    void graphSine();
    void initAccum(int dmax);
    void initRtable();
    void getEdgeInformation();
    void setCentroid();
    //cv::Mat vec2Mat(std::vector< std::vector <std::vector <std::vector <int> > > > &vec); //for r and s
    cv::Mat vec2Mat(std::vector <std::vector <std::vector <int> > > &vec); //for s
    cv::Mat getGradient(cv::Mat im);
    cv::Mat blurImage(cv::Mat im);
public:
    int getEntryNum(float phi);
    
    
    GHT (cv::Mat obf) ;
};

int iPow(int x, int p);

#endif /* hough_hpp */
