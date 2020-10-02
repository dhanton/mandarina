#include "render_node.hpp"

RenderNode::RenderNode(u32 uniqueId)
{
    this->uniqueId = uniqueId;

    usingSprite = false;
    drawable = nullptr;

    height = 0.f;
    manualFilter = 0;

    takingDamage = false;
}

bool RenderNode::operator<(const RenderNode& other) 
{
    if (height == other.height) {
        if (uniqueId == other.uniqueId) {
            return manualFilter < other.manualFilter;
        }

        return uniqueId < other.uniqueId;
    }

    return height < other.height;
}
