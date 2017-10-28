#pragma once
#include "opencv2/opencv.hpp"
using namespace cv;
using namespace cv;
using namespace std;

//����λ������̬
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
//��ʼ��
bool initVideo();
bool initGesRec();
//����ʶ��
void detectPalm(Mat ,Rect&);
bool isHand(const Mat);
void getSkinModel(const Mat, Rect);

//����ʶ��
void runGesRec();
void frameDiff(const Mat, Mat&);
void calSkinPro(Mat);
bool detectFist(Mat, Rect);
bool processFrame(Mat, Rect&);

void closeCamera();
void hideCamera();
void showCamera();
//����״̬
state getState();