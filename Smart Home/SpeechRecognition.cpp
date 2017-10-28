/*
* ������д(iFly Auto Transform)�����ܹ�ʵʱ�ؽ�����ת���ɶ�Ӧ�����֡�
*/
#include"speechRecognition.h"
#ifdef _WIN64
#pragma comment(lib,"./libs/msc_x64.lib") //x64
#else
#pragma comment(lib,"./libs/msc.lib") //x86
#endif

#define	BUFFER_SIZE	4096
#define FRAME_LEN	640 
#define HINTS_SIZE  100

char rec_result[BUFFER_SIZE] = { NULL };

//����ʶ���ʼ��
bool initSpeRec()
{
	int			ret = MSP_SUCCESS;
	//��¼����
	const char* login_params = "appid = 53bf8448, work_dir = .";
	
	//��½Ѷ��
	ret = MSPLogin(NULL, NULL, login_params); 	
	if (MSP_SUCCESS != ret)
	{
		//��½ʧ��
		_getch();
		MSPLogout(); //�˳���¼
		return false;
	}

	printf("�ϴ��û��ʱ� ...\n");
	//�ϴ��û��ʱ�
	ret = upload_userwords();
	if (MSP_SUCCESS != ret)
	{
		//�ϴ�ʧ��
		_getch();
		MSPLogout();
		return false;
	}
	else printf("�ϴ��û��ʱ�ɹ�\n");
	return true;
}

//�ϴ��û��ʱ�
int upload_userwords()
{
	char*			userwords = NULL;
	unsigned int	len = 0;
	unsigned int	read_len = 0;
	FILE*			fp = NULL;
	int				ret = -1;

	//��
	fp = fopen("userwords.txt", "rb");
	if (NULL == fp)
	{
		printf("�û��ʱ����ڣ� \n");
		return -1;
	}

	//��ȡ�ʱ��ļ���С
	fseek(fp, 0, SEEK_END);
	len = ftell(fp); 
	fseek(fp, 0, SEEK_SET);

	//����洢�ʱ�ռ�
	userwords = (char*)malloc(len + 1);
	if (NULL == userwords)
	{
		printf("out of memory! \n");
		return -1;
	}
	//��ȡ�û��ʱ�����
	read_len = fread((void*)userwords, 1, len, fp); 
	if (read_len != len)
	{
		printf("read [userwords.txt] failed!\n");
		return -1;
	}
	userwords[len] = '\0';

	//�ϴ��û��ʱ�
	MSPUploadData("userwords", userwords, len, "sub = uup, dtt = userword", &ret); 
	if (MSP_SUCCESS != ret)
	{
		printf("MSPUploadData failed ! errorCode: %d \n", ret);
		return -1;
	}

	return ret;
}

//��������ʶ��
void run_iat(char *content, const char* session_begin_params,int size)
{
	const char*	session_id = NULL;	
	char hints[HINTS_SIZE] = { NULL }; 
	int total_len = 0;
	//����״̬
	int	aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
	//�˵�
	int	ep_stat = MSP_EP_LOOKING_FOR_SPEECH;
	//ʶ��״̬
	int	rec_stat = MSP_REC_STATUS_SUCCESS;
	int	errcode = MSP_SUCCESS;
	FILE* f_pcm = NULL;
	char* p_pcm = NULL;
	long pcm_count = 0;
	long pcm_size = 0;
	long read_size = 0;

	//�ÿս��
	rec_result[0]='\0';
	//��ȡ��ʶ�������С
	p_pcm = content;
	pcm_size = size;
	printf("��ʼ����ʶ�� ...\n");

	//��ʼ�����Ự���ϴ��Ự����
	session_id = QISRSessionBegin(NULL, session_begin_params, &errcode);
	if (MSP_SUCCESS != errcode)
		return;

	//��ʼѭ��ʶ��
	while (1)
	{
		//ÿ��д��10*640B=6.4KB
		unsigned int len = 10 * FRAME_LEN;
		int ret = 0;

		if (pcm_size < 2 * len)
			len = pcm_size;
		if (len <= 0)
			break;

		//����״̬
		aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
		if (0 == pcm_count)
			aud_stat = MSP_AUDIO_SAMPLE_FIRST;

		printf(">");
		//�ϴ������������鿴ʶ���Ƿ����½��
		ret = QISRAudioWrite(session_id, (const void *)&p_pcm[pcm_count], len, aud_stat, &ep_stat, &rec_stat);
		if (MSP_SUCCESS != ret)
			return;

		pcm_count += (long)len;
		pcm_size -= (long)len;

		//���µ���д���
		if (MSP_REC_STATUS_SUCCESS == rec_stat) 
		{
			//�����½��
			const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
			if (MSP_SUCCESS != errcode)
				return;

			if (NULL != rslt)
			{
				unsigned int rslt_len = strlen(rslt);
				total_len += rslt_len;
				if (total_len >= BUFFER_SIZE)
					return;
				//���½�����Ƶ����������
				strncat(rec_result, rslt, rslt_len);
			}
		}

		if (MSP_EP_AFTER_SPEECH == ep_stat)
			break;
	}

	//�ϴ���������
	errcode = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
	if (MSP_SUCCESS != errcode)
		return;

	//������ȡʶ������ֱ������
	while (MSP_REC_STATUS_COMPLETE != rec_stat)
	{
		const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
		if (MSP_SUCCESS != errcode)
			return;

		if (NULL != rslt)
		{
			unsigned int rslt_len = strlen(rslt);
			total_len += rslt_len;
			if (total_len >= BUFFER_SIZE)
				return;
			strncat(rec_result, rslt, rslt_len);
		}
	}
	printf("\n");

	//���������Ự
	QISRSessionEnd(session_id, hints);
}

//����ʶ��
int speechRecognition(char* content,int size)
{
	//������������ʶ�����
	const char* session_begin_params = "sub = iat, domain = iat, language = zh_ch, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = gb2312";
	run_iat(content, session_begin_params, size);
	return 0;
}

//���ؽ��
char* getRecReslut()
{
	return rec_result;
}