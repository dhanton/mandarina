#pragma once

#include "defines.hpp"
#include "crcpacket.hpp"

struct CasterSnapshot {
    bool valid;
    
    float primaryPercentage;
    float secondaryPercentage;
    float altPercentage;
    float ultimatePercentage;

    CasterSnapshot();

    void loadFromData(CRCPacket& inPacket);

    CasterSnapshot getDiff(const CasterSnapshot& other, float minDiff = 0.f) const;
    bool isAllZero() const;

    CasterSnapshot operator -(const CasterSnapshot& other) const;
};
