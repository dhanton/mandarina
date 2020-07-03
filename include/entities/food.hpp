#pragma once

#include <SFML/Graphics/CircleShape.hpp>
#include "entity.hpp"

enum FoodType {
    FOOD_APPLE,
    FOOD_BANANA,
    FOOD_PEAR,
    FOOD_LIME,
    FOOD_STRAWBERRY,
    FOOD_BERRY,
    FOOD_CARROT,
    FOOD_CORN,
    FOOD_GARLIC,
    FOOD_TOMATO,
    FOOD_EGGPLANT,
    FOOD_PEPPER,
    FOOD_FUNGUS,
    FOOD_BREAD,
    FOOD_BREAD_LOAF,
    FOOD_CHICKEN,
    FOOD_CHICKENLEG,
    FOOD_BEEF,
    FOOD_BEEFLEG,
    FOOD_ROASTED_BEEF,
    FOOD_FISH,
    FOOD_EGGS,
    FOOD_EGG,
    FOOD_CHEESE,
    FOOD_SALT,
    FOOD_HONEY,
    FOOD_COKE,
    FOOD_BROWN_COKE,
    FOOD_CANDY,
    FOOD_CAKE,
    FOOD_COFFEE,

    MAX_FOOD_TYPES
};

enum FoodRarityType {
    FOOD_RARITY_COMMON,
    FOOD_RARITY_UNCOMMON,
    FOOD_RARITY_RARE,
    FOOD_RARITY_MYTHIC,

    MAX_FOOD_RARITY_TYPES
};

class FoodBase {
public:
    u8 getFoodType() const;
    void setFoodType(u8 foodType);

    u8 getRarity() const;

    static float getDropRate(u8 foodType);
    static u32 getPowerGiven(u8 foodType);
    static sf::Color getRarityColor(u8 foodType);

    static u8 getRandomFood();
    static void scatterFood(const Vector2& pos, const std::vector<u8>& foodVec, const ManagersContext& context);
    static void scatterRandomFood(const Vector2& pos, int foodAmount, const ManagersContext& context);

protected:
    u8 m_foodType;

    void loadFoodData(const rapidjson::Document& doc);

private:
    static FoodRarityType m_rarityType[MAX_FOOD_TYPES];
    static u8 m_dropRate[MAX_FOOD_RARITY_TYPES];
    static u32 m_powerGiven[MAX_FOOD_RARITY_TYPES];
    static sf::Color m_color[MAX_FOOD_RARITY_TYPES];
    static u8 m_rarityOffset[MAX_FOOD_RARITY_TYPES + 1];
    
    //used internally by this class
    static float m_collisionRadius;

    static bool m_foodDataLoaded;
};

class Food : public Entity, public FoodBase
{
public:
    Food* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc);

    virtual void update(sf::Time eTime, const ManagersContext& context);
    virtual void preUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void postUpdate(sf::Time eTime, const ManagersContext& context);
    virtual void packData(const Entity* prevEntity, u8 teamId, u32 controlledEntityUniqueId, CRCPacket& outPacket) const;
};

class C_Food : public C_Entity, public FoodBase
{
public:
    C_Food* clone() const;

    virtual void loadFromJson(const rapidjson::Document& doc, u16 textureId, const Context& context);

    virtual void update(sf::Time eTime, const C_ManagersContext& context);
    virtual void loadFromData(u32 controlledEntityUniqueId, CRCPacket& inPacket, CasterSnapshot& casterSnapshot);
    virtual void interpolate(const C_Entity* prevEntity, const C_Entity* nextEntity, double t, double d, bool isControlled);
    virtual void copySnapshotData(const C_Entity* snapshotEntity, bool isControlled);

    virtual void insertRenderNode(const C_ManagersContext& managersContext, const Context& context);

private:
    void updateTextureSubRect();

    sf::CircleShape m_glow;
};
