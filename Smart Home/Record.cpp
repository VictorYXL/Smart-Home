#include"Record.h"

static bool  IsRecord = false;//停止标志
int tmpTime;
WAVEFORMATEX  m_soundFormat;//声音格式
HWAVEIN     m_hWaveIn;
WAVEHDR     m_pWaveHdrIn[3];
//缓冲区
CHAR     m_cBufferIn[3][MAX_SOUNDSIZE];
//待识别区
char tmp[MAX_BUFFERSIZE];
int cou;

//录音初始化
bool initRecord()
{
	//获取录音设备
	int n = waveInGetNumDevs(), i;
	printf("%d", n);
	if (n<1)
		return false;

	//设置声音格式
	memset(&m_soundFormat, 0, sizeof(m_soundFormat));
	m_soundFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_soundFormat.nChannels = 2;
	m_soundFormat.nSamplesPerSec = 8000;
	m_soundFormat.nAvgBytesPerSec = 8000 * 2 * 16 / 8;
	m_soundFormat.nBlockAlign = 2 * 16 / 8;
	m_soundFormat.wBitsPerSample = 16;
	m_soundFormat.cbSize = 0;

	MMRESULT m_res;
	//打开设备
	m_res = waveInOpen(&m_hWaveIn, WAVE_MAPPER, &m_soundFormat, (DWORD)(waveInProc), 0, CALLBACK_FUNCTION);
	if (m_res != 0)
	{
		//打开失败
		return false;
	}

	unsigned int id;
	//选择输入设备，默认-1
	waveInGetID(m_hWaveIn, &id);
	
	//设置内存块格式
	for (i = 0; i<3; i++) 
	{
		m_pWaveHdrIn[i].lpData = m_cBufferIn[i];
		m_pWaveHdrIn[i].dwBufferLength = MAX_SOUNDSIZE;
		m_pWaveHdrIn[i].dwBytesRecorded = 0;
		m_pWaveHdrIn[i].dwUser = i;
		m_pWaveHdrIn[i].dwFlags = 0;

		//准备内存块录音
		waveInPrepareHeader(m_hWaveIn, &m_pWaveHdrIn[i], sizeof(WAVEHDR)); 
		//增加内存块
		waveInAddBuffer(m_hWaveIn, &m_pWaveHdrIn[i], sizeof(WAVEHDR)); 
	}

	//开始获取pcm声音
	waveInStart(m_hWaveIn);
	return true;
}
void record()
{
	//待识别区写入位置，44B为预留的wav头
	cou = 44;
	IsRecord = true;

	//计时
	tmpTime = clock();
	printf("开始录音...\n");

}

//会调函数，当一个内存块满时，自动调用
DWORD CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg,
	DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg == WIM_DATA)
	{
		//WAVEHDR的地址
		WAVEHDR*p = (WAVEHDR*)dwParam1;
		int i = p->dwUser;

		//是否复制到待识别区
		if (IsRecord) 
		{
			//识别时间
			int px = clock()- tmpTime;
			printf("%ds  ", px / 1000);
			cou = cou < MAX_BUFFERSIZE ? cou : 0;

			//从缓冲区复制到
			for (int k = 0; k < MAX_SOUNDSIZE; k++)
				tmp[cou + k] = m_cBufferIn[i][k];
			cou += MAX_SOUNDSIZE;

			//释放
			waveInUnprepareHeader(m_hWaveIn, p, sizeof(WAVEHDR)); 
		}
		p->lpData = m_cBufferIn[i];
		p->dwBufferLength = MAX_SOUNDSIZE;
		p->dwBytesRecorded = 0;
		p->dwUser = i;
		p->dwFlags = 0;

		//准备内存块录音
		waveInPrepareHeader(m_hWaveIn, p, sizeof(WAVEHDR)); 
		//增加内存块
		waveInAddBuffer(m_hWaveIn, p, sizeof(WAVEHDR)); 
	}
	if (uMsg == WIM_OPEN)
	{
		printf("录音设备已打开\n");
	}
	if (uMsg == WIM_CLOSE)
	{
		printf("录音设备已关闭\n");
	}
	return 0;
}

//关闭设备，释放空间
void closeRecord()
{
	IsRecord = false;
	printf("正在停止录音...\n");
	waveInStop(m_hWaveIn);
	printf("正在关闭录音设备...\n");
	waveInClose(m_hWaveIn);
}

//获取语音内容，pcm-〉wav
char* getWavContent()
{
	IsRecord = false;
	printf("正在转化格式...\n");
	long filesize = 0;
	long n = 0;
	filesize = cou-44;
	RIFF_HEADER m_riff = { 0 };
	FMT_BLOCK m_fmt = { 0 };
	DATA_BLOCK m_data = { 0 };

	//设置文件头
	//RIFF
	strncpy(m_riff.RIFF, "RIFF", 4);
	m_riff.RSize = 4 + sizeof(FMT_BLOCK) + sizeof(DATA_BLOCK) + filesize;
	strncpy(m_riff.WAVE, "WAVE", 4);
	//fmt
	strncpy(m_fmt.szFmtID, "fmt ", 4);
	m_fmt.dwFmtSize = sizeof(WAVE_FORMAT);
	m_fmt.wavFormat = *(WAVE_FORMAT*)&m_soundFormat;
	//Data
	strncpy(m_data.szDataID, "data", 4);
	m_data.dwDataSize = filesize;

	//写入文件头
	memcpy(tmp, &m_riff, sizeof(RIFF_HEADER));
	memcpy(tmp + sizeof(RIFF_HEADER), &m_fmt, sizeof(FMT_BLOCK));
	memcpy(tmp + sizeof(RIFF_HEADER) + sizeof(FMT_BLOCK), &m_data, sizeof(DATA_BLOCK));

	return tmp;
}

//返回音频数据大小
int getWavSize()
{
	return cou;
}