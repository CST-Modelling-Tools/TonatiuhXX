#pragma once

#include "kernel/TonatiuhKernel.h"

class TONATIUH_KERNEL Random
{
public:
    virtual ~Random() = default;
    virtual double RandomDouble() = 0;
};
