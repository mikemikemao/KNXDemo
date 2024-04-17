//
// Created by maochaoqun on 2024/4/8.
//

#ifndef KNXDEMO_ADDRESSTYPE_H
#define KNXDEMO_ADDRESSTYPE_H

#define ADDRESS_TYPE_MAP(XX) \
                         XX(INDIVIDUAL,0x00, "Individual Address")          \
                         XX(GROUP,0x01,"Group Address")

enum class AddressType{
    ADDRESS_TYPE_Invalid = -1,
#define XX(name, code, disc) name = code,
    ADDRESS_TYPE_MAP(XX)
#undef XX
    ADDRESS_TYPE_MAX
};

#endif //KNXDEMO_ADDRESSTYPE_H
