#include"Record.h"
#include"SpeechRecognition.h"
#include"GestureRecognition.h"
#include<Windows.h>
#include"Smart Home.h"

bool testGesRec()
{
	if (!initVideo())
	{
		printf("摄像程序初始化失败！\n");
		return false;
	}
	if (!initGesRec())
	{
		printf("手势识别程序初始化失败！\n");
		return false;
	}
	state now;
	char ges[7][15] = { "张开", "握紧", "未检测到手", "上", "下", "左", "右" };
	int  t = 300;
	while (t--)
	{
		runGesRec();
		now = getState();
		//录音时不打印手势状态，以减少输出紊乱
		printf("%d ****************     手势状态（%d，%d，%s）      ****************\n", t, now.x, now.y, ges[now.gesture]);
	}
	closeCamera();
	return true;
}

bool testSpeRec()
{
	if (!initRecord())
	{
		printf("录音程序初始化失败！\n");
		return false;
	}
	if (!initSpeRec())
	{
		printf("语音识别程序初始化失败！\n");
		return false;
	}
	char *s;
	for (int i = 0; i < 3; i++)
	{
		printf("**********第%d次试验**********\n", i + 1);
		record();
		Sleep(6000);
		speechRecognition(getWavContent(), getWavSize());
		s = getRecReslut();
		printf("语音识别结果:%s\n", s);
	}
}

bool init()
{
	if (!initRecord())
	{
		printf("录音程序初始化失败！\n");
		return false;
	}
	if (!initVideo())
	{
		printf("摄像程序初始化失败！\n");
		return false;
	}
	if (!initSpeRec())
	{
		printf("语音识别程序初始化失败！\n");
		return false;
	}
	if (!initGesRec())
	{
		printf("手势识别程序初始化失败！\n");
		return false;
	}

	return true;
}

void close()
{
	hideCamera();
	closeRecord();
	closeCamera();
}
int getWord(char *s, char funcWord[][20],int funcCount)
{
	char *t;
	t = (char *)malloc(100 * sizeof(char));
	int l, m;
	for (int i = 0; i < funcCount; i++)
	{
		l = strlen(funcWord[i]);
		m = strlen(s);
		for (int j = 0; j < m - l; j++)
		{
			strncpy(t, s+j, l);
			t[l] = '\0';
			if (strcmp(t, funcWord[i]) == 0)
				return i;
		}
	}
	return  -1;
}
void modifyState(int a, int b, int c)
{
	FILE *fp = fopen("config/config.txt", "w");
	fprintf(fp, "%d,%d,%d", a, b, c);
	fclose(fp);
}
void openImage()
{
	Mat testPic = imread("test.jpg");
	imshow("test", testPic);
	
}
void closeImage()
{
	cvDestroyWindow("test");
}
int mainFunc()
{
	if (!init())
		return 0;
	state now;
	bool isRunning = true, isRecording = false;//运行于录音状态
	int lastTime = 0, lastGesture = -1;//上一个动作与持续时间
	int funcCount = 10, res;
	int a=0, b=0, c=0;
	char *s="",funcWord[9][20] = { 
		"打开左边的灯", 
		"打开右边的灯",
		"关闭左边的灯",
		"关闭右边的灯",
		"打开电视",
		"关闭电视",
		"隐藏摄像头",
		"显示摄像头",
		"退出系统"
	};
	char ges[7][15] = { "张开", "握紧", "未检测到手", "上", "下", "左", "右" };
	res = getWord(s, funcWord, funcCount);
	//defineFunc(funcWord);
	while (isRunning)
	{
		runGesRec();
		now = getState();
		//录音时不打印手势状态，以减少输出紊乱
		if (isRecording == 0)
			printf("****************     手势状态（%d，%d，%s）      ****************\n", now.x, now.y, ges[now.gesture]);

		//姿势替换	
		if (lastGesture == now.gesture)
			lastTime++;
		else
		{
			lastGesture = now.gesture;
			lastTime = 1;
		}

		//手势操控的接口
		switch (now.gesture)
		{
		case 0://张开
			if (isRecording == 1)
			{
				//手松开且正在录音，则结束录音，开始识别
				printf("\n录音完成\n");
				speechRecognition(getWavContent(), getWavSize());
				s = getRecReslut();
				printf("语音识别结果:%s\n", s);

				res = getWord(s, funcWord, funcCount);
				//语音操控的接口
				switch (res)
				{
				case 0://打开图像
					a = 1;
					modifyState(a,b,c);
					break;
				case 1:
					b = 1;
					modifyState(a, b, c);
					break;
				case 2:
					a = 0;
					modifyState(a, b, c);
					break;
				case 3:
					b = 0;
					modifyState(a, b, c);
					break;
				case 4:
					c = 1;
					modifyState(a, b, c);
					break;
				case 5:
					c = 0;
					modifyState(a, b, c);
					break;
				case 6:
					hideCamera();
					break;
				case 7:
					showCamera();
					break;
				case 8:
					isRunning = 0;
					break;
				default:
					printf("未定义该命令\n");
					break;
				}
				//initGesRec();
				isRecording = 0;
			}
			break;
		case 1://握手
			if (isRecording == 0)
			{
				//手握紧且未录音，则开始录音
				if (lastTime >= 8)
				{
					//握手动作至少8帧，才能确定为握手
					record();
					isRecording = 1;
					lastTime = 0;
				}
			}
			break;
		case 2://未检测到手
			if (lastTime >= 50)
			{
				printf("多次未检测到手势，程序结束\n");
				isRunning = 0;
			}
			break;
		case 3://上
			if (lastTime >= 3)
			{
				//UP();
				lastTime = 0;
			}
			break;
		case 4://下
			if (lastTime >= 3)
			{
				//DOWN();
				lastTime = 0;
			}
			break;
		case 5://左
			if (lastTime >= 3)
			{
				if (c == 1 || c == 2)
				{
					c = 3 - c;
					modifyState(a, b, c);
				}
				lastTime = 0;
			}
			break;
		case 6://右
			if (lastTime >= 3)
			{
				if (c == 1 || c == 2)
				{
					c = 3 - c;
					modifyState(a, b, c);
				}
				lastTime = 0;
			}
			break;
		default:
			break;
		}
	}

	close();
	return 1;
}
int main()
{
	//testGesRec();
	//testSpeRec();
	mainFunc();

	system("pause");
	return 0;
}