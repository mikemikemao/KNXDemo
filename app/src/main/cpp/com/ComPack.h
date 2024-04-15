#ifndef __Com_PACK__
#define __Com_PACK__
#include "Define.h"
typedef struct ServiceFrame
{
    unsigned char size;
    unsigned char controlType;
    unsigned char property;
    unsigned char channel;
    unsigned char controlStatus;
    unsigned char data[MAX_SERVICE_FRAME];
};

int SendPacketIn(int comPort, ServiceFrame* serviceFrame);
int sendAck(int comPort);

int analysisAck(unsigned char* data,int len);

int analysisPack(unsigned char* data,int dataLen,unsigned char* recvData,int* recvDataLen);

int analysisFrame(unsigned char* data,unsigned char* recvdata,int* recvdataLen);
#endif

