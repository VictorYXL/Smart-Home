#include "GestureRecognition.h"
using namespace cv;

Mat backProject;
//皮肤模型
MatND hist;
//级联分类器
CascadeClassifier palmCascade;
CascadeClassifier fistCascade;
//灰度图
Mat preGray;
int successiveDetect;
//源图
Mat frame;
//目标区域
Rect trackBox;
Point curPoint;
//摄像头
VideoCapture capture;
bool gotTrackBox;
int interCount;
int continue_fist;
bool isRunning;
//手势状态
state now, pre;
bool show;

//摄像初始化
bool initVideo()
{
	//摄像头打开
	capture.open(0); 
	if (capture.isOpened())
	{
		show = true;
		printf("摄像头已打开\n");
		return true;
	}
	else return false;
}

//手势识别初始化
bool initGesRec()
{
	gotTrackBox = false;
	interCount = 0;
	continue_fist = 0;
	successiveDetect = 0;
	const char *palmCascadeName = "dat/palm.dat";
	const char *fistCascadeName = "dat/fist.dat";
	int time = 300;
	isRunning = false;

	//加载级联分类器
	if (!palmCascade.load(palmCascadeName) || !fistCascade.load(fistCascadeName))
		return false;
	else
	{
		//初次定位
		printf("正在定位手势...\n");
		while (!gotTrackBox && time > 0)
		{
			//获取摄像头内容
			capture >> frame;
			if (frame.empty())
				continue;
			//显示
			if (show)
				imshow("handTracker", frame);

			trackBox = Rect(0, 0, 0, 0);
			time--;;
			bool flag = false;
			//定位手掌
			detectPalm(frame, trackBox);

			//判断大小
			if (trackBox.area() > 900 && 0.3 * frame.cols < trackBox.x + 0.5 * trackBox.width
				&& trackBox.x + 0.5 * trackBox.width < 0.7 * frame.cols
				&& 0.3 * frame.rows < trackBox.y + 0.5 * trackBox.height
				&& trackBox.y + 0.5 * trackBox.height < 0.7 * frame.rows)
			{
				//判断为手
				if (isHand(frame(trackBox)))
				{
					//获取皮肤模型
					getSkinModel(frame, trackBox);

					successiveDetect = 0;
					gotTrackBox = true;

					//获取当前位置
					curPoint = Point(trackBox.x + 0.5 * trackBox.width, trackBox.y + 0.5 * trackBox.height);
					now.x = curPoint.x;
					now.y = curPoint.y;
					printf("定位成功\n");
					return true;
				}
			}
			cvWaitKey(3);
		}
	}
	printf("手势定位失败，未能检测到手\n");
	return false;
}

//检测掌心
void detectPalm(Mat img, Rect &box)
{
	double scale = 1.3;
	Mat small_img, gray;
	vector<Rect> boxs;
	//灰度图像创建
	gray.create(img.rows, img.cols, CV_8UC1);
	small_img.create(cvRound(gray.rows / scale), cvRound(gray.cols / scale), CV_8UC1);

	//转为灰度图像
	cvtColor(img, gray, CV_BGR2GRAY);
	resize(gray, small_img, small_img.size(), 0, 0, INTER_LINEAR);
	//直方图均衡化
	equalizeHist(small_img, small_img);

	//级联分类器分类张开手势
	palmCascade.detectMultiScale(small_img, boxs, 1.1, 2, CV_HAAR_SCALE_IMAGE, Size(30, 30));

	//获取目标区域中最大的区域
	Rect maxBox(0, 0, 0, 0);
	for (vector<Rect>::const_iterator r = boxs.begin(); r != boxs.end(); r++)
	{
		if (r->area() > maxBox.area())
			maxBox = *r;
	}

	if (boxs.size() > 0)
	{
		box.x = cvRound(maxBox.x * scale);
		box.y = cvRound(maxBox.y * scale);
		box.width = cvRound(maxBox.width * scale);
		box.height = cvRound(maxBox.height * scale);
	}
}

//判断为手
bool isHand(const Mat frame)
{
	Mat YCbCr;
	vector<Mat> planes;
	int count = 0;
	//转为灰度图
	cvtColor(frame, YCbCr, CV_RGB2YCrCb);
	//分离RGB三通道
	split(YCbCr, planes);

	MatIterator_<uchar> it_Cb = planes[1].begin<uchar>(),
		it_Cb_end = planes[1].end<uchar>();
	MatIterator_<uchar> it_Cr = planes[2].begin<uchar>();

	//统计RB值在规定范围内的像素个数
	for (; it_Cb != it_Cb_end; ++it_Cr, ++it_Cb)
	{
		if (138 <= *it_Cr &&  *it_Cr <= 170 && 100 <= *it_Cb &&  *it_Cb <= 127)
			count++;
	}

	//满足条件的像素占30%以上则判断成功
	return (count > 0.3 * frame.cols * frame.rows && count < 0.7 * frame.cols * frame.rows);
}

//判断握拳
bool detectFist(Mat frame, Rect palmBox)
{
	Rect detectFistBox;
	//确定握拳范围
	detectFistBox.x = (palmBox.x - 40) > 0 ? (palmBox.x - 40) : 0;
	detectFistBox.y = (palmBox.y - 20) > 0 ? (palmBox.y - 20) : 0;
	detectFistBox.width = palmBox.width + 80;
	detectFistBox.height = palmBox.height + 40;
	detectFistBox &= Rect(0, 0, frame.cols, frame.rows);

	Mat gray;
	//转为灰度图
	cvtColor(frame, gray, CV_BGR2GRAY);
	Mat tmp = gray(detectFistBox);
	vector<Rect> fists;

	//级联分类器分类握拳姿势
	fistCascade.detectMultiScale(tmp, fists, 1.1, 2, CV_HAAR_SCALE_IMAGE, Size(30, 30));
	return fists.size();
}

//获取皮肤模型
void getSkinModel(const Mat img, Rect rect)
{
	int hue_Bins = 50;
	float hue_Ranges[] = { 0, 180 };
	const float *ranges = hue_Ranges;
	Mat HSV, hue, mask;
	vector<Mat> planes;

	//转为灰度图
	cvtColor(img, HSV, CV_RGB2HSV);

	//提取[0，30，30]到[180，256，256]内像素
	inRange(HSV, Scalar(0, 30, 10), Scalar(180, 256, 256), mask);

	//分离红通道
	split(HSV, planes);
	hue = planes[0];

	Mat roi(hue, rect), maskroi(mask, rect);
	//计算直方图
	calcHist(&roi, 1, 0, maskroi, hist, 1, &hue_Bins, &ranges);
	//标准化
	normalize(hist, hist, 0, 255, CV_MINMAX);
	printf("f");
}

//连续识别函数
void runGesRec()
{
	int dir[4] = { pre.y - now.y, now.y - pre.y, pre.x - now.x, now.x - pre.x }, max = 0;

	//检测目标区域是否为手
	if (isHand(frame(trackBox)))
	{
		if (show)
			imshow("handTracker", frame);
		waitKey(2);
		capture >> frame;

		//手势重定位
		if (!processFrame(frame, trackBox))
			gotTrackBox = false;

		//获取姿态
		pre.x = now.x;
		pre.y = now.y;
		curPoint = Point(trackBox.x + 0.5 * trackBox.width, trackBox.y + 0.5 * trackBox.height);
		now.x = curPoint.x;
		now.y = curPoint.y;

		//判断移动
		for (int i = 1; i < 4; i++)
		if (dir[i]>dir[max])
			max = i;
		if (dir[max] > 10)
			now.gesture = max+3;
		else
		{
			//判断握拳
			if (detectFist(frame, trackBox))
				now.gesture = 1;
			else now.gesture = 0;
		}

		rectangle(frame, trackBox, Scalar(0, 0, 255), 3);
		if (show)
			imshow("handTracker", frame);
		cvWaitKey(3);
	}
	else
	{
		//容错处理，握拳-〉离开修正为握拳-〉握拳
		if (now.gesture == 1)
		{
			if (show)
				imshow("handTracker", frame);
			waitKey(2);
			capture >> frame;
			rectangle(frame, trackBox, Scalar(0, 0, 255), 3);
		}
		else 
		{
			//判断离开，张开-〉离开
			now.x = -1;
			now.y = -1;
			now.gesture = 2;

			capture >> frame;
			if (show)
				imshow("handTracker", frame);
			waitKey(2);
		}
	}
}

//识别过程
bool processFrame(Mat frame, Rect &trackBox)
{
	float rate = 0.9;
	Mat diff;

	//图片预处理
	//获取皮肤轮廓
	calSkinPro(frame); 
	//获取图像偏差
	frameDiff(frame, diff);

	//合成新图
	Mat handProMap = backProject * rate + (1 - rate) * diff;

	//手势跟踪
	meanShift(handProMap, trackBox, TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));

	//非空像素在40%以上则跟踪成功
	Mat skin = backProject(trackBox) > 100;
	return countNonZero(skin) > 0.3 * trackBox.area();
}
//获取皮肤轮廓
void calSkinPro(Mat frame)
{
	Mat mask, hue, HSV;
	//转为灰度图
	cvtColor(frame, HSV, CV_RGB2HSV);
	inRange(HSV, Scalar(0, 30, 10), Scalar(180, 256, 256), mask);
	vector<Mat> planes;

	//分类RGN三通道
	split(HSV, planes);
	hue = planes[0];

	//利用皮肤模型对红色通道做反向处理
	float hue_Ranges[] = { 0, 180 };
	const float *ranges = hue_Ranges;
	calcBackProject(&hue, 1, 0, hist, backProject, &ranges, 1.0, true);
	backProject &= mask;
}
//获取图像偏差
void frameDiff(const Mat image, Mat &diff)
{
	int thresValue = 20;
	Mat curGray;
	//转为灰度图
	cvtColor(image, curGray, CV_RGB2GRAY);
	if (preGray.size != curGray.size)
		curGray.copyTo(preGray);

	//计算偏差矩阵
	absdiff(preGray, curGray, diff);

	//二值化
	threshold(diff, diff, thresValue, 255, CV_THRESH_BINARY);
	//腐蚀
	erode(diff, diff, Mat(3, 3, CV_8UC1), Point(-1, -1));
	//膨胀
	dilate(diff, diff, Mat(3, 3, CV_8UC1), Point(-1, -1));

	curGray.copyTo(preGray);
}

//返回手势状态
state getState()
{
	return now;
}
void closeCamera()
{
	printf("正在关闭摄像头\n");
	capture.release();
}
void hideCamera()
{
	cvDestroyWindow("handTracker");
	show = false;
}
void showCamera()
{
	show = true;
}