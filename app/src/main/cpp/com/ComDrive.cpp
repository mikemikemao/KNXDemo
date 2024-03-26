
#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "ComDrive.h"

int BaudRateflag(int iBaudrate)
{
	int iBaudrateflag =B9600;
	switch (iBaudrate)
	{
	case 300:
		iBaudrateflag = B300;
		break;
	case 600:
		iBaudrateflag = B600;
		break;
	case 1200:
		iBaudrateflag = B1200;
		break;
	case 1800:
		iBaudrateflag = B1800;
		break;
	case 2400:
		iBaudrateflag = B2400;
		break;
	case 4800:
		iBaudrateflag = B4800;
		break;
	case 9600:
		iBaudrateflag = B9600;
		break;
	case 19200:
		iBaudrateflag = B19200;
		break;
	case 38400:
		iBaudrateflag = B38400;
		break;
	case 57600:
		iBaudrateflag = B57600;
		break;
	case 115200:
		iBaudrateflag = B115200;
		break;
	default:
		iBaudrateflag =B9600;
		break;
	}
	return iBaudrateflag;
}
/****************************************************************************
 * 功能：打开串口
 * 参数：portname - 设备节点名称
 *			baudrate  - 通信波特率
 *			errmsg     - 错误提示信息（64字节缓存）
 *	返回：>=0 设备句柄，<0 失败
****************************************************************************/
int com_open(char* portname, int bps, char* errmsg) {
	int port_h = 0;
	struct termios config;
	int iBaudrateflag = BaudRateflag(bps);
	port_h = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
	if (port_h < 0) {
		strcpy(errmsg, strerror(errno));
		return -2;
	}
	tcgetattr(port_h, &config);
	cfmakeraw(&config);
	cfsetispeed(&config, iBaudrateflag);
	cfsetospeed(&config, iBaudrateflag);
	tcsetattr(port_h, TCSANOW, &config);
	return port_h;
}

/****************************************************************************
 * 功能：关闭串口
 * 参数：port_h - 设备句柄
 *	返回：
****************************************************************************/
void com_close(int port_h) {
	close(port_h);
}

/****************************************************************************
 * 功能：等待串口接收数据到达
 * 参数：port_h   - 设备句柄
 * 			timeout - 超时时间(毫秒)
 *	返回：0-数据到达，1-超时，-1-未知错误
****************************************************************************/
int com_wait(int port_h, int timeout) {
	struct timeval tm_out;
	int result;
	fd_set read_fds;

	FD_ZERO(&read_fds);
	FD_SET(port_h, &read_fds);

	tm_out.tv_sec = timeout / 1000;
	tm_out.tv_usec = 1000 * (timeout % 1000);

	result = select(port_h + 1, &read_fds, NULL, NULL, &tm_out);
	switch (result) {
	case -1: /* 未知错误*/
		return -1;
	case 0: /* 超时 */
		return 1;
	default: /* 数据到达*/
		break;
	}
	return 0;
}

/****************************************************************************
 * 功能：发送数据
 * 参数：port_h   - 设备句柄
 * 			data      - 发送数据缓存
 * 			len        - 预发送数据字节
 *	返回：实际发送数据字节
****************************************************************************/
int com_send(int port_h, char* data, int len)
{
	return write(port_h, data, len);
}

/****************************************************************************
 * 功能：接收数据
 * 参数：port_h   - 设备句柄
 * 			data      - 接收数据缓存
 * 			len        - 预接收数据字节
 *	返回：实际接收数据字节
****************************************************************************/
int com_recv(int port_h, char* data, int len) {
	int k = 0;
	int m = 0;
	while (k < len) {
		m = len - k;
		if (m > 200) {
			m = 200;
		}
		m = read(port_h, data + k, m);
		if (m < 0) {
			if (errno == EAGAIN) {
				if (com_wait(port_h, 1000) != 0) {   //500 -> 1000 by cgs 20160604
					/*
					 {
					 char s[1000];
					 s[0] = 0;
					 for (int i=0; i<len; i++)
					 {
					 sprintf(s+strlen(s), "%02x ", data[i]);
					 }
					 __android_log_print(ANDROID_LOG_DEBUG, "receive", "%s", s);
					 }
					 */
					return k;
				}
				continue;
			}
			return k;
		}
		k += m;
	}
	/*
	 {
	 char s[1000];
	 s[0] = 0;
	 for (int i=0; i<len; i++)
	 {
	 sprintf(s+strlen(s), "%02x ", data[i]);
	 }
	 __android_log_print(ANDROID_LOG_DEBUG, "receive", "%s", s);
	 }
	 */
	return len;
}



