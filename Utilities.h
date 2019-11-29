#pragma once
#include"stdint.h"

template<class type>
int32_t indexOfSetBit(type value){
    static_assert(type(-1) > 0, "type need match unsigned type");
    int32_t index = 0;
    type v = value;

    for (type c = 16, mask = 0; ; c >>= 1) {
        mask = (1 << c) - 1;
        if (!(value & mask)){
            value >>= c;
            index += c;
        }
        if (c == 0){
            break;
        }
    }
    return v & (1ull << index) ? index : -1;
}

template<class type>
int32_t indexOfResetBit(type value){
    return indexOfSetBit(~value);
}