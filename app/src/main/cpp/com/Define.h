
#ifndef __TMF20_ERR_CODE_H__
#define __TMF20_ERR_CODE_H__

/************************************************************************
 ----------------------  常规定义  --------------------------------------
 ********************************************************************** */
#define SHORT_TIME_OUT       5000
#define COM_PATH             "/dev/ttyS7"
#define BAUND_RATE           19200

/************************************************************************
 ----------------------  命令字  --------------------------------------
 ********************************************************************** */
#define CMD_ID_READ_DESCRIPT 				0X09 //get version
#define CHANNEL_0                           0

typedef enum
{
    LIGHT_CONTROL = 0x01,
}CONTROL_TYPE;

typedef enum
{
    OPEN_CLOSE = 0x00,
    OPEN_CLOSE_STATE = 0x01,
}LIGHT_PROPERTY;

typedef enum
{
    CONTROL_MSG    = 0x00,
    STATE_MSG      = 0x01,
    READ_STATE_MSG = 0x02,
}CONTROL_STATES;


typedef enum
{
    SERVICE_ID_RECV   = 0x5,
    SERVICE_ID_SEND   = 0x0B,
}SERVICE_ID;

#define MAX_PACK_SZIE           1500
#define MAX_SERVICE_FRAME       255
#define PACK_START_FLAG         0xAA
#define PACK_FRAME_LEFT         6
#define SERVICE_FRAME_HEAD      7
#define RESPONS_SUCCESS         0X01
#define RESPONS_FAIL            0X00

#endif

