#include "caster_snapshot.hpp"

#include <cmath>
#include "helper.hpp"

CasterSnapshot::CasterSnapshot()
{
    primaryPercentage = 0;
    secondaryPercentage = 0;
    altPercentage = 0;
    ultimatePercentage = 0;

    valid = true;
}

void CasterSnapshot::loadFromData(CRCPacket& inPacket)
{
    u16 _primary;
    inPacket >> _primary;
    primaryPercentage = Helper_percentageFrom16bit(_primary);

    u16 _secondary;
    inPacket >> _secondary;
    secondaryPercentage = Helper_percentageFrom16bit(_secondary);

    u16 _altAbility;
    inPacket >> _altAbility;
    altPercentage = Helper_percentageFrom16bit(_altAbility);

    u16 _ultimate;
    inPacket >> _ultimate;
    ultimatePercentage = Helper_percentageFrom16bit(_ultimate);
}

CasterSnapshot CasterSnapshot::getDiff(const CasterSnapshot& other, float delta) const
{
    CasterSnapshot diff = (*this) - other;

    if (diff.primaryPercentage < delta) {
        diff.primaryPercentage = 0.f;
    }

    if (diff.secondaryPercentage < delta) {
        diff.secondaryPercentage = 0.f;
    }

    if (diff.altPercentage < delta) {
        diff.altPercentage = 0.f;
    }

    if (diff.ultimatePercentage < delta) {
        diff.ultimatePercentage = 0.f;
    }

    return diff;
}

bool CasterSnapshot::isAllZero() const
{
    return primaryPercentage == 0.f && secondaryPercentage == 0.f && altPercentage == 0.f && ultimatePercentage == 0.f;
}

CasterSnapshot CasterSnapshot::operator -(const CasterSnapshot& other) const
{
    CasterSnapshot caster;

    caster.primaryPercentage = primaryPercentage - other.primaryPercentage;
    caster.secondaryPercentage = secondaryPercentage - other.secondaryPercentage;
    caster.altPercentage = altPercentage - other.altPercentage;
    caster.ultimatePercentage = ultimatePercentage - other.ultimatePercentage;

    return caster;
}
