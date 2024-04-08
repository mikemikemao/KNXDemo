//
// Created by maochaoqun on 2024/4/8.
//

#ifndef KNXDEMO_ERRCODE_H
#define KNXDEMO_ERRCODE_H

typedef unsigned char byte;

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

#endif //KNXDEMO_ERRCODE_H
