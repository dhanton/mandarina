#pragma once

#include "buff.hpp"

class SlowBuff : public Buff
{
public:
    SlowBuff* clone() const;

    void loadFromJson(const rapidjson::Document& doc);

    void onPreUpdate(sf::Time eTime);

private:
    int m_slowAmount;
};
