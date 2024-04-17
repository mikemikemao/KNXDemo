//
// Created by maochaoqun on 2024/4/8.
//

#ifndef KNXDEMO_KNXADDRESS_H
#define KNXDEMO_KNXADDRESS_H

#include <string>
#include "AddressType.h"

using namespace std;
class KnxAddress {
public:
    static constexpr auto STRUCTURE_LENGTH           = 2;
    AddressType getAddressType();
    string getAddress();
};

#endif //KNXDEMO_KNXADDRESS_H
