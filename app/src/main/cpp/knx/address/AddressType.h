//
// Created by maochaoqun on 2024/4/8.
//

#ifndef KNXDEMO_ADDRESSTYPE_H
#define KNXDEMO_ADDRESSTYPE_H

#include <string>
using namespace std;
class AddressType {
public:
    AddressType(int code,string friendlyName) {
        this->code = code;
        this->friendlyName = friendlyName;
    }
    int getCode() {
        return code;
    }
    string getFriendlyName() {
        return friendlyName;
    }
private:
    int code;
    string friendlyName;
};


#endif //KNXDEMO_ADDRESSTYPE_H
