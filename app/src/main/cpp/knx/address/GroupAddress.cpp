//
// Created by maochaoqun on 2024/4/8.
//

#include <utils/ErrCode.h>
#include "GroupAddress.h"

int GroupAddress::of(unsigned char main, unsigned char middle, unsigned char sub) {
    if ((main>=0 && main<=31) != true)
    {
        return ERR_FAIL;
    }
    if ((middle >=0 && middle <=7) != true)
    {
        return ERR_FAIL;
    }
    if ((sub >=0 && sub <=255) != true)
    {
        return ERR_FAIL;
    }
    if (main ==0 && middle ==0 && sub ==0)
    {
        return ERR_FAIL;
    }
    gropAddr[0] = ((main & 0x1F) << 3) + (middle & 0x07);
    // byte 1: xxxx xxxx
    gropAddr[1] = sub;
    return  ERR_OK;
}
