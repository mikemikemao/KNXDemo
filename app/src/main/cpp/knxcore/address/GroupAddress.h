//
// Created by maochaoqun on 2024/4/8.
//

#ifndef KNXDEMO_GROUPADDRESS_H
#define KNXDEMO_GROUPADDRESS_H


#include "KnxAddress.h"

class GroupAddress : public KnxAddress{
public:
    GroupAddress() {}
    ~GroupAddress(){}
    int of(unsigned char main, unsigned char middle, unsigned char sub);
private:
    signed char gropAddr[STRUCTURE_LENGTH];
};


#endif //KNXDEMO_GROUPADDRESS_H
