//
// Created by maochaoqun on 2024/4/8.
//

#include <utils/ErrCode.h>
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

GroupAddress::Ptr GroupAddress::of(unsigned char main, unsigned char middle, unsigned char sub) {
    if ((main>=0 && main<=31) != true)
    {
        return nullptr;
    }
    if ((middle >=0 && middle <=7) != true)
    {
        return nullptr;
    }
    if ((sub >=0 && sub <=255) != true)
    {
        return nullptr;
    }
    if (main ==0 && middle ==0 && sub ==0)
    {
        return nullptr;
    }
    byte mainAsByte = (byte) ((main & 0x1F) << 3);
    // byte 0: .... .xxx
    byte middleAsByte = (byte) (middle & 0x07);
    // byte 1: xxxx xxxx
    byte subAsByte = (byte) sub;
    byte groupAddr[2] = {(byte) (mainAsByte | middleAsByte), subAsByte};
    return std::make_shared<GroupAddress>(groupAddr,2);
}

AddressType GroupAddress::getAddressType()
{
    return AddressType::GROUP;
}

string GroupAddress::getAddress()
{
    return hexmem(address, STRUCTURE_LENGTH);
}