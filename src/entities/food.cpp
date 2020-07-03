#include "entities/food.hpp"

#include "bit_stream.hpp"
#include "texture_ids.hpp"
#include "collision_manager.hpp"
#include "quadtree.hpp"
#include "server_entity_manager.hpp"
#include "client_entity_manager.hpp"
#include "hero.hpp"

FoodRarityType FoodBase::m_rarityType[MAX_FOOD_TYPES];
u8 FoodBase::m_dropRate[MAX_FOOD_RARITY_TYPES];
u32 FoodBase::m_powerGiven[MAX_FOOD_RARITY_TYPES];
sf::Color FoodBase::m_color[MAX_FOOD_RARITY_TYPES];
u8 FoodBase::m_rarityOffset[MAX_FOOD_RARITY_TYPES + 1];
float FoodBase::m_collisionRadius = 0.f;
bool FoodBase::m_foodDataLoaded = false;

u8 FoodBase::getFoodType() const
{
    return m_foodType;
}

void FoodBase::setFoodType(u8 foodType)
{
    m_foodType = foodType;
}

u8 FoodBase::getRarity() const
{
    return m_rarityType[m_foodType];
}

float FoodBase::getDropRate(u8 foodType)
{
    return m_dropRate[m_rarityType[foodType]];
}

u32 FoodBase::getPowerGiven(u8 foodType)
{
    return m_powerGiven[m_rarityType[foodType]];
}

sf::Color FoodBase::getRarityColor(u8 foodType)
{
    return m_color[m_rarityType[foodType]];
}

u8 FoodBase::getRandomFood()
{
    u8 uniform = rand() % 100;
    u8 rarity = 0xff;
    u8 combinedPercentage = 0;

    //Get a random type using the appropiate drop chance
    for (int i = 0; i < MAX_FOOD_RARITY_TYPES; ++i) {
        combinedPercentage += m_dropRate[i];

        if (uniform < combinedPercentage) {
            rarity = i;
            break;
        }
    }

    //Get a random food within the same type
    return (rand() % (m_rarityOffset[rarity + 1] - m_rarityOffset[rarity])) + m_rarityOffset[rarity];
}

void FoodBase::scatterFood(const Vector2& pos, const std::vector<u8>& foodVec, const ManagersContext& context)
{
    if (foodVec.empty()) return;
    
    const float scatterRadius = 60.f;

    //don't scatter food closer than this
    const float offset = 25.f;

    for (int i = 0; i < foodVec.size(); ++i) {
        const float randAngle = (rand() % 360) * PI/180.0;
        const float dist = static_cast<float>(rand() % static_cast<int>(scatterRadius - offset)) + offset;
        const Vector2 randVec = Vector2(std::sin(randAngle) * dist, std::cos(randAngle) * dist);

        Vector2 finalPos = randVec + pos;

        //make sure the food doesn't get stuck in a wall or block
        finalPos = _UnitBase::moveCollidingTilemap_impl(pos, finalPos, m_collisionRadius, context.tileMap);

        Entity* entity = context.entityManager->createEntity(ENTITY_FOOD, finalPos, 0);

        if (entity) {
            Food* food = static_cast<Food*>(entity);
            food->setFoodType(foodVec[i]);
        }
    }
}

void FoodBase::scatterRandomFood(const Vector2& pos, int foodAmount, const ManagersContext& context)
{
    std::vector<u8> foodVec;

    for (int i = 0; i < foodAmount; ++i) {
        foodVec.push_back(getRandomFood());
    }

    scatterFood(pos, foodVec, context);
}

void FoodBase::loadFoodData(const rapidjson::Document& doc)
{
    m_foodType = MAX_FOOD_TYPES;

    if (m_foodDataLoaded) return;
    
    //number of food in each rarity
    m_rarityOffset[FOOD_RARITY_COMMON] = 0;
    m_rarityOffset[FOOD_RARITY_UNCOMMON] = 18;
    m_rarityOffset[FOOD_RARITY_RARE] = 27;
    m_rarityOffset[FOOD_RARITY_MYTHIC] = 30;
    m_rarityOffset[MAX_FOOD_RARITY_TYPES] = MAX_FOOD_TYPES;    

    for (int i = 0; i < MAX_FOOD_TYPES; ++i) {
        if (i < m_rarityOffset[FOOD_RARITY_UNCOMMON]) {
            m_rarityType[i] = FOOD_RARITY_COMMON;

        } else if (i < m_rarityOffset[FOOD_RARITY_RARE]) {
            m_rarityType[i] = FOOD_RARITY_UNCOMMON;

        } else if (i < m_rarityOffset[FOOD_RARITY_MYTHIC]) {
            m_rarityType[i] = FOOD_RARITY_RARE;

        } else {
            m_rarityType[i] = FOOD_RARITY_MYTHIC;
        }
    }

    //in percentage
    m_dropRate[FOOD_RARITY_COMMON] = 60;
    m_dropRate[FOOD_RARITY_UNCOMMON] = 30;
    m_dropRate[FOOD_RARITY_RARE] = 9;
    m_dropRate[FOOD_RARITY_MYTHIC] = 1;

    m_powerGiven[FOOD_RARITY_COMMON] = 200;
    m_powerGiven[FOOD_RARITY_UNCOMMON] = 400;
    m_powerGiven[FOOD_RARITY_RARE] = 900;
    m_powerGiven[FOOD_RARITY_MYTHIC] = 4000;

    //glow color used for non common food
    m_color[FOOD_RARITY_COMMON] = sf::Color::Black;
    m_color[FOOD_RARITY_UNCOMMON] = sf::Color(192, 192, 192);
    m_color[FOOD_RARITY_RARE] = sf::Color(255, 215, 0);
    m_color[FOOD_RARITY_MYTHIC] = sf::Color(97, 26, 98);

    m_collisionRadius = doc["collision_radius"].GetFloat();

    m_foodDataLoaded = true;
}

Food* Food::clone() const
{
    return new Food(*this);
}

void Food::loadFromJson(const rapidjson::Document& doc)
{
    Entity::loadFromJson(doc);

    loadFoodData(doc);

    //@REMOVE
    m_foodType = FOOD_TOMATO;
}

void Food::update(sf::Time eTime, const ManagersContext& context)
{
    if (m_dead) true;

    Circlef circle(getPosition(), getCollisionRadius());
    auto query = context.collisionManager->getQuadtree()->QueryIntersectsRegion(BoundingBody<float>(circle));

    while (!query.EndOfQuery()) {
        Entity* entity = context.entityManager->entities.atUniqueId(query.GetCurrent()->uniqueId);

        if (m_dead || !entity) {
            query.Next();
            continue;
        }

        Hero* hero = dynamic_cast<Hero*>(entity);

        if (hero) {
            hero->consumeFood(getFoodType());
            m_dead = true;
        }

        query.Next();
    }
}

void Food::preUpdate(sf::Time eTime, const ManagersContext& context)
{

}

void Food::postUpdate(sf::Time eTime, const ManagersContext& context)
{

}

void Food::packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const
{
    outPacket << m_foodType;

    BitStream bits;

    bool posXChanged = !prevEntity || m_pos.x != prevEntity->getPosition().x;
    bits.pushBit(posXChanged);

    bool posYChanged = !prevEntity || m_pos.y != prevEntity->getPosition().y;
    bits.pushBit(posYChanged);

    outPacket << bits.popByte();

    if (posXChanged) {
        outPacket << m_pos.x;
    }

    if (posYChanged) {
        outPacket << m_pos.y;
    }
}

C_Food* C_Food::clone() const
{
    return new C_Food(*this);
}

void C_Food::loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context)
{
    C_Entity::loadFromJson(doc, textureId, context);

    loadFoodData(doc);
}

void C_Food::update(sf::Time eTime, const C_ManagersContext& context)
{

}

void C_Food::loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot)
{
    u8 prevFoodType = m_foodType;
    inPacket >> m_foodType;

    if (prevFoodType != m_foodType) {
        updateTextureSubRect();
    }

    BitStream bits;

    u8 byte;
    inPacket >> byte;
    bits.pushByte(byte);

    bool posXChanged = bits.popBit();
    bool posYChanged = bits.popBit();

    if (posXChanged) {
        inPacket >> m_pos.x;
    }

    if (posYChanged) {
        inPacket >> m_pos.y;
    }
}

void C_Food::interpolate(const C_Entity* prevEntity, const C_Entity* nextEntity, double t, double d, bool isControlled)
{

}

void C_Food::copySnapshotData(const C_Entity* snapshotEntity, bool isControlled)
{
    *this = *(static_cast<const C_Food*>(snapshotEntity));
}

void C_Food::insertRenderNode(const C_ManagersContext& managersContext, const Context& context)
{
    std::vector<RenderNode>& renderNodes = managersContext.entityManager->getRenderNodes();
    C_Entity::insertRenderNode(managersContext, context);

    // rarer than common have a glow around them
    if (getRarity() != FOOD_RARITY_COMMON) {
        sf::Sprite& sprite = renderNodes.back().sprite;
        float height = renderNodes.back().height;

        renderNodes.emplace_back(m_uniqueId);
        renderNodes.back().usingSprite = false;
        renderNodes.back().height = height;
        renderNodes.back().drawable = &m_glow;

        //glow should be behind the normal sprite
        renderNodes.back().manualFilter = -1;

        float radius = (float) getCollisionRadius() + 5.f;

        m_glow.setRadius(radius);
        m_glow.setOrigin(radius, radius);
        m_glow.setPosition(getPosition());
        m_glow.setFillColor(getRarityColor(getFoodType()));
    }
}

void C_Food::updateTextureSubRect()
{
    //food texture has 16 columns of icons
    size_t cols = 16;

    m_subTextureRect.left = (m_foodType%cols) * m_subTextureRect.width;
    m_subTextureRect.top = (m_foodType/cols) * m_subTextureRect.height;
}
