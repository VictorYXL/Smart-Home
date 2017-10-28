/*
* 语音听写(iFly Auto Transform)技术能够实时地将语音转换成对应的文字。
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

//语音识别初始化
bool initSpeRec()
{
	int			ret = MSP_SUCCESS;
	//登录参数
	const char* login_params = "appid = 53bf8448, work_dir = .";
	
	//登陆讯飞
	ret = MSPLogin(NULL, NULL, login_params); 	
	if (MSP_SUCCESS != ret)
	{
		//登陆失败
		_getch();
		MSPLogout(); //退出登录
		return false;
	}

	printf("上传用户词表 ...\n");
	//上传用户词表
	ret = upload_userwords();
	if (MSP_SUCCESS != ret)
	{
		//上传失败
		_getch();
		MSPLogout();
		return false;
	}
	else printf("上传用户词表成功\n");
	return true;
}

//上传用户词表
int upload_userwords()
{
	char*			userwords = NULL;
	unsigned int	len = 0;
	unsigned int	read_len = 0;
	FILE*			fp = NULL;
	int				ret = -1;

	//打开
	fp = fopen("userwords.txt", "rb");
	if (NULL == fp)
	{
		printf("用户词表不存在！ \n");
		return -1;
	}

	//获取词表文件大小
	fseek(fp, 0, SEEK_END);
	len = ftell(fp); 
	fseek(fp, 0, SEEK_SET);

	//申请存储词表空间
	userwords = (char*)malloc(len + 1);
	if (NULL == userwords)
	{
		printf("out of memory! \n");
		return -1;
	}
	//读取用户词表内容
	read_len = fread((void*)userwords, 1, len, fp); 
	if (read_len != len)
	{
		printf("read [userwords.txt] failed!\n");
		return -1;
	}
	userwords[len] = '\0';

	//上传用户词表
	MSPUploadData("userwords", userwords, len, "sub = uup, dtt = userword", &ret); 
	if (MSP_SUCCESS != ret)
	{
		printf("MSPUploadData failed ! errorCode: %d \n", ret);
		return -1;
	}

	return ret;
}

//运行语音识别
void run_iat(char *content, const char* session_begin_params,int size)
{
	const char*	session_id = NULL;	
	char hints[HINTS_SIZE] = { NULL }; 
	int total_len = 0;
	//语音状态
	int	aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
	//端点
	int	ep_stat = MSP_EP_LOOKING_FOR_SPEECH;
	//识别状态
	int	rec_stat = MSP_REC_STATUS_SUCCESS;
	int	errcode = MSP_SUCCESS;
	FILE* f_pcm = NULL;
	char* p_pcm = NULL;
	long pcm_count = 0;
	long pcm_size = 0;
	long read_size = 0;

	//置空结果
	rec_result[0]='\0';
	//获取待识别区与大小
	p_pcm = content;
	pcm_size = size;
	printf("开始语音识别 ...\n");

	//开始语音会话，上传会话参数
	session_id = QISRSessionBegin(NULL, session_begin_params, &errcode);
	if (MSP_SUCCESS != errcode)
		return;

	//开始循环识别
	while (1)
	{
		//每次写入10*640B=6.4KB
		unsigned int len = 10 * FRAME_LEN;
		int ret = 0;

		if (pcm_size < 2 * len)
			len = pcm_size;
		if (len <= 0)
			break;

		//定义状态
		aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
		if (0 == pcm_count)
			aud_stat = MSP_AUDIO_SAMPLE_FIRST;

		printf(">");
		//上传本段语音，查看识别是否有新结果
		ret = QISRAudioWrite(session_id, (const void *)&p_pcm[pcm_count], len, aud_stat, &ep_stat, &rec_stat);
		if (MSP_SUCCESS != ret)
			return;

		pcm_count += (long)len;
		pcm_size -= (long)len;

		//有新的听写结果
		if (MSP_REC_STATUS_SUCCESS == rec_stat) 
		{
			//下载新结果
			const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
			if (MSP_SUCCESS != errcode)
				return;

			if (NULL != rslt)
			{
				unsigned int rslt_len = strlen(rslt);
				total_len += rslt_len;
				if (total_len >= BUFFER_SIZE)
					return;
				//将新结果复制到完整结果中
				strncat(rec_result, rslt, rslt_len);
			}
		}

		if (MSP_EP_AFTER_SPEECH == ep_stat)
			break;
	}

	//上传最后的语音
	errcode = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
	if (MSP_SUCCESS != errcode)
		return;

	//继续获取识别结果，直到结束
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

	//结束语音会话
	QISRSessionEnd(session_id, hints);
}

//回音识别
int speechRecognition(char* content,int size)
{
	//设置语音语音识别参数
	const char* session_begin_params = "sub = iat, domain = iat, language = zh_ch, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = gb2312";
	run_iat(content, session_begin_params, size);
	return 0;
}

//返回结果
char* getRecReslut()
{
	return rec_result;
}