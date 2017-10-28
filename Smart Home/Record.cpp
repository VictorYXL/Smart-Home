#include"Record.h"

static bool  IsRecord = false;//ֹͣ��־
int tmpTime;
WAVEFORMATEX  m_soundFormat;//������ʽ
HWAVEIN     m_hWaveIn;
WAVEHDR     m_pWaveHdrIn[3];
//������
CHAR     m_cBufferIn[3][MAX_SOUNDSIZE];
//��ʶ����
char tmp[MAX_BUFFERSIZE];
int cou;

//¼����ʼ��
bool initRecord()
{
	//��ȡ¼���豸
	int n = waveInGetNumDevs(), i;
	printf("%d", n);
	if (n<1)
		return false;

	//����������ʽ
	memset(&m_soundFormat, 0, sizeof(m_soundFormat));
	m_soundFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_soundFormat.nChannels = 2;
	m_soundFormat.nSamplesPerSec = 8000;
	m_soundFormat.nAvgBytesPerSec = 8000 * 2 * 16 / 8;
	m_soundFormat.nBlockAlign = 2 * 16 / 8;
	m_soundFormat.wBitsPerSample = 16;
	m_soundFormat.cbSize = 0;

	MMRESULT m_res;
	//���豸
	m_res = waveInOpen(&m_hWaveIn, WAVE_MAPPER, &m_soundFormat, (DWORD)(waveInProc), 0, CALLBACK_FUNCTION);
	if (m_res != 0)
	{
		//��ʧ��
		return false;
	}

	unsigned int id;
	//ѡ�������豸��Ĭ��-1
	waveInGetID(m_hWaveIn, &id);
	
	//�����ڴ���ʽ
	for (i = 0; i<3; i++) 
	{
		m_pWaveHdrIn[i].lpData = m_cBufferIn[i];
		m_pWaveHdrIn[i].dwBufferLength = MAX_SOUNDSIZE;
		m_pWaveHdrIn[i].dwBytesRecorded = 0;
		m_pWaveHdrIn[i].dwUser = i;
		m_pWaveHdrIn[i].dwFlags = 0;

		//׼���ڴ��¼��
		waveInPrepareHeader(m_hWaveIn, &m_pWaveHdrIn[i], sizeof(WAVEHDR)); 
		//�����ڴ��
		waveInAddBuffer(m_hWaveIn, &m_pWaveHdrIn[i], sizeof(WAVEHDR)); 
	}

	//��ʼ��ȡpcm����
	waveInStart(m_hWaveIn);
	return true;
}
void record()
{
	//��ʶ����д��λ�ã�44BΪԤ����wavͷ
	cou = 44;
	IsRecord = true;

	//��ʱ
	tmpTime = clock();
	printf("��ʼ¼��...\n");

}

//�����������һ���ڴ����ʱ���Զ�����
DWORD CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg,
	DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg == WIM_DATA)
	{
		//WAVEHDR�ĵ�ַ
		WAVEHDR*p = (WAVEHDR*)dwParam1;
		int i = p->dwUser;

		//�Ƿ��Ƶ���ʶ����
		if (IsRecord) 
		{
			//ʶ��ʱ��
			int px = clock()- tmpTime;
			printf("%ds  ", px / 1000);
			cou = cou < MAX_BUFFERSIZE ? cou : 0;

			//�ӻ��������Ƶ�
			for (int k = 0; k < MAX_SOUNDSIZE; k++)
				tmp[cou + k] = m_cBufferIn[i][k];
			cou += MAX_SOUNDSIZE;

			//�ͷ�
			waveInUnprepareHeader(m_hWaveIn, p, sizeof(WAVEHDR)); 
		}
		p->lpData = m_cBufferIn[i];
		p->dwBufferLength = MAX_SOUNDSIZE;
		p->dwBytesRecorded = 0;
		p->dwUser = i;
		p->dwFlags = 0;

		//׼���ڴ��¼��
		waveInPrepareHeader(m_hWaveIn, p, sizeof(WAVEHDR)); 
		//�����ڴ��
		waveInAddBuffer(m_hWaveIn, p, sizeof(WAVEHDR)); 
	}
	if (uMsg == WIM_OPEN)
	{
		printf("¼���豸�Ѵ�\n");
	}
	if (uMsg == WIM_CLOSE)
	{
		printf("¼���豸�ѹر�\n");
	}
	return 0;
}

//�ر��豸���ͷſռ�
void closeRecord()
{
	IsRecord = false;
	printf("����ֹͣ¼��...\n");
	waveInStop(m_hWaveIn);
	printf("���ڹر�¼���豸...\n");
	waveInClose(m_hWaveIn);
}

//��ȡ�������ݣ�pcm-��wav
char* getWavContent()
{
	IsRecord = false;
	printf("����ת����ʽ...\n");
	long filesize = 0;
	long n = 0;
	filesize = cou-44;
	RIFF_HEADER m_riff = { 0 };
	FMT_BLOCK m_fmt = { 0 };
	DATA_BLOCK m_data = { 0 };

	//�����ļ�ͷ
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

	//д���ļ�ͷ
	memcpy(tmp, &m_riff, sizeof(RIFF_HEADER));
	memcpy(tmp + sizeof(RIFF_HEADER), &m_fmt, sizeof(FMT_BLOCK));
	memcpy(tmp + sizeof(RIFF_HEADER) + sizeof(FMT_BLOCK), &m_data, sizeof(DATA_BLOCK));

	return tmp;
}

//������Ƶ���ݴ�С
int getWavSize()
{
	return cou;
}