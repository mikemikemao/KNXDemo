#include <stdint.h>
#include <string.h>
#include <utils/LogUtil.h>
#include <utils/ToolUnits.h>
#include "ComPack.h"
#include "Define.h"
#include "ComFunc.h"
#include "ComDrive.h"

int comLightControl(unsigned char state)
{
    int lRV=-1;
	int port_h= 0;
    char errmsg[64];
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
    lRV = SendPacketIn(port_h,&serviceFrame);
	if(lRV < 0)
	{
		com_close(port_h);
		return ERR_IOSEND;
	}
	lRV= com_wait(port_h, SHORT_TIME_OUT);
	if(lRV!=0)
	{
        com_close(port_h);
        return ERR_TIME_OUT;
	}
	int len=100;
	char RecvBuff[100]={0};
	len = com_recv(port_h,RecvBuff,len);
	LOGCATE("%s", hexdump(reinterpret_cast<void *>(RecvBuff), len).c_str());
	com_close(port_h);
	if(len<=0)
	{
		return ERR_IORECV;
	}
	return 0;
	
}











