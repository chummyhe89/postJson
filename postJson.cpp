#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <fcgiapp.h>
//#include <fcgi_stdio.h>
#include <sys/time.h>
#include <string>
#include <iostream>
#include "ProcessData.h"

#include "test/gzip.cpp"

#define LOG_FULL_PATH	"/data/logs/listen_client/listen_client.log"
#define BUF_SIZE	1024*1024
#define THREAD_NUM	8
#define FIRST_SEVEN_CHARS	"{\"data\""
using namespace std;
string safeGetEnv(const char* name, FCGX_Request* request);
class CPostJson_cgi
{
public:
	CPostJson_cgi(){};
	~CPostJson_cgi(){};
	int StartNewThread(pthread_t& tid);
private:
	static void* Thread_fun(void* arg);
	CProcessData	m_proData;
};

void ltrim(char * s)
{
	char *p;
	p = s;
	while(' ' == *p || '\t' == *p || '\n' == *p || '\r' == *p){p++;}
	strcpy(s,p);
}

void rtrim(char *s)
{
	int i;
	i = strlen(s)-1;
	while(i >= 0 &&(' ' == s[i] || '\t' == s[i] || '\n' == s[i] || '\r' == s[i])){ i--;}
	s[i+1]='\0';
	 
}
void trim(char *s)
{
	ltrim(s);
	rtrim(s);
}
void* CPostJson_cgi::Thread_fun(void* args)
{		
	char first_chars[10] = FIRST_SEVEN_CHARS;
	FCGX_Request	fcgi_req;
	CPostJson_cgi* pPostJson_cgi = (CPostJson_cgi*)args;
	FCGX_InitRequest(&fcgi_req,0,0);
	while(FCGX_Accept_r(&fcgi_req) >= 0)
	{	
		int code = 1;
		char tempcode[127];
		string strMsg="ok";		
		string respJson;
		string strOut;
		string strReqType;
		int length = 0;
		unsigned long outlen = BUF_SIZE;
		char data_out[BUF_SIZE];
		memset(data_out,0,BUF_SIZE);
		strReqType=safeGetEnv("REQUEST_METHOD",&fcgi_req);
		strOut=safeGetEnv("REMOTE_ADDR",&fcgi_req)+"`" \
			+safeGetEnv("REMOTE_PORT",&fcgi_req)+"`" \
			+safeGetEnv("REQUEST_URI",&fcgi_req)+"`"+strReqType+"`"\
			+safeGetEnv("QUERY_STRING",&fcgi_req)+"`";
		 
		length=atoi(FCGX_GetParam("CONTENT_LENGTH",fcgi_req.envp));
		if("POST"==strReqType)
		{	
			char buffer[length+1];
		//	memset(buffer,0,length+1);
			FCGX_GetStr(buffer,length+1,fcgi_req.in);
			buffer[length]='\0';
			if(strncmp(first_chars,buffer,7))
			{
				int iRet	= 0;
				//if((*pPostJson_cgi).m_proData.ProcessData(buffer,data_out,&outlen) != 0)
				if((iRet=httpgzdecompress((Byte*)buffer,length+1,(Byte*)data_out,&outlen)) != 0)
					{
					code = 0;
					strMsg = "error";
					continue;
					}
			}
			else
			{
			strcpy(data_out,buffer);
			}
			//trim
			trim(data_out);
			strOut+=string(data_out);
			cout<<strOut<<endl;
		}
		if( !((*pPostJson_cgi).m_proData.IsLogExist(LOG_FULL_PATH)) || !((*pPostJson_cgi).m_proData.IsLogOpen()) )
		{	
			
			int iRet	= 0;
			if((iRet=(*pPostJson_cgi).m_proData.OpenLog(LOG_FULL_PATH)) != 0)
				throw iRet;
		}
		(*pPostJson_cgi).m_proData.WriteLog(strOut);
		sprintf(tempcode,"%d",code);
		respJson="{\"code\":"+string(tempcode)+",\"msg\":\""+string(strMsg)+"\"}";
		FCGX_FPrintF(fcgi_req.out,"Content-type:text/html\r\n\r\n%s",respJson.c_str());
	//	FCGX_FPrintF(fcgi_req.err,"%s",strOut.c_str());
		FCGX_Finish_r(&fcgi_req);
		
	}
	return (void*)NULL;
}
string safeGetEnv(const char* name, FCGX_Request* request)
{	
	const char* ptr=FCGX_GetParam(name,request->envp);
	if(NULL == ptr)
	{
		return "";
	}
	else
	{
		return string(ptr);
	}

}
int CPostJson_cgi::StartNewThread(pthread_t& tid)
{
	int iRet	= 0;
	iRet = pthread_create(&tid,NULL,Thread_fun,(void*)this);
	return iRet;
}

int main(int argc,char *argv[])
{
	int iRet	= 0;
	int* puThreadExitCode	= 0;
	pthread_t* pTid = NULL;
	CPostJson_cgi	postJson_cgi;
	FCGX_Init();
	pTid = new pthread_t[THREAD_NUM];
	if(NULL == pTid)
	{
		return -1;
	}
	for(int i = 0;i < THREAD_NUM;i++)
	{
		if(iRet=postJson_cgi.StartNewThread(pTid[i]))
			return iRet;
	}
	for(int i =0;i < THREAD_NUM;i++)
	{
		pthread_join(pTid[i],(void**)&puThreadExitCode);
	}
	delete[] pTid;
	return 0;
}
