//
// Created by maochaoqun on 2024/4/8.
//

#ifndef KNXDEMO_GROUPADDRESS_H
#define KNXDEMO_GROUPADDRESS_H


#include "KnxAddress.h"
#include "ErrCode.h"
/**
* <pre>
* +-7-+-6-+-5-+-4-+-3-+-2-+-1-+-0-+-7-+-6-+-5-+-4-+-3-+-2-+-1-+-0-+
* | Byte 1                      | Byte 2                          |
* | (1 octet)                   | (1 octet)                       |
* +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
* </pre>
 * */
class GroupAddress : public std::enable_shared_from_this<GroupAddress>, public KnxAddress{
public:
    using Ptr = std::shared_ptr<GroupAddress>;
    GroupAddress(byte* addressRawData,int addressRawDataLen);
    Ptr of(byte* bytes,int bytesLen) {
        return std::make_shared<GroupAddress>(bytes,bytesLen);
    }
    GroupAddress() {}
    ~GroupAddress(){}
    Ptr of(unsigned char main, unsigned char middle, unsigned char sub);
    AddressType getAddressType();
    string getAddress();
private:
    unsigned char address[STRUCTURE_LENGTH];
};


#endif //KNXDEMO_GROUPADDRESS_H
