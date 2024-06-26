#include "ComDrive.h"
#include "Define.h"
#include <time.h>
#include <string.h>
#include "util.h"
#include "logger.h"
#include <utils/ErrCode.h>
#include "ComPack.h"
using namespace toolkit;

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
		LogE("serviceFrame2Array failed ret =%d",ret);
		return ERR_FAIL;
	}
	index += ret;
	crc = Calc_CRC_CCITT(RawPack, index);
	RawPack[index] = (crc >> 8);//0x79;
	index += 1;
	RawPack[index] = (crc & 0xFF);//0x83;
	index += 1;
	LogE("%s", hexdump(reinterpret_cast<void *>(RawPack), index).c_str());
	int len = com_send(comPort, reinterpret_cast<char *>(RawPack), index);
	if(len!= index)
	{
		LogE("com_send failed ret =%d need =%d",len,index);
	    return ERR_IOSEND;
	}
	return ERR_OK;
}
int sendAck(int comPort)
{
	unsigned char ackBuf[7] = {0xaa,0x00,0x07,0x05,0x01,0x17,0x82};
	int len = com_send(comPort, reinterpret_cast<char *>(ackBuf), 7);
	if(len!= 7)
	{
		LogE("com_send failed ret =%d need =%d",len,7);
		return ERR_IOSEND;
	}
	return ERR_OK;
}

int analysisAck(unsigned char* data,int len)
{
	if(NULL == data)
	{
		return ERR_FAIL;
	}
	if(len < 7)
	{
		return ERR_FAIL;
	}
	int idx = 0;
	int packLen = 0;
	unsigned short crc = 0;
	if(data[idx] != PACK_START_FLAG)
	{
		LogE("PACK_START_FLAG ERR");
		return ERR_FAIL;
	}
	idx++;
	packLen = (data[idx] << 8) + data[idx+1];
	if (packLen != len)
	{
		LogE("len ERR");
		return ERR_FAIL;
	}
	idx+=2;
	if (data[idx] != SERVICE_ID_RECV)
	{
		LogE("SERVICE_ID_RECV ERR");
		return ERR_FAIL;
	}
	idx++;
	if (data[idx] != RESPONS_SUCCESS)
	{
		LogE("RESPONS ERR");
		return ERR_FAIL;
	}
	idx++;
	crc = Calc_CRC_CCITT(data, len-2);
	if(data[idx] != (crc >> 8))
	{
		LogE("crc ERR");
		return ERR_FAIL;
	}
	idx++;
	if(data[idx] != (crc & 0xFF))
	{
		LogE("crc ERR");
		return ERR_FAIL;
	}
	return ERR_OK;
}

int analysisFrame(unsigned char* data,unsigned char* recvdata,int* recvdataLen)
{
	int frameLen =0;
	int idx = 0;
	frameLen = data[idx];
	if(frameLen < 7 || frameLen >255)
	{
		LogE("frameLen failed len =%d",frameLen);
		return ERR_FAIL;
	}
	*recvdataLen = frameLen - 7;
	memcpy(recvdata,data+7,*recvdataLen);
	LogE("RECV0 =%d ",recvdata[0]);
	LogE("frameLen =%d ",frameLen);
	LogE("%s", hexdump(reinterpret_cast<void *>(data), frameLen).c_str());
	return ERR_OK;
}

int analysisPack(unsigned char* data,int dataLen,unsigned char* recvData,int* recvDataLen)
{
	if(NULL == recvData || NULL == data)
	{
		return ERR_FAIL;
	}
	if(dataLen < 7)
	{
		return ERR_FAIL;
	}
	int ret = 0;
	int idx = 0;
	int packLen = 0;
	unsigned short crc = 0;
	if(data[idx] != PACK_START_FLAG)
	{
		LogE("PACK_START_FLAG ERR,start =%u",data[idx]);
		return ERR_FAIL;
	}
	idx++;
	packLen = (data[idx] << 8) + data[idx+1];
//	if (packLen != len)
//	{
//		DEBUG_LOGCATE("len ERR");
//		return ERR_FAIL;
//	}
	idx+=2;
	if (!(data[idx] == SERVICE_ID_RECV || data[idx] == SERVICE_ID_SEND) )
	{
		LogE("SERVICE_ID_RECV ERR");
		return ERR_FAIL;
	}
	idx++;
	ret = analysisFrame(data+idx,recvData,recvDataLen);
	if (ret != ERR_OK)
	{
		LogE("analysisFrame ERR ret =%d",ret);
		return ERR_FAIL;
	}

	crc = Calc_CRC_CCITT(data, packLen-2);
	if(data[packLen-2] != (crc >> 8))
	{
		LogE("crc ERR data =%u %u",data[packLen-2],data[packLen-1]);
		LogE("crc ERR crc =%u %u",crc >> 8,(crc & 0xFF));
		return ERR_FAIL;
	}
	if(data[packLen-1] != (crc & 0xFF))
	{
		LogE("crc ERR");
		return ERR_FAIL;
	}
	return ERR_OK;
}



















