
//
//  hough.cpp
//  Hough
//
//  Created by Maxx Sokal on 2/7/17.
//  Copyright © 2017 Maxx Sokal. All rights reserved.
//

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "hough.hpp"
#include <math.h>

#define DEBUG 1
#define PI 3.14159265
#define SHOWPLOT 1 //SHOWPLOT plots every step of the transform in a window.
using namespace std;
using namespace cv;


int BINCOUNT = 18;
int LOWTHRESHOLD = 30;
int RATIO = 3;
int KERNEL_SIZE = 3;
int BLUR_SIZE = 5;
int MULTIPLE = 10;
int GRAPH_SPACE = 5;
int MINUSMAX = 0;
int ACCUM_KERNEL = 3;


float PHIMIN = -CV_PI;
float PHIMAX = CV_PI;
float HALF_PI = CV_PI/2;
float TODEG = 180/CV_PI;
float TORAD = CV_PI/180;
float INV_DELTAPHI = (float)BINCOUNT/CV_PI;

int SCALE_MIN = 1;
int SCALE_MAX = 3;
float SCALE_STEP = .25f;
int SCALE = ceil(float(SCALE_MAX - SCALE_MIN)/SCALE_STEP + 1.0f);



/* where 0: blur; 1: GaussianBlur, 2: medianBlur; 3:bilateralFilter; */
int blurType = 1;

GHT::GHT(cv::Mat obf):imOrig(obf)
{
    cvtColor( imOrig, imGray, CV_BGR2GRAY );
    getEdgeInformation();
    setCentroid();
    initRtable();
}


void GHT::getEdgeInformation() {
    //Perform canny edge detection according to thresholds in global variables
    imGray = blurImage(imGray);
    Canny( imGray, imEdge, LOWTHRESHOLD, LOWTHRESHOLD*RATIO, KERNEL_SIZE );
    imEdge.convertTo(imEdge, CV_32F);
    imEdge /= 255.0f;
    
#if SHOWPLOT
    imshow("Blurred", imGray);
    waitKey(0);
    destroyWindow("Blurred");
    imshow("Canny", imEdge);
    waitKey(0);
    destroyWindow("Canny");
#endif
    
    //Get gradient information from edge image
    imGrad = getGradient(imGray);
}


void GHT::findObject(Mat imRGB) {
    Mat im, imblur;
    cvtColor( imRGB, im, CV_BGR2GRAY );
    Mat edge, gradient, mag, gx, gy;
    int temp = iPow(im.rows,2) + (iPow(im.cols,2));
    
    //Get dmax and build the accum array
    dmax = (int)(sqrt(temp) +0.5f) + 1;
    cout << "Scale = " << SCALE << endl;
    vector <vector <vector<int> > > accum (dmax, vector < vector<int> > (dmax, vector<int> (SCALE, 0)));
    
    //blur the image and perform canny edge detection
    imblur = blurImage(im);
    Canny( imblur, edge, LOWTHRESHOLD, LOWTHRESHOLD*RATIO, KERNEL_SIZE );
#if SHOWPLOT
    imshow("CannyEdge", edge);
    waitKey(0);
    destroyWindow("CannyEdge");
#endif
    edge.convertTo(edge, CV_32F);
    edge /= 255.0f;
    
    gradient = getGradient(im);
    for (int i = 0; i < edge.rows; i ++) {
        for (int j = 0; j < edge.cols; j++) {
            if (edge.at<float>(i,j) == 1.0f){
                float phi = gradient.at<float>(i,j);
                //Determine phi from the gradient, and get the bin number for R-Table
                int entrynum = getEntryNum(phi);
                vector<struct Rentry>::iterator it;
                
                //iterate through entries for phi in the Rtable
                for (it = rtable.at(entrynum).begin(); it != rtable.at(entrynum).end(); it++) {
                    //Calculate the scale based on global variables
                    for (int k = 0; k < SCALE; k ++) {
                        float coscol = cos(it->alpha);
                        float sinrow = sin(it->alpha);
                        float scaleFactor = (k + 1) * SCALE_STEP;
                        float coltemp = (i + scaleFactor * (it->radius)*coscol);
                        float rowtemp = (j + scaleFactor * (it->radius)*sinrow);
                        int coli = (int)(coltemp + 0.5f);
                        int rowi = (int)(rowtemp + 0.5f);
                        if ( coli < dmax && coli >= 0){
                            if (rowi < dmax && rowi >= 0){
                                //accumulate array with estimated xc,yc
                                accum[coli][rowi][k]++;
                                
                            }
                        }
                    }
                }
            }
        }
    }
    
    //Find the maximum of the accumulator array
    cout << "vecMax accum\n";
    int vecMax = 0;
    for (int i = 0; i < dmax; i ++) {
        for (int j = 0; j < dmax; j++){
            for (int k = 0; k < SCALE; k ++) {
                vecMax = ( accum[i][j][k] < vecMax ? vecMax : accum[i][j][k]);
            }
        }
    }
    vector <Point> pv;
    
    int cRad = 20;
    int thickness = 2;
    int lineType = 8;
    int shift = 0;
    int mm = vecMax - MINUSMAX;
    Mat accumTemp;
    imRGB.copyTo(accumTemp);
    printf("MaxVal is %i\n", mm);
    printf("VecMax = %i\n",vecMax);
    //get median in accum array
    
    
    cout << "Accum == vecMax\n";
    for (int i = 0; i < dmax; i ++){
        for (int j = 0; j < dmax; j++){
            for (int k = 0; k < SCALE; k++) {
                if (accum[i][j][k] == vecMax) {
                    Point temp = Point(j,i);
                    
                    cout << "POINT FOUND "<< temp << endl;
                    printf("Max at (i,j,k) = (%i,%i,%i)\n", i,j,k);
                    
                    circle(accumTemp, temp, cRad, Scalar(255,0,0),thickness,lineType, shift);
                    circle(accumTemp, temp, 3, Scalar(0,0,255), -1, lineType,shift);
                }
            }
        }
    }
    
    namedWindow( "Display window", WINDOW_AUTOSIZE );
    imshow( "Display window", accumTemp);
    
    waitKey(0);
    destroyWindow("Display window");
    
    
    //Transform the accum array to Mat, sum the scale and average
    Mat accumMat;
    accumMat = vec2Mat(accum);
    Mat matTemp;
    imRGB.copyTo(matTemp);
    medianBlur(accumMat,accumMat, ACCUM_KERNEL);
    cout << "MAT : " << accumMat << endl;

    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(accumMat, &minVal,&maxVal, &minLoc, &maxLoc);
#if DEBUG
    cout << "AccumMat stats:\n";
    cout << "Max Val : "<< maxVal << endl;
    cout << "MaxLoc: " << maxLoc << endl;
#endif
    
    //Draw circles on the objects, pinpoint the centers
    mm = maxVal - MINUSMAX;
    printf("MaxVal is %i\n", mm);
    for (int i = 0; i < accumMat.rows; i++){
        for (int j = 0; j < accumMat.cols; j++) {
            if (accumMat.at<uchar>(i,j) == mm){
                Point coor = Point(j,i);
                cout << "POINT FOUND "<< coor << endl;
                circle(matTemp, coor, cRad, Scalar(255,0,0),thickness,lineType, shift);
                circle(matTemp, coor, 3, Scalar(0,0,255), -1, lineType,shift);
            }
        }
    }
    
    namedWindow( "Mat Disp", WINDOW_AUTOSIZE );
    imshow( "Mat Disp", matTemp);
    
    waitKey(0);
    destroyWindow("Mat Disp");
    
}

void GHT::initRtable(){
    
    //initialize the R table
    for (int i = 0; i < BINCOUNT; i++){
        vector<struct Rentry> temp;
        rtable.push_back(temp);
    }
    
    for (int i = 0; i < imEdge.rows; i++ ) {
        for (int j = 0; j < imEdge.cols; j++) {
            if (imEdge.at<float>(i,j) == 1.0f) {
                float rowtemp = j - rowc; //y axis
                float coltemp = i - colc;  //x axis
                float r = iPow(rowtemp,2) + iPow(coltemp,2);
                r = sqrt(r);
                
                //compute the gradient
                float a = atan2(rowtemp, coltemp);
                
                float beta;
                if (coltemp < 0 && rowtemp >= 0) {
                    beta = a - CV_PI;
                }
                else if (coltemp <0 && rowtemp < 0){
                    beta = a + CV_PI;
                }
                else{
                    beta = a;
                }
                float phi = imGrad.at<float>(i,j);
                
                int entrynum = getEntryNum(phi);
                
                //add entry for phi in the rtable
                struct Rentry tempentry(coltemp, rowtemp, r,beta);
                rtable.at(entrynum).push_back(tempentry);
            }
        }
    }
}

Mat GHT::getGradient(Mat im){
    Mat gx, gy;
    Mat gradImage;
    Sobel(im, gx, CV_32F, 1, 0, 1);
    Sobel(im, gy, CV_32F, 0, 1, 1);
    gradImage = Mat::zeros(im.size(), gx.type());
    
    for (int i = 0; i < gx.rows; i ++){
        for (int j = 0; j < gx.cols; j++) {
            float coltemp =gx.at<float>(i,j);
            float rowtemp =gy.at<float>(i,j);
            float temp = atan2(rowtemp, coltemp);
            //convert grad to between -90 and 90;
            if (coltemp < 0 && rowtemp >= 0) {
                temp = temp - CV_PI;
            }
            else if (coltemp <0 && rowtemp < 0){
                temp = temp + CV_PI;
            }
            else{
                temp = temp;
            }
            gradImage.at<float>(i,j) = temp;
        }
    }
    return gradImage;
}

Mat GHT::blurImage(Mat im){
    Mat blurred;
    switch(blurType){
        case 0:
            blur(im, blurred, Size(BLUR_SIZE,BLUR_SIZE) );
            break;
        case 1:
            GaussianBlur( im, blurred, Size(BLUR_SIZE,BLUR_SIZE), 0, 0 );
            break;
        case 2:
            medianBlur(im, blurred, BLUR_SIZE);
            break;
        case 3:
            bilateralFilter(im, blurred, BLUR_SIZE, BLUR_SIZE, BLUR_SIZE);
            break;
        default:
            blur(im, blurred, Size(BLUR_SIZE,BLUR_SIZE) );
            break;
    }
    return blurred;
}

int GHT::getEntryNum(float phi) {
    
    float phideg = (phi + HALF_PI) * TODEG;
    int entrynum = (int)(phideg);
    
    
    
    entrynum = ((entrynum + MULTIPLE/2)/MULTIPLE);
    if (entrynum == BINCOUNT){
        entrynum = 0;
    }
    return entrynum;
}

void GHT::printRTable() {
    int vecPos = 0;
    cout << "****RTABLE****" << endl;
    vector< vector<struct Rentry> >::iterator row;
    vector<struct Rentry>::iterator col;
    for (row = rtable.begin(); row != rtable.end(); row++) {
        printf("VecPos %i : ", vecPos);
        vecPos ++;
        for (col = row->begin(); col != row->end(); col++) {
            printf(" r = (%f), a = %f ,", col->radius, col->alpha);
        }
        cout << endl;
    }
}

void GHT::setCentroid() {
    //determine the center of the object
    int sumcol = 0;
    int sumrow = 0;
    int pointCount = 0;
    for (int i = 0; i < imEdge.rows; i++) {
        for (int  j = 0; j < imEdge.cols; j++) {
            if (imEdge.at<float>(i,j) == 1){
                sumrow += i;
                sumcol += j;
                pointCount ++;
            }
        }
    }
    rowc = sumrow/pointCount;
    colc = sumcol/pointCount;
    
    int lineType = 8;
    int shift = 0;
    Point centroid = Point(colc,rowc);
    circle(imOrig, centroid, 1, Scalar(0,0,255), -1, lineType,shift);
    imshow("imOrig",imOrig);
    
    waitKey(0);
    destroyWindow("imOrig");
}

Mat GHT::vec2Mat( vector <vector< vector<int> > > &vec) {
    //converts the accum vector into a Mat
    int rows = vec.size();
    int cols = vec.at(0).size();
    int sum_of_elems = 0;
    cv::Mat accumMat( rows, cols, CV_8UC1);
    for(int i=0; i<accumMat.rows; ++i) {
        for(int j=0; j<accumMat.cols; ++j) {
            
            for(std::vector<int>::iterator it = vec[i][j].begin(); it != vec[i][j].end(); ++it){
                sum_of_elems += *it;
            }
            accumMat.at<uchar>(i,j) = sum_of_elems;
            sum_of_elems = 0;
            
        }
    }
    
    return accumMat;
}

int iPow(int x, int p){
    if (p == 0) return 1;
    if (p == 1) return x;
    
    int tmp = iPow(x, p/2);
    if (p%2 == 0) return tmp * tmp;
    else return x * tmp * tmp;
}

