//
// Created by maochaoqun on 2024/4/8.
//

#include <knx/address/GroupAddress.h>
#include "KnxFunc.h"

int KnxlightControl(byte lightState){
    GroupAddress groupAddr;
    groupAddr.of(0,0,8);
    return ERR_OK;
}