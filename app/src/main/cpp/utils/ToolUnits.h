//
// Created by maochaoqun on 2024/3/25.
//

#ifndef TOOLUNITS_H
#define TOOLUNITS_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

class noncopyable {
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    //禁止拷贝
    noncopyable(const noncopyable &that) = delete;
    noncopyable(noncopyable &&that) = delete;
    noncopyable &operator=(const noncopyable &that) = delete;
    noncopyable &operator=(noncopyable &&that) = delete;
};

string hexdump(const void *buf, int len);
string hexmem(const void* buf, int len);


#endif //KNXDEMO_TOOLUNITS_H
