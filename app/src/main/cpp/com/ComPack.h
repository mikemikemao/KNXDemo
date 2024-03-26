#ifndef __Com_PACK__
#define __Com_PACK__

#define MAX_PACK_SZIE           1500
#define MAX_SERVICE_FRAME       255
#define PACK_START_FLAG         0xAA
#define PACK_FRAME_LEFT         6
#define SERVICE_FRAME_HEAD      7

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


#endif

