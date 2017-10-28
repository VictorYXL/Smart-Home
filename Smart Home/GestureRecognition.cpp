#include "GestureRecognition.h"
using namespace cv;

Mat backProject;
//Ƥ��ģ��
MatND hist;
//����������
CascadeClassifier palmCascade;
CascadeClassifier fistCascade;
//�Ҷ�ͼ
Mat preGray;
int successiveDetect;
//Դͼ
Mat frame;
//Ŀ������
Rect trackBox;
Point curPoint;
//����ͷ
VideoCapture capture;
bool gotTrackBox;
int interCount;
int continue_fist;
bool isRunning;
//����״̬
state now, pre;
bool show;

//�����ʼ��
bool initVideo()
{
	//����ͷ��
	capture.open(0); 
	if (capture.isOpened())
	{
		show = true;
		printf("����ͷ�Ѵ�\n");
		return true;
	}
	else return false;
}

//����ʶ���ʼ��
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

	//���ؼ���������
	if (!palmCascade.load(palmCascadeName) || !fistCascade.load(fistCascadeName))
		return false;
	else
	{
		//���ζ�λ
		printf("���ڶ�λ����...\n");
		while (!gotTrackBox && time > 0)
		{
			//��ȡ����ͷ����
			capture >> frame;
			if (frame.empty())
				continue;
			//��ʾ
			if (show)
				imshow("handTracker", frame);

			trackBox = Rect(0, 0, 0, 0);
			time--;;
			bool flag = false;
			//��λ����
			detectPalm(frame, trackBox);

			//�жϴ�С
			if (trackBox.area() > 900 && 0.3 * frame.cols < trackBox.x + 0.5 * trackBox.width
				&& trackBox.x + 0.5 * trackBox.width < 0.7 * frame.cols
				&& 0.3 * frame.rows < trackBox.y + 0.5 * trackBox.height
				&& trackBox.y + 0.5 * trackBox.height < 0.7 * frame.rows)
			{
				//�ж�Ϊ��
				if (isHand(frame(trackBox)))
				{
					//��ȡƤ��ģ��
					getSkinModel(frame, trackBox);

					successiveDetect = 0;
					gotTrackBox = true;

					//��ȡ��ǰλ��
					curPoint = Point(trackBox.x + 0.5 * trackBox.width, trackBox.y + 0.5 * trackBox.height);
					now.x = curPoint.x;
					now.y = curPoint.y;
					printf("��λ�ɹ�\n");
					return true;
				}
			}
			cvWaitKey(3);
		}
	}
	printf("���ƶ�λʧ�ܣ�δ�ܼ�⵽��\n");
	return false;
}

//�������
void detectPalm(Mat img, Rect &box)
{
	double scale = 1.3;
	Mat small_img, gray;
	vector<Rect> boxs;
	//�Ҷ�ͼ�񴴽�
	gray.create(img.rows, img.cols, CV_8UC1);
	small_img.create(cvRound(gray.rows / scale), cvRound(gray.cols / scale), CV_8UC1);

	//תΪ�Ҷ�ͼ��
	cvtColor(img, gray, CV_BGR2GRAY);
	resize(gray, small_img, small_img.size(), 0, 0, INTER_LINEAR);
	//ֱ��ͼ���⻯
	equalizeHist(small_img, small_img);

	//���������������ſ�����
	palmCascade.detectMultiScale(small_img, boxs, 1.1, 2, CV_HAAR_SCALE_IMAGE, Size(30, 30));

	//��ȡĿ����������������
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

//�ж�Ϊ��
bool isHand(const Mat frame)
{
	Mat YCbCr;
	vector<Mat> planes;
	int count = 0;
	//תΪ�Ҷ�ͼ
	cvtColor(frame, YCbCr, CV_RGB2YCrCb);
	//����RGB��ͨ��
	split(YCbCr, planes);

	MatIterator_<uchar> it_Cb = planes[1].begin<uchar>(),
		it_Cb_end = planes[1].end<uchar>();
	MatIterator_<uchar> it_Cr = planes[2].begin<uchar>();

	//ͳ��RBֵ�ڹ涨��Χ�ڵ����ظ���
	for (; it_Cb != it_Cb_end; ++it_Cr, ++it_Cb)
	{
		if (138 <= *it_Cr &&  *it_Cr <= 170 && 100 <= *it_Cb &&  *it_Cb <= 127)
			count++;
	}

	//��������������ռ30%�������жϳɹ�
	return (count > 0.3 * frame.cols * frame.rows && count < 0.7 * frame.cols * frame.rows);
}

//�ж���ȭ
bool detectFist(Mat frame, Rect palmBox)
{
	Rect detectFistBox;
	//ȷ����ȭ��Χ
	detectFistBox.x = (palmBox.x - 40) > 0 ? (palmBox.x - 40) : 0;
	detectFistBox.y = (palmBox.y - 20) > 0 ? (palmBox.y - 20) : 0;
	detectFistBox.width = palmBox.width + 80;
	detectFistBox.height = palmBox.height + 40;
	detectFistBox &= Rect(0, 0, frame.cols, frame.rows);

	Mat gray;
	//תΪ�Ҷ�ͼ
	cvtColor(frame, gray, CV_BGR2GRAY);
	Mat tmp = gray(detectFistBox);
	vector<Rect> fists;

	//����������������ȭ����
	fistCascade.detectMultiScale(tmp, fists, 1.1, 2, CV_HAAR_SCALE_IMAGE, Size(30, 30));
	return fists.size();
}

//��ȡƤ��ģ��
void getSkinModel(const Mat img, Rect rect)
{
	int hue_Bins = 50;
	float hue_Ranges[] = { 0, 180 };
	const float *ranges = hue_Ranges;
	Mat HSV, hue, mask;
	vector<Mat> planes;

	//תΪ�Ҷ�ͼ
	cvtColor(img, HSV, CV_RGB2HSV);

	//��ȡ[0��30��30]��[180��256��256]������
	inRange(HSV, Scalar(0, 30, 10), Scalar(180, 256, 256), mask);

	//�����ͨ��
	split(HSV, planes);
	hue = planes[0];

	Mat roi(hue, rect), maskroi(mask, rect);
	//����ֱ��ͼ
	calcHist(&roi, 1, 0, maskroi, hist, 1, &hue_Bins, &ranges);
	//��׼��
	normalize(hist, hist, 0, 255, CV_MINMAX);
	printf("f");
}

//����ʶ����
void runGesRec()
{
	int dir[4] = { pre.y - now.y, now.y - pre.y, pre.x - now.x, now.x - pre.x }, max = 0;

	//���Ŀ�������Ƿ�Ϊ��
	if (isHand(frame(trackBox)))
	{
		if (show)
			imshow("handTracker", frame);
		waitKey(2);
		capture >> frame;

		//�����ض�λ
		if (!processFrame(frame, trackBox))
			gotTrackBox = false;

		//��ȡ��̬
		pre.x = now.x;
		pre.y = now.y;
		curPoint = Point(trackBox.x + 0.5 * trackBox.width, trackBox.y + 0.5 * trackBox.height);
		now.x = curPoint.x;
		now.y = curPoint.y;

		//�ж��ƶ�
		for (int i = 1; i < 4; i++)
		if (dir[i]>dir[max])
			max = i;
		if (dir[max] > 10)
			now.gesture = max+3;
		else
		{
			//�ж���ȭ
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
		//�ݴ�����ȭ-���뿪����Ϊ��ȭ-����ȭ
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
			//�ж��뿪���ſ�-���뿪
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

//ʶ�����
bool processFrame(Mat frame, Rect &trackBox)
{
	float rate = 0.9;
	Mat diff;

	//ͼƬԤ����
	//��ȡƤ������
	calSkinPro(frame); 
	//��ȡͼ��ƫ��
	frameDiff(frame, diff);

	//�ϳ���ͼ
	Mat handProMap = backProject * rate + (1 - rate) * diff;

	//���Ƹ���
	meanShift(handProMap, trackBox, TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));

	//�ǿ�������40%��������ٳɹ�
	Mat skin = backProject(trackBox) > 100;
	return countNonZero(skin) > 0.3 * trackBox.area();
}
//��ȡƤ������
void calSkinPro(Mat frame)
{
	Mat mask, hue, HSV;
	//תΪ�Ҷ�ͼ
	cvtColor(frame, HSV, CV_RGB2HSV);
	inRange(HSV, Scalar(0, 30, 10), Scalar(180, 256, 256), mask);
	vector<Mat> planes;

	//����RGN��ͨ��
	split(HSV, planes);
	hue = planes[0];

	//����Ƥ��ģ�ͶԺ�ɫͨ����������
	float hue_Ranges[] = { 0, 180 };
	const float *ranges = hue_Ranges;
	calcBackProject(&hue, 1, 0, hist, backProject, &ranges, 1.0, true);
	backProject &= mask;
}
//��ȡͼ��ƫ��
void frameDiff(const Mat image, Mat &diff)
{
	int thresValue = 20;
	Mat curGray;
	//תΪ�Ҷ�ͼ
	cvtColor(image, curGray, CV_RGB2GRAY);
	if (preGray.size != curGray.size)
		curGray.copyTo(preGray);

	//����ƫ�����
	absdiff(preGray, curGray, diff);

	//��ֵ��
	threshold(diff, diff, thresValue, 255, CV_THRESH_BINARY);
	//��ʴ
	erode(diff, diff, Mat(3, 3, CV_8UC1), Point(-1, -1));
	//����
	dilate(diff, diff, Mat(3, 3, CV_8UC1), Point(-1, -1));

	curGray.copyTo(preGray);
}

//��������״̬
state getState()
{
	return now;
}
void closeCamera()
{
	printf("���ڹر�����ͷ\n");
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