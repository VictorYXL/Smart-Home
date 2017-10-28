#include"Record.h"
#include"SpeechRecognition.h"
#include"GestureRecognition.h"
#include<Windows.h>
#include"Smart Home.h"

bool testGesRec()
{
	if (!initVideo())
	{
		printf("��������ʼ��ʧ�ܣ�\n");
		return false;
	}
	if (!initGesRec())
	{
		printf("����ʶ������ʼ��ʧ�ܣ�\n");
		return false;
	}
	state now;
	char ges[7][15] = { "�ſ�", "�ս�", "δ��⵽��", "��", "��", "��", "��" };
	int  t = 300;
	while (t--)
	{
		runGesRec();
		now = getState();
		//¼��ʱ����ӡ����״̬���Լ����������
		printf("%d ****************     ����״̬��%d��%d��%s��      ****************\n", t, now.x, now.y, ges[now.gesture]);
	}
	closeCamera();
	return true;
}

bool testSpeRec()
{
	if (!initRecord())
	{
		printf("¼�������ʼ��ʧ�ܣ�\n");
		return false;
	}
	if (!initSpeRec())
	{
		printf("����ʶ������ʼ��ʧ�ܣ�\n");
		return false;
	}
	char *s;
	for (int i = 0; i < 3; i++)
	{
		printf("**********��%d������**********\n", i + 1);
		record();
		Sleep(6000);
		speechRecognition(getWavContent(), getWavSize());
		s = getRecReslut();
		printf("����ʶ����:%s\n", s);
	}
}

bool init()
{
	if (!initRecord())
	{
		printf("¼�������ʼ��ʧ�ܣ�\n");
		return false;
	}
	if (!initVideo())
	{
		printf("��������ʼ��ʧ�ܣ�\n");
		return false;
	}
	if (!initSpeRec())
	{
		printf("����ʶ������ʼ��ʧ�ܣ�\n");
		return false;
	}
	if (!initGesRec())
	{
		printf("����ʶ������ʼ��ʧ�ܣ�\n");
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
	bool isRunning = true, isRecording = false;//������¼��״̬
	int lastTime = 0, lastGesture = -1;//��һ�����������ʱ��
	int funcCount = 10, res;
	int a=0, b=0, c=0;
	char *s="",funcWord[9][20] = { 
		"����ߵĵ�", 
		"���ұߵĵ�",
		"�ر���ߵĵ�",
		"�ر��ұߵĵ�",
		"�򿪵���",
		"�رյ���",
		"��������ͷ",
		"��ʾ����ͷ",
		"�˳�ϵͳ"
	};
	char ges[7][15] = { "�ſ�", "�ս�", "δ��⵽��", "��", "��", "��", "��" };
	res = getWord(s, funcWord, funcCount);
	//defineFunc(funcWord);
	while (isRunning)
	{
		runGesRec();
		now = getState();
		//¼��ʱ����ӡ����״̬���Լ����������
		if (isRecording == 0)
			printf("****************     ����״̬��%d��%d��%s��      ****************\n", now.x, now.y, ges[now.gesture]);

		//�����滻	
		if (lastGesture == now.gesture)
			lastTime++;
		else
		{
			lastGesture = now.gesture;
			lastTime = 1;
		}

		//���ƲٿصĽӿ�
		switch (now.gesture)
		{
		case 0://�ſ�
			if (isRecording == 1)
			{
				//���ɿ�������¼���������¼������ʼʶ��
				printf("\n¼�����\n");
				speechRecognition(getWavContent(), getWavSize());
				s = getRecReslut();
				printf("����ʶ����:%s\n", s);

				res = getWord(s, funcWord, funcCount);
				//�����ٿصĽӿ�
				switch (res)
				{
				case 0://��ͼ��
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
					printf("δ���������\n");
					break;
				}
				//initGesRec();
				isRecording = 0;
			}
			break;
		case 1://����
			if (isRecording == 0)
			{
				//���ս���δ¼������ʼ¼��
				if (lastTime >= 8)
				{
					//���ֶ�������8֡������ȷ��Ϊ����
					record();
					isRecording = 1;
					lastTime = 0;
				}
			}
			break;
		case 2://δ��⵽��
			if (lastTime >= 50)
			{
				printf("���δ��⵽���ƣ��������\n");
				isRunning = 0;
			}
			break;
		case 3://��
			if (lastTime >= 3)
			{
				//UP();
				lastTime = 0;
			}
			break;
		case 4://��
			if (lastTime >= 3)
			{
				//DOWN();
				lastTime = 0;
			}
			break;
		case 5://��
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
		case 6://��
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