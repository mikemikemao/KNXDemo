#include "ComDrive.h"
#include "Define.h"
#include <time.h>
#include <string.h>
#include <utils/LogUtil.h>
#include <utils/ToolUnits.h>
#include "ComPack.h"

int serviceFrame2Array(ServiceFrame* serviceFrame,unsigned char* pack)
{
	int index = 0;
	if(NULL == serviceFrame || NULL == pack)
	{
		return ERR_FAIL;
	}
	*(pack+index) = serviceFrame->size;
	index += 1;
	//预留2个
	index += 2;
	*(pack+index) = serviceFrame->controlType;
	index += 1;
	*(pack+index) = serviceFrame->property;
	index += 1;
	*(pack+index) = serviceFrame->channel;
	index += 1;
	*(pack+index) = serviceFrame->controlStatus;
	index += 1;
	memcpy(pack+index,serviceFrame->data,serviceFrame->size - SERVICE_FRAME_HEAD);
	index += serviceFrame->size - SERVICE_FRAME_HEAD;
	return index;
}

unsigned short Get_CRC_CCITT(unsigned short u_crc_val, unsigned char btVal)
{
	u_crc_val = ((unsigned char)(u_crc_val >> 8)) | (u_crc_val << 8);
	u_crc_val ^= btVal;
	u_crc_val ^= ((unsigned char)(u_crc_val & 0xFF)) >> 4;
	u_crc_val ^= u_crc_val << 12;
	u_crc_val ^= (u_crc_val & 0xFF) << 5;
	return u_crc_val;
}

//CRC−CCITT value over a buffer of bytes can be calculated with following code
//pBuf is pointer to the start of frame buffer
//uLength is the frame length in bytes
unsigned short Calc_CRC_CCITT(unsigned char* pBuf, unsigned short uLength)
{
	unsigned short u_crc_ccitt;
	for (u_crc_ccitt = 0xFFFF; uLength--; pBuf++)
	{
		u_crc_ccitt = Get_CRC_CCITT(u_crc_ccitt, *pBuf);
	}
	return u_crc_ccitt;
}

int SendPacketIn(int comPort, ServiceFrame* serviceFrame)
{
	unsigned char RawPack[MAX_PACK_SZIE];
	char nCheckSum = 0;
	int ret = 0;
	int index = 0;
	int packLen = 0;
	unsigned short crc = 0;
	memset(RawPack, 0, sizeof(RawPack));
	if(NULL == serviceFrame)
	{
		return ERR_FAIL;
	}
	packLen = serviceFrame->size + PACK_FRAME_LEFT;

	RawPack[index] = PACK_START_FLAG;
	index += 1;
	RawPack[index] = (packLen>>8);
	index += 1;
	RawPack[index] = (packLen & 0xFF);
	index += 1;
	RawPack[index] = SERVICE_ID_SEND;
	index += 1;
	ret = serviceFrame2Array(serviceFrame,RawPack + index);
	if(ret < ERR_OK)
	{
		LOGCATE("serviceFrame2Array failed ret =%d",ret);
		return ERR_FAIL;
	}
	index += ret;
	unsigned char tmp1[5] ={0xaa,0x00,0x07,0x05,0x00};
	unsigned char tmp[12] = {0xaa, 0x00, 0x0e,0x11,0x08,0x00,0x00,0x01 ,0x00 ,0x00 ,0x00 ,0x01};
	crc = Calc_CRC_CCITT(tmp, 12);
	LOGCATE("CRC=%x",crc);
	RawPack[index] = 0x79;//(crc >> 8);//0x79;
	index += 1;
	RawPack[index] = 0x83;//(crc & 0xFF);//0x83;
	index += 1;
    LOGCATE("%s", hexdump(reinterpret_cast<void *>(RawPack), index).c_str());
	int len = com_send(comPort, reinterpret_cast<char *>(RawPack), index);
	if(len!= index)
	{
		LOGCATE("com_send failed ret =%d need =%d",len,index);
	    return ERR_IOSEND;
	}
	return ERR_OK;
}






















