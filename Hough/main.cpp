//
//  main.cpp
//  Hough
//
//  Created by Maxx Sokal on 2/6/17.
//  Copyright © 2017 Maxx Sokal. All rights reserved.
//

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "hough.hpp"


using namespace cv;
using namespace std;



Mat image, image_gray;
Mat canny_image, detected_edges;

string WINDOW_NAME = "Edge Map";


int main( int argc, char** argv )
{
   
    Mat image, imObject;
    //Object to locate
    imObject = imread("PA2-testimages/template_dog.png",CV_LOAD_IMAGE_COLOR);
    if(! imObject.data ) // Check for invalid input
    {
        cout << "Could not open or find the image" << std::endl ;
        return -1;
    }
    image = imread("PA2-testimages/animals2.jpg", CV_LOAD_IMAGE_COLOR);
    if(!image.data ) // Check for invalid input
    {
        cout << "Could not open or find the image " << std::endl ;
        return -1;
    }
    
    GHT* ght = new GHT(imObject);
    ght->findObject(image);
    return 0;
}
