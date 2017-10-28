#pragma once
//#include"stdafx.h"
#include <STDIO.H>
#include <STDLIB.H>
#include<WINDOWS.H>
#include <TIME.H>
#include <mmsystem.h>
#include <windows.h>
#pragma comment(lib, "winmm.lib")
//��ʱ�ڴ�����ʶ������С
#define MAX_SOUNDSIZE 32000
#define MAX_BUFFERSIZE MAX_SOUNDSIZE * 100 + 44

//wav�ļ�ͷ
struct RIFF_HEADER
{
	char RIFF[4];//RIFF
	DWORD RSize;//���ļ�����-8
	char WAVE[4];//WAVE
};
struct WAVE_FORMAT
{
	WORD    Format;        //��ʽ��־
	WORD    ChannelsCount;         //������
	DWORD   nSamplesPerSec;    //������
	DWORD   nAvgBytesPerSec;   //ÿ���С
	WORD    nBlockAlign;       //�ڴ����Ŀ
	WORD    wBitsPerSample;    
};
struct FMT_BLOCK
{
	char  szFmtID[4]; // fmt 
	DWORD  dwFmtSize;//WAVE��ʽ��С
	WAVE_FORMAT wavFormat;
};

//���ݿ�
struct DATA_BLOCK
{
	char  szDataID[4]; // data
	DWORD  dwDataSize;
};


//��ʼ��
bool initRecord();
//¼��
void record();
DWORD CALLBACK waveInProc(HWAVEIN, UINT, DWORD, DWORD, DWORD);
void closeRecord();
//��ȡ״̬
char* getWavContent();
int getWavSize();

