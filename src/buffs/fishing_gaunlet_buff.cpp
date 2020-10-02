#include "buffs/fishing_gaunlet_buff.hpp"

#include "unit.hpp"
#include "projectiles.hpp"
#include "helper.hpp"

FishingGaunletBuff* FishingGaunletBuff::clone() const
{
    return new FishingGaunletBuff(*this);
}

void FishingGaunletBuff::onProjectileHit(Projectile& projectile, Entity* target)
{
    if (projectile.type == PROJECTILE_FISHING_GAUNLET) {
        if (target == nullptr) return;

        const Vector2 movement = target->getPosition() - m_unit->getPosition();
        const float length = Helper_vec2length(movement) - m_unit->getCollisionRadius() - target->getCollisionRadius();

        //teleport to the hit unit (leaving space for collision)
        m_unit->setPosition(m_unit->getPosition() + Helper_vec2unitary(movement) * length);
    }
}
