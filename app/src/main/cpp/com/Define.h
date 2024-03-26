
#ifndef __TMF20_ERR_CODE_H__
#define __TMF20_ERR_CODE_H__

/************************************************************************
 ----------------------  常规定义  --------------------------------------
 ********************************************************************** */
#define SHORT_TIME_OUT       5000
#define COM_PATH             "/dev/ttyS7"
#define BAUND_RATE           119200

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
    SERVICE_ID_SEND   = 0x11,
}SERVICE_ID;

/************************************************************************
 ----------------------  错误码  --------------------------------------
 ********************************************************************** */
#define  ERR_OK            				0			//Success
#define  ERR_FAIL            			-1			//failed
#define  ERR_IOSEND						10			//Send data error
#define  ERR_IORECV						11			//Receive data error
#define  ERR_HEAD_FLAG					12			//Package head error
#define  ERR_END_FLAG					13			//Package end error
#define  ERR_CRC		  				14			//Check sum error
#define  ERR_CANCEL						38  		//Cancel error
#define  ERR_TIME_OUT					41  		//Command timeout
#define  ERR_SET_COM			        46
#define  ERR_NO_DEV		  	        	100      	//No device, or Open device error
#define  ERR_NO_PERMISION         		101      	//No permission
#define  ERR_NO_CONTEXT    	       		102      	//No context
#define  ERR_OPEN			        	ERR_NO_DEV

#endif

