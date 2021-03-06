//
// Guttemberg Machado on 24/07/17.
//
#ifndef DORA_BINARIZATION_H
#define DORA_BINARIZATION_H

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "helper.h"

using namespace cv;
using namespace std;

enum enumBinarization
{
    binarization_NONE = 0,
    binarization_TRESHOLD = 1, //thresold is 127
    binarization_MEAN = 2,     //treshold is "the mean of neighbourhood area"
    binarization_GAUSSIAN =3,  //threshold is weighted sum of neighbourhood values where weights are a gaussian window
    binarization_NIBLACK = 4,
    binarization_SAUVOLA = 5,
    binarization_WOLFJOLION = 6,
    binarization_BRADLEY = 7,
    binarization_CLAHE = 8,
};

#define uget(x,y)    at<unsigned char>(y,x)
#define uset(x,y,v)  at<unsigned char>(y,x)=v;
#define fget(x,y)    at<float>(y,x)
#define fset(x,y,v)  at<float>(y,x)=v;


bool binarize(Mat &source, Mat &dest, enumBinarization method);

#endif