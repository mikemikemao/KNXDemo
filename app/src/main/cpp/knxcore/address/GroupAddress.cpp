//
// Created by maochaoqun on 2024/4/8.
//

#include <utils/ErrCode.h>
#include <utils/ToolUnits.h>
#include <zltoolkit/Util/logger.h>
#include "GroupAddress.h"
using namespace toolkit;

GroupAddress::GroupAddress(byte* addressRawData,int addressRawDataLen)
{
    if (addressRawDataLen != STRUCTURE_LENGTH){
        LogE("GroupAddress len err");
    } else{
        memcpy(address,addressRawData,STRUCTURE_LENGTH);
    }
}

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

    return  ERR_OK;
}

AddressType GroupAddress::getAddressType()
{
    return AddressType::GROUP;
}

string GroupAddress::getAddress()
{
    return hexmem(address, STRUCTURE_LENGTH);
}