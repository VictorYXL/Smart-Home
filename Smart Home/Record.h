#pragma once
//#include"stdafx.h"
#include <STDIO.H>
#include <STDLIB.H>
#include<WINDOWS.H>
#include <TIME.H>
#include <mmsystem.h>
#include <windows.h>
#pragma comment(lib, "winmm.lib")
//临时内存块与带识别区大小
#define MAX_SOUNDSIZE 32000
#define MAX_BUFFERSIZE MAX_SOUNDSIZE * 100 + 44

//wav文件头
struct RIFF_HEADER
{
	char RIFF[4];//RIFF
	DWORD RSize;//总文件长度-8
	char WAVE[4];//WAVE
};
struct WAVE_FORMAT
{
	WORD    Format;        //格式标志
	WORD    ChannelsCount;         //声道数
	DWORD   nSamplesPerSec;    //采样率
	DWORD   nAvgBytesPerSec;   //每秒大小
	WORD    nBlockAlign;       //内存块数目
	WORD    wBitsPerSample;    
};
struct FMT_BLOCK
{
	char  szFmtID[4]; // fmt 
	DWORD  dwFmtSize;//WAVE格式大小
	WAVE_FORMAT wavFormat;
};

//数据块
struct DATA_BLOCK
{
	char  szDataID[4]; // data
	DWORD  dwDataSize;
};


//初始化
bool initRecord();
//录音
void record();
DWORD CALLBACK waveInProc(HWAVEIN, UINT, DWORD, DWORD, DWORD);
void closeRecord();
//获取状态
char* getWavContent();
int getWavSize();

