#pragma once
#include "opencv2/opencv.hpp"
using namespace cv;
using namespace cv;
using namespace std;

//手势位置与姿态
struct state
{
	int x;
	int y;
	int gesture;
	state()
	{
		x = -1;
		y = -1;
		gesture = -1;
	}
};
//初始化
bool initVideo();
bool initGesRec();
//初次识别
void detectPalm(Mat ,Rect&);
bool isHand(const Mat);
void getSkinModel(const Mat, Rect);

//连续识别
void runGesRec();
void frameDiff(const Mat, Mat&);
void calSkinPro(Mat);
bool detectFist(Mat, Rect);
bool processFrame(Mat, Rect&);

void closeCamera();
void hideCamera();
void showCamera();
//返回状态
state getState();