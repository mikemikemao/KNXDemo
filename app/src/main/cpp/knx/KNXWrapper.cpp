//
// Created by maochaoqun on 2024/3/28.
//

#include <linux/in.h>
#include "KNXWrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/filter.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "alloc.h"
#include <pthread.h>
#include <utils/LogUtil.h>
#include "proto.h"


#define LOCAL_IP      "10.6.120.85"
#define KNX_MUILCAST_RECV_LEN 2048*100
#define KNX_MUILCAST_ADDR      "224.0.23.12"
#define KNX_NET_IP_ROUTER       "10.6.120.23"
#define KNXS_PORT      (3671)
#define KNXC_PORT      (53985)
#define SOCKET_RECV_BUFFER      (1024*1024)
#define SOCKET_SEND_BUFFER      (1024*1024)
//pthread_mutex_t knx_mutex = PTHREAD_MUTEX_INITIALIZER;


#define WIRESHARK_PACKET (1)//�ſ��˺�ʹ��wirshark����(�ɹ�)�����������������⣬������payload��ֵ������


struct KNX_CLIENT
{
    int  control_socket;
    int  tunnel_socket;
    int  remote_chid;
    char remote_ip[16];
    int  remote_port;
}KNX_CLIENT;

static struct KNX_CLIENT g_knxClient;



int knx_send (int fd, u_int8_t* packet,u_int32_t len, char*pAddr, int32_t port)
{
    int c;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(pAddr);
    sa.sin_port = htons(port);

    c = sendto(fd, packet, len, 0, (struct sockaddr *)&sa, sizeof(sa));

    return c;
}

/*************************************************************************
  �������ƣ�KNXCSearch
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCSearchRQ()
{
    int sendlen = 0;
    knx_search_request packet_in;
    memset(&packet_in,0x00,sizeof(knx_search_request));
    packet_in.control_host.protocol = KNX_PROTO_UDP;
    packet_in.control_host.address = inet_addr(LOCAL_IP);//���ص�ַ
    packet_in.control_host.port = htons(KNXC_PORT);
    // Generate
    uint8_t buffer[KNX_HEADER_SIZE + KNX_SEARCH_REQUEST_SIZE];
    knx_generate(buffer, KNX_SEARCH_REQUEST, &packet_in);

    LOGCATE("KNXCSearchRQ->server_ip %s server_port %d.\n",KNX_MUILCAST_ADDR,KNXS_PORT);
    sendlen = knx_send(g_knxClient.control_socket,(unsigned char*)buffer, sizeof(buffer),KNX_MUILCAST_ADDR, KNXS_PORT);
    if(sendlen != sizeof(buffer))
    {
        LOGCATE("knx_send error .\n");
    }


    LOGCATE("KNXCSearchRQ success %d.\n",g_knxClient.control_socket);

    return ;
}


void KNXCDescription()
{
    int sendlen = 0;
    knx_search_request packet_in;
    memset(&packet_in,0x00,sizeof(knx_search_request));
    packet_in.control_host.protocol = KNX_PROTO_UDP;
    packet_in.control_host.address = inet_addr(LOCAL_IP);//���ص�ַ
    packet_in.control_host.port = htons(KNXC_PORT);
    // Generate
    uint8_t buffer[KNX_HEADER_SIZE + KNX_SEARCH_REQUEST_SIZE];
    knx_generate(buffer, KNX_DESCRIPTION_REQUEST, &packet_in);

    LOGCATE("KNXCSearchRQ->server_ip %s server_port %d.\n",KNX_NET_IP_ROUTER,KNXS_PORT);
    sendlen = knx_send(g_knxClient.control_socket,(unsigned char*)buffer, sizeof(buffer),KNX_NET_IP_ROUTER, KNXS_PORT);
    if(sendlen != sizeof(buffer))
    {
        LOGCATE("knx_send error .\n");
    }


    LOGCATE("KNXCSearchRQ success %d.\n",g_knxClient.control_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCConnectRQ
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCConnectRQ()
{
    int sendlen = 0;

    knx_connection_request packet_in;
    memset(&packet_in,0x00,sizeof(knx_connection_request));

    packet_in.type = KNX_CONNECTION_REQUEST_TUNNEL;
    packet_in.layer = KNX_CONNECTION_LAYER_TUNNEL;
    packet_in.control_host.protocol = KNX_PROTO_UDP;
    packet_in.control_host.address = inet_addr(LOCAL_IP);//���ص�ַ
    packet_in.control_host.port = htons(KNXC_PORT );

    packet_in.tunnel_host.protocol = KNX_PROTO_UDP;
    packet_in.tunnel_host.address = inet_addr(LOCAL_IP);//���ص�ַ
    packet_in.tunnel_host.port = htons(KNXC_PORT+1);

    // Generate
    uint8_t buffer[KNX_HEADER_SIZE + KNX_CONNECTION_REQUEST_SIZE];
    knx_generate(buffer, KNX_CONNECTION_REQUEST, &packet_in);

    LOGCATE("KNXCConnectRQ->server_ip %s server_port %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port);
    sendlen = knx_send(g_knxClient.control_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(buffer))
    {
        LOGCATE("knx_send error .\n");
    }


    LOGCATE("KNXCConnectRQ success %d.\n",g_knxClient.control_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCConStateRQ
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCConStateRQ()
{
    int sendlen = 0;

    knx_connection_state_request packet_in = {0};
    packet_in.channel = g_knxClient.remote_chid;
    packet_in.status = 0;
    packet_in.host.protocol = KNX_PROTO_UDP;
    packet_in.host.address = inet_addr(LOCAL_IP);//���ص�ַ
    packet_in.host.port = htons(KNXC_PORT );

    // Generate
    uint8_t buffer[KNX_HEADER_SIZE + KNX_CONNECTION_STATE_REQUEST_SIZE];
    knx_generate(buffer, KNX_CONNECTION_STATE_REQUEST, &packet_in);

    LOGCATE("KNXCConStateRQ->server_ip %s server_port %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port);
    sendlen = knx_send(g_knxClient.control_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(buffer))
    {
        LOGCATE("knx_send error .\n");
    }


    LOGCATE("KNXCConStateRQ success %d.\n",g_knxClient.control_socket);

    return ;
}


/*************************************************************************
  �������ƣ�KNXCTunnelRQ_OpenLight
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_OpenLight()
{
    int sendlen = 0;
    uint8_t ldata_req = 0x01;
    knx_tunnel_request packet_in = {0};
    packet_in.channel = g_knxClient.remote_chid;
    packet_in.seq_number = 0;
    packet_in.data.service = KNX_CEMI_LDATA_REQ;
    packet_in.data.add_info_length = 0;
    packet_in.data.add_info = NULL;

    packet_in.data.payload.ldata.source = 0xfff1; //source:0xfff1=15.15.241
    packet_in.data.payload.ldata.destination = 0x0a04;//destination:0x0a04=1/2/4

    packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_LOW;
    //packet_in.data.payload.ldata.control1.repeat = true;
    //packet_in.data.payload.ldata.control1.system_broadcast = true;
    //packet_in.data.payload.ldata.control1.request_ack = true;
    //packet_in.data.payload.ldata.control1.error = false;

    packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
    packet_in.data.payload.ldata.control2.hops = 6;

    packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
    packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_GROUPVALUEWRITE;//���ֵַд��
    packet_in.data.payload.ldata.tpdu.info.data.payload = &ldata_req;//������:0x01
    packet_in.data.payload.ldata.tpdu.info.data.length = 1;

    // Generate
    uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

    LOGCATE("KNXCTunnelRQ_Open->server_ip %s server_port %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port);
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(buffer))
    {
        LOGCATE("knx_send error .\n");
    }

    LOGCATE("KNXCTunnelRQ_Open success %d.\n",g_knxClient.tunnel_socket);

    return ;
}


/*************************************************************************
  �������ƣ�KNXCTunnelRQ_AddrWrite
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_AddrWrite()
{
    int sendlen = 0;
    const uint8_t ldata_req[2] = {0x00, 0x10};
    knx_tunnel_request packet_in = {0};
    packet_in.channel = g_knxClient.remote_chid;
    packet_in.seq_number = 0;
    packet_in.data.service = KNX_CEMI_LDATA_REQ;
    packet_in.data.add_info_length = 0;
    packet_in.data.add_info = NULL;

    packet_in.data.payload.ldata.source = 0x1104;
    packet_in.data.payload.ldata.destination = 0x0000;

    packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
    //packet_in.data.payload.ldata.control1.repeat = true;
    //packet_in.data.payload.ldata.control1.system_broadcast = true;
    //packet_in.data.payload.ldata.control1.request_ack = true;
    //packet_in.data.payload.ldata.control1.error = false;

    packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
    packet_in.data.payload.ldata.control2.hops = 6;

    packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
    packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
    packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
    packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

    // Generate
    uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

    LOGCATE("KNXCTunnelRQ_AddrWrite->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(buffer))
    {
        LOGCATE("knx_send error .\n");
    }


    LOGCATE("KNXCTunnelRQ_AddrWrite success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrRead0
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_IndAddrRead_step1(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 519 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x15, 0x04, 0x15,
            0x0d, 0x00, 0x11, 0x00, 0xb0, 0xe0, 0x00, 0x00,
            0x00, 0x00, 0x01, 0x01, 0x00 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x00, 0x10};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x1104;
	packet_in.data.payload.ldata.destination = 0x0000;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_IndAddrRead_step1->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_IndAddrRead_step1->server_ip %s server_port %d peersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_IndAddrRead_step1 success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrWrite
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_IndAddrWrite(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 2752 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x17, 0x04, 0x15,
            0x15, 0x00, 0x11, 0x00, 0xb0, 0xe0, 0x00, 0x00,
            0x00, 0x00, 0x03, 0x00, 0xc0, 0x11, 0x01 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x11,0x01};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = seq_num;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x0000;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_IndAddrWrite->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_IndAddrWrite->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_IndAddrWrite success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrRead
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_IndAddrRead_step2(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 2763 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x15, 0x04, 0x15,
            0x16, 0x00, 0x11, 0x00, 0xb0, 0xe0, 0x00, 0x00,
            0x00, 0x00, 0x01, 0x01, 0x00 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x00, 0x10};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x1104;
	packet_in.data.payload.ldata.destination = 0x0000;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_IndAddrRead_step2->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_IndAddrRead_step2->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_IndAddrRead_step2 success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrConnect
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_IndAddrConnect(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 2777 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x14, 0x04, 0x15,
            0x17, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x00, 0x80 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_AddrWrite->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WkKNXCTunnelRQ_IndAddrConnect->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_IndAddrConnect success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrDevDescrRead
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_IndAddrDevDescrRead(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 2782 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x15, 0x04, 0x15,
            0x18, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x01, 0x43, 0x00 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x00, 0x10};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x1104;
	packet_in.data.payload.ldata.destination = 0x0000;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_AddrWrite->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_IndAddrDevDescrRead->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_IndAddrDevDescrRead success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrPropValueRead
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_IndAddrPropValueRead(int seq_num)
{
    int sendlen = 0;

#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 2808 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x19, 0x04, 0x15,
            0x1a, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x05, 0x47, 0xd5, 0x00, 0x0b, 0x10,
            0x01 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x00, 0x10};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x1104;
	packet_in.data.payload.ldata.destination = 0x0000;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_AddrWrite->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_IndAddrPropValueRead->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_IndAddrPropValueRead success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrRestart
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_IndAddrRestart(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 2827 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x15, 0x04, 0x15,
            0x1c, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x01, 0x4b, 0x80 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x00, 0x10};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = seq_num;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x1104;
	packet_in.data.payload.ldata.destination = 0x0000;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_IndAddrRestart->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_IndAddrRestart->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_IndAddrRestart success %d.\n",g_knxClient.tunnel_socket);

    return ;
}
/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrDisconnect
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_IndAddrDisconnect(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 2832 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x14, 0x04, 0x15,
            0x1d, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x00, 0x81 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x00, 0x10};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x1104;
	packet_in.data.payload.ldata.destination = 0x0000;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_IndAddrDisconnect->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_IndAddrDisconnect->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_IndAddrDisconnect success %d.\n",g_knxClient.tunnel_socket);

    return ;
}


/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrConnect
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_EditConnect(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 6911 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x14, 0x04, 0x2f,
            0x00, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x00, 0x80 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_EditConnect->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_EditConnect->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_EditConnect success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrConnect
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_EditAuthReq(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 7054 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x1a, 0x04, 0x2f,
            0x05, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x06, 0x4b, 0xd1, 0x00, 0xff, 0xff,
            0xff, 0xff };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_EditAuthReq->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_EditAuthReq->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_EditAuthReq success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_EditPropValueWrite
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_EditPropValueWrite(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 7419 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x23, 0x04, 0x2f,
            0x13, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x0f, 0x67, 0xd7, 0x05, 0x05, 0x10,
            0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_EditPropValueWrite->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_EditPropValueWrite->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_EditPropValueWrite success %d.\n",g_knxClient.tunnel_socket);

    return ;
}


/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrConnect
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_EditMemWrite(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 8059 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x4b, 0x04, 0x2f,
            0x37, 0x00, 0x11, 0x00, 0x30, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x37, 0x6e, 0xb4, 0x18, 0xe5, 0x04,
            0x14, 0x13, 0x0a, 0x80, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
            0x00, 0x00, 0x40, 0x34, 0x42, 0x20, 0x35, 0x41,
            0x20, 0x36, 0x43 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_EditMemWrite->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_EditMemWrite->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_EditMemWrite success %d.\n",g_knxClient.tunnel_socket);

    return ;
}


/*************************************************************************
  �������ƣ�KNXCTunnelRQ_IndAddrConnect
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_EditRestart(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 8840 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x15, 0x04, 0x2f,
            0x6d, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x01, 0x5b, 0x80 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_EditRestart->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_EditRestart->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_EditRestart success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_Rs485DevConfigRq
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_Rs485DevConfigRq(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 503 */
            0x06, 0x10, 0x03, 0x10, 0x00, 0x11, 0x04, 0x60,
            0x00, 0x00, 0xfc, 0x00, 0x00, 0x01, 0x53, 0x10,
            0x01 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_Rs485DevConfigRq->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_Rs485DevConfigRq->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.control_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_Rs485DevConfigRq success %d.\n",g_knxClient.control_socket);

    return ;
}


/*************************************************************************
  �������ƣ�KNXCTunnelRQ_Rs485Connect
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_Rs485Connect(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 514 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x14, 0x04, 0x5f,
            0x00, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x00, 0x80 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_Rs485Connect->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_Rs485Connect->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_Rs485Connect success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_Rs485DevDescrRead
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_Rs485DevDescrRead(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 519 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x15, 0x04, 0x5f,
            0x01, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x01, 0x43, 0x00 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_Rs485DevDescrRead->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("KNXCTunnelRQ_Rs485DevDescrRead->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_Rs485DevDescrRead success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_Rs485DevDescrRead
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_Rs485PropValueRead(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 537 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x19, 0x04, 0x5f,
            0x03, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x05, 0x47, 0xd5, 0x00, 0x38, 0x10,
            0x01 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_Rs485DevDescrRead->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("KNXCTunnelRQ_Rs485DevDescrRead->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_Rs485DevDescrRead success %d.\n",g_knxClient.tunnel_socket);

    return ;
}



/*************************************************************************
  �������ƣ�KNXCTunnelRQ_Rs485PropValueWrite_open
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_Rs485PropValueWrite_open(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 18289 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x1a, 0x04, 0x53,
            0x0f, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x06, 0x4b, 0xd7, 0x00, 0x36, 0x10,
            0x01, 0x01 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_Rs485PropValueWrite_open->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_Rs485PropValueWrite_open->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_Rs485PropValueWrite_open success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_Rs485PropValueWrite_close
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_Rs485PropValueWrite_close(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 558 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x1a, 0x04, 0x5f,
            0x05, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x06, 0x4b, 0xd7, 0x00, 0x36, 0x10,
            0x01, 0x00 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_Rs485PropValueWrite_close->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_Rs485PropValueWrite_close->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_Rs485PropValueWrite_close success %d.\n",g_knxClient.tunnel_socket);

    return ;
}

/*************************************************************************
  �������ƣ�KNXCTunnelRQ_Rs485DevDescrRead
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_Rs485PropValueRead_step2(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 576 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x19, 0x04, 0x5f,
            0x07, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x05, 0x4f, 0xd5, 0x00, 0x36, 0x10,
            0x01 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x80, 0x00};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x0000;
	packet_in.data.payload.ldata.destination = 0x1101;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_Rs485DevDescrRead->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("KNXCTunnelRQ_Rs485DevDescrRead->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_Rs485DevDescrRead success %d.\n",g_knxClient.tunnel_socket);

    return ;
}


/*************************************************************************
  �������ƣ�KNXCTunnelRQ_Rs485Disconnect
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
void KNXCTunnelRQ_Rs485Disconnect(int seq_num)
{
    int sendlen = 0;
#ifdef WIRESHARK_PACKET
    char peer[] = { /* Packet 596 */
            0x06, 0x10, 0x04, 0x20, 0x00, 0x14, 0x04, 0x5f,
            0x09, 0x00, 0x11, 0x00, 0xb0, 0x60, 0x00, 0x00,
            0x11, 0x01, 0x00, 0x81 };
    peer[7] = g_knxClient.remote_chid;
    peer[8] = seq_num;
#endif

#ifndef WIRESHARK_PACKET
    const uint8_t ldata_req[2] = {0x00, 0x10};
	knx_tunnel_request packet_in = {0};
	packet_in.channel = g_knxClient.remote_chid;
	packet_in.seq_number = 0;
	packet_in.data.service = KNX_CEMI_LDATA_REQ;
	packet_in.data.add_info_length = 0;
	packet_in.data.add_info = NULL;

	packet_in.data.payload.ldata.source = 0x1104;
	packet_in.data.payload.ldata.destination = 0x0000;

	packet_in.data.payload.ldata.control1.priority = KNX_LDATA_PRIO_SYSTEM;
	//packet_in.data.payload.ldata.control1.repeat = true;
	//packet_in.data.payload.ldata.control1.system_broadcast = true;
	//packet_in.data.payload.ldata.control1.request_ack = true;
	//packet_in.data.payload.ldata.control1.error = false;

	packet_in.data.payload.ldata.control2.address_type = KNX_LDATA_ADDR_GROUP;
	packet_in.data.payload.ldata.control2.hops = 6;

	packet_in.data.payload.ldata.tpdu.tpci = KNX_TPCI_UNNUMBERED_DATA;
	packet_in.data.payload.ldata.tpdu.info.data.apci = KNX_APCI_INDIVIDUALADDRWRITE;
	packet_in.data.payload.ldata.tpdu.info.data.payload = ldata_req;
	packet_in.data.payload.ldata.tpdu.info.data.length = sizeof(ldata_req);

	// Generate
	uint8_t buffer[KNX_HEADER_SIZE + knx_tunnel_request_size(&packet_in)];//knx_tunnel_request_size�˽ӿ�������
    knx_generate(buffer, KNX_TUNNEL_REQUEST, &packet_in);

	LOGCATE("KNXCTunnelRQ_Rs485Disconnect->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(buffer));
	sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(buffer),g_knxClient.remote_ip, g_knxClient.remote_port);
	if(sendlen != sizeof(buffer))
	{
		LOGCATE("knx_send error .\n");
	}

#else
    uint8_t buffer[256] = {0};
    memcpy(buffer,peer,sizeof(peer));
    LOGCATE("WKNXCTunnelRQ_Rs485Disconnect->server_ip %s server_port %d buffersize %d.\n",g_knxClient.remote_ip,g_knxClient.remote_port,sizeof(peer));
    sendlen = knx_send(g_knxClient.tunnel_socket,(unsigned char*)buffer, sizeof(peer),g_knxClient.remote_ip, g_knxClient.remote_port);
    if(sendlen != sizeof(peer))
    {
        LOGCATE("knx_send error .\n");
    }
#endif



    LOGCATE("KNXCTunnelRQ_Rs485Disconnect success %d.\n",g_knxClient.tunnel_socket);

    return ;
}


int knx_packet_parse(int fd)
{
    int n;
    socklen_t len;
    struct sockaddr_in sa;
    unsigned char buffer[KNX_MUILCAST_RECV_LEN];
    knx_packet packet_out;
    memset(&packet_out,0x00,sizeof(knx_packet));
    len = sizeof(struct sockaddr_in);
    bzero(&sa, len);
    n = recvfrom(fd, buffer, KNX_MUILCAST_RECV_LEN-1, 0,(struct sockaddr *)&sa,&len);
    if(0 < n)
    {
        knx_parse(buffer, sizeof(buffer), &packet_out);

        LOGCATE("knx_packet_parse packet_out.service: 0x%x.\n",packet_out.service);
        if(KNX_SEARCH_RESPONSE == packet_out.service)
        {
            knx_host_info host;
            memset(&host,0x00,sizeof(knx_host_info));
            char *host_ip = NULL;
            struct in_addr addr;
            knx_host_info_parse(buffer + KNX_HEADER_SIZE, n - KNX_HEADER_SIZE, &host);
            memcpy(&addr,&host.address,4);
            host_ip = inet_ntoa(addr);
            LOGCATE("knx_packet_parse host address: %s.\n",host_ip);
            LOGCATE("knx_packet_parse host port: %d.\n",ntohs(host.port));
            g_knxClient.remote_port = ntohs(host.port);
            snprintf(g_knxClient.remote_ip,sizeof(g_knxClient.remote_ip),"%s",host_ip);
            LOGCATE("knx_packet_parse host address: %s port: %d .\n",g_knxClient.remote_ip,g_knxClient.remote_port);
        }
        else if(KNX_DISCONNECT_RESPONSE == packet_out.service)
        {
            DEBUG_LOGCATI("1111111111111111");
        }
        else if(KNX_CONNECTION_RESPONSE == packet_out.service)
        {
            char *host_ip = NULL;
            struct in_addr addr;
            memcpy(&addr,&packet_out.payload.conn_res.host.address,4);
            host_ip = inet_ntoa(addr);
            LOGCATE("knx_packet_parse host address: %s.\n",host_ip);
            LOGCATE("knx_packet_parse host port: %d.\n",ntohs(packet_out.payload.conn_res.host.port));
            g_knxClient.remote_chid = packet_out.payload.conn_res.channel;
            LOGCATE("knx_packet_parse conn_res.channel: %d.\n",g_knxClient.remote_chid);
            LOGCATE("knx_packet_parse conn_res.status: %d.\n",packet_out.payload.conn_res.status);
        }
    }
    return 0;
}



static void* thr_knx_control_capture(void* pv)
{
    int i = 0;
    fd_set fdset_r;
    struct  timeval tv;
    int iMaxSockFd;
    int iSelectRet = -1;
    int caplen = 0;
    LOGCATE("thr_knx_control_capture entry....\n");
    iMaxSockFd = g_knxClient.control_socket;
    while(1)
    {

        FD_ZERO(&fdset_r);
        FD_SET(g_knxClient.control_socket,&fdset_r);
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        iSelectRet = select((int)iMaxSockFd+1,&fdset_r,NULL,NULL,&tv);
        if(iSelectRet > 0)
        {

            if(FD_ISSET(g_knxClient.control_socket, &fdset_r))
            {
                struct sockaddr_in sa;
                memset(&sa, 0, sizeof(sa));
                caplen = knx_packet_parse(g_knxClient.control_socket);
                if (caplen <= 0)
                {
                    //LOGCATE("thr_knx_control_capture, recvfrom err!\n");
                    continue;
                }

            }

        }
        else if(iSelectRet == 0)
        {
            //LOGCATE("\n-------------we select over time ,return 0;-------------\n");
            continue;
        }
        else
        {
            LOGCATE("select failed!!\n");
        }
    }


    close(g_knxClient.control_socket);
    return NULL;
}

static void* thr_knx_tunnel_capture(void* pv)
{
    int i = 0;
    fd_set fdset_r;
    struct  timeval tv;
    int iMaxSockFd;
    int iSelectRet = -1;
    int caplen = 0;
    LOGCATE("thr_knx_tunnel_capture entry....\n");
    iMaxSockFd = g_knxClient.tunnel_socket;
    while(1)
    {

        FD_ZERO(&fdset_r);
        FD_SET(g_knxClient.tunnel_socket,&fdset_r);
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        iSelectRet = select((int)iMaxSockFd+1,&fdset_r,NULL,NULL,&tv);
        if(iSelectRet > 0)
        {

            if(FD_ISSET(g_knxClient.tunnel_socket, &fdset_r))
            {
                struct sockaddr_in sa;
                memset(&sa, 0, sizeof(sa));
                caplen = knx_packet_parse(g_knxClient.tunnel_socket);
                if (caplen <= 0)
                {
                    //LOGCATE("thr_knx_tunnel_capture, recvfrom err!\n");
                    continue;
                }

            }

        }
        else if(iSelectRet == 0)
        {
            //LOGCATE("\n-------------we select over time ,return 0;-------------\n");
            continue;
        }
        else
        {
            LOGCATE("select failed!!\n");
        }
    }


    close(g_knxClient.tunnel_socket);
    return NULL;
}

static void* thr_knx_keep_live(void* pv)
{

    LOGCATE("thr_knx_keep_live entry....\n");
    while(1)
    {
        sleep(30);
        KNXCConStateRQ();

    }
    return NULL;
}


/*************************************************************************
  �������ƣ�KNXCreateSocket
  ��������������socket
  IN��	iSndBuffSize-���ͻ�������С
  IN��	iRcvBuffSize-���ջ�������С
  OUT����
  return��sock
  ע�������
 *************************************************************************/
static int KNXCreateSocket(int iSndBuffSize, int iRcvBuffSize)
{
    int optval = 1;
    int sock =-1;

    sock = socket (AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        LOGCATE("create socket failed!!!\n");

        return -1;
    }

#if 1
    if(-1 == setsockopt (sock, SOL_SOCKET, SO_RCVBUF,(const char*)&iRcvBuffSize,sizeof(int)))
    {
        LOGCATE("setsockopt failed!!!\n");
        close(sock);
        return -1;
    }

    if(-1 == setsockopt (sock, SOL_SOCKET, SO_SNDBUF,(const char*)&iSndBuffSize,sizeof(int)))
    {
        LOGCATE("setsockopt failed!!!\n");
        close(sock);
        return -1;
    }
#endif

    if(-1 == setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)))
    {
        LOGCATE("setsockopt failed!!!\n");
        close(sock);
        return -1;
    }

    return sock;
}


/*************************************************************************
  �������ƣ�knx_init_sock
  ����������
  return��ERROR or Ok
  ע�������
 *************************************************************************/
int knx_init_sock()
{
    struct sockaddr_in srv;
    memset(&g_knxClient,0, sizeof(g_knxClient));
    //����
    bzero(&srv, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = htonl(INADDR_ANY);
    srv.sin_port = htons(KNXC_PORT);
    g_knxClient.control_socket = KNXCreateSocket(SOCKET_SEND_BUFFER, SOCKET_RECV_BUFFER);
    if(bind(g_knxClient.control_socket, (struct sockaddr *)&srv, sizeof(srv)) < 0)
    {
        LOGCATE("bind to dev fail: %s",strerror(errno));
        close(g_knxClient.control_socket);
        return -1;
    }
    LOGCATE("KNXCreateSocket control_socket success %d.\n",g_knxClient.control_socket);
    //����
    bzero(&srv, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = htonl(INADDR_ANY);
    srv.sin_port = htons(KNXC_PORT+1);
    g_knxClient.tunnel_socket = KNXCreateSocket(SOCKET_SEND_BUFFER, SOCKET_RECV_BUFFER);
    if(bind(g_knxClient.tunnel_socket, (struct sockaddr *)&srv, sizeof(srv)) < 0)
    {
        LOGCATE("bind to dev fail: %s",strerror(errno));
        close(g_knxClient.tunnel_socket);
        return -1;
    }
    LOGCATE("KNXCreateSocket tunnel_socket success %d.\n",g_knxClient.tunnel_socket);
#if 0
    //����
	bzero(&srv, sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_addr.s_addr = htonl(INADDR_ANY);
	srv.sin_port = htons(KNXC_PORT+2);
	g_knxClient.mul_fd = KNXCreateSocket(SOCKET_SEND_BUFFER, SOCKET_RECV_BUFFER);
	if(bind(g_knxClient.mul_fd, (struct sockaddr *)&srv, sizeof(srv)) < 0)
	{
		LOGCATE("bind to dev fail: %s",strerror(errno));
		close(g_knxClient.mul_fd);
		return -1;
	}
	LOGCATE("KNXCreateSocket mul socket success %d.\n",g_knxClient.mul_fd);
#endif
    return 0;
}


int knxTest()
{

    int retVal = 0;
    retVal = knx_init_sock();
    LOGCATE("knx_init_sock done.\n");

    pthread_t tid1;
    pthread_create( &tid1, NULL, thr_knx_control_capture, NULL );

    pthread_t tid2;
    pthread_create( &tid2, NULL, thr_knx_tunnel_capture, NULL );
    sleep(3);
    KNXCSearchRQ();
    LOGCATE("KNXCSearch done.\n");
    KNXCDescription();
    LOGCATE("KNXCDescription done.\n");
    sleep(5);
    KNXCConnectRQ();
    printf("KNXCConnectRQ done.\n");
    sleep(3);
    /*灯控*/
    KNXCTunnelRQ_OpenLight();
    return 0;
}

