#pragma once
#include <pthread.h>
#include <string>
#include "zlib.h"

using namespace std;
class CProcessData
{
public:
CProcessData();
~CProcessData();
bool IsLogOpen();
bool IsLogExist(const string strLogPath);
int  OpenLog(const string strLogPath);
int  WriteLog(string strLogMsg);
void  CloseLog();
int  ProcessData(char *data,char *data_out,unsigned long *outlen);
private:
int gzdecompress(Byte *zdata, uLong nzdata,Byte *data, uLong *ndata);
int httpgzdecompress(Byte *zdata, uLong nzdata,Byte *data, uLong *ndata);
private:
	FILE* m_fp;
	bool  m_bIsAlreadyOpen;
	pthread_rwlock_t	m_rwlock;

};
