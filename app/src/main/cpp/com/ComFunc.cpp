#include <stdint.h>
#include <string.h>
#include <utils/LogUtil.h>
#include <utils/ToolUnits.h>
#include <utils/ErrCode.h>
#include "ComPack.h"
#include "Define.h"
#include "ComFunc.h"
#include "ComDrive.h"

static int recAck(int port_h)
{
    int ret = 0;
    int len = 0;
    char ackBuff[7] = {0};
    ret= com_wait(port_h, SHORT_TIME_OUT);
    if(ret!=0)
    {
        return ERR_TIME_OUT;
    }
    //收ACK
    len = com_recv(port_h,ackBuff,ACK_LEN);
    LOGCATE("%s", hexdump(reinterpret_cast<void *>(ackBuff), len).c_str());
    if(len<=0)
    {
        return ERR_IORECV;
    }
    ret = analysisAck((unsigned char*)ackBuff,len);
    if(ret != 0)
    {
        LOGCATE("analysisPack err");
        return ERR_FAIL;
    }

    return ERR_OK;
}

static int recBuff(int port_h,unsigned char* data,int* dataLen)
{
    int ret = 0;
    int len=100;
    char RecvBuff[100]={0};
    ret= com_wait(port_h, SHORT_SHORT_TIME_OUT);
    if(ret!=0)
    {
        return ERR_TIME_OUT;
    }
    //收ACK
    len = com_recv(port_h,RecvBuff,len);
    LOGCATE("%s", hexdump(reinterpret_cast<void *>(RecvBuff), len).c_str());
    if(len<=0)
    {
        return ERR_IORECV;
    }
    ret = analysisPack((unsigned char*)RecvBuff,len,data,dataLen);
    if(ret != 0)
    {
        LOGCATE("analysisPack err");
        return ERR_FAIL;
    }
    return ERR_OK;
}

int comLightControl(unsigned char state)
{
    int ret =-1;
	int port_h= 0;
    char errmsg[64];
	int len=100;
    unsigned char recvBuff[100]={0};
    ServiceFrame serviceFrame;
    memset(&serviceFrame,0x00,sizeof(ServiceFrame));

    port_h = com_open(COM_PATH, BAUND_RATE, errmsg);
    if (port_h < 0) {
    	LOGCATE("com_open failed ret=%d msg=%s",port_h,errmsg);
    	return -1;
    }
    serviceFrame.size = 8;
    serviceFrame.controlType = LIGHT_CONTROL;
    serviceFrame.property = OPEN_CLOSE;
    serviceFrame.channel = CHANNEL_0;
    serviceFrame.controlStatus = CONTROL_MSG;
    serviceFrame.data[0] = (state == 0) ? 0 : 1;
    ret = SendPacketIn(port_h,&serviceFrame);
	if(ret < 0)
	{
		com_close(port_h);
		return ERR_IOSEND;
	}
	ret = recAck(port_h);
	if(ret != ERR_OK)
    {
	    LOGCATE("recAck failed ret =%d",ret);
        com_close(port_h);
        return ERR_FAIL;
    }
    ret = recBuff(port_h,recvBuff,&len);
	if(ret == ERR_TIME_OUT)
    {
        com_close(port_h);
        return ERR_OK;
    }
    if(ret != ERR_OK)
    {
        LOGCATE("recAck recBuff ret =%d",ret);
        com_close(port_h);
        return ERR_FAIL;
    }
    if(state != recvBuff[0])
    {
        LOGCATE("recvBuff is disconstent state =%d recv =%d ",state,recvBuff[0]);
        com_close(port_h);
        return ERR_FAIL;
    }
    ret = sendAck(port_h);
    if(ret < 0)
    {
        com_close(port_h);
        return ERR_IOSEND;
    }
    com_close(port_h);
	return 0;
	
}

int comLightStateControl()
{
	int ret=-1;
	int port_h= 0;
	char errmsg[64];
	int len=100;
	unsigned char recvBuff[100]={0};
	ServiceFrame serviceFrame;
	memset(&serviceFrame,0x00,sizeof(ServiceFrame));
	port_h = com_open(COM_PATH, BAUND_RATE, errmsg);
	if (port_h < 0) {
		LOGCATE("com_open failed ret=%d msg=%s",port_h,errmsg);
		return -1;
	}
	serviceFrame.size = 7;
	serviceFrame.controlType = LIGHT_CONTROL;
	serviceFrame.property = OPEN_CLOSE_STATE;
	serviceFrame.channel = CHANNEL_0;
	serviceFrame.controlStatus = READ_STATE_MSG;
	//serviceFrame.data[0] = (state == 0) ? 0 : 1;
    ret = SendPacketIn(port_h,&serviceFrame);
	if(ret < 0)
	{
		com_close(port_h);
		return ERR_IOSEND;
	}
    ret = recAck(port_h);
    if(ret != ERR_OK)
    {
        LOGCATE("recAck failed ret =%d",ret);
        com_close(port_h);
        return ERR_FAIL;
    }
    ret = recBuff(port_h,recvBuff,&len);
    if(ret == ERR_TIME_OUT)
    {
        com_close(port_h);
        return ERR_OK;
    }
    if(ret != ERR_OK)
    {
        LOGCATE("recAck recBuff ret =%d",ret);
        com_close(port_h);
        return ERR_FAIL;
    }
    ret = sendAck(port_h);
    if(ret < 0)
    {
        com_close(port_h);
        return ERR_IOSEND;
    }
    com_close(port_h);
	return recvBuff[0];
}




