#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <errno.h>

#include "../include/qisr.h"
#include "../include/msp_cmn.h"
#include "../include/msp_errors.h"
#include"Record.h"

//初始化
bool initSpeRec();
int upload_userwords();

//语音识别
void run_iat(char*,const char*,int);
int speechRecognition(char*, int);

//返回结果
char* getRecReslut();