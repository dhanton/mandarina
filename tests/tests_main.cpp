#include "../include/quadtree_entity.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>

//Add to a vector of functions 
// testing 3/11 etc
//Check number of tests failed

#include "entities/food.hpp"
#include "quadtree.hpp"
#include "paths.hpp"

#define ASSERT(CONDITION) if (!(CONDITION)) {\
    printf("Assertion failure %s:%d ASSERT(%s)\n", __FILE__, __LINE__, #CONDITION);\
}

void bounding_body_test()
{
    BoundingBodyf big(sf::FloatRect(100.f, 100.f, 200.f, 50.f));
    BoundingBodyf small_inside(sf::FloatRect(200.f, 125, 5, 5.f));
    BoundingBodyf edge_inside(sf::FloatRect(110.f, 110.f, 190.f, 40.f));
    BoundingBodyf edge_outside(sf::FloatRect(300.f, 150.f, 20.f, 5.f));
    BoundingBodyf intersecting1(sf::FloatRect(290.f, 90.f, 29.f, 25.f));
    BoundingBodyf intersecting2(sf::FloatRect(290.f, 110.f, 29.f, 25.f));
    BoundingBodyf outside(sf::FloatRect(290.f, 210.f, 29.f, 25.f));

    BoundingBodyf big_circle(Circlef(150.f, 125.f, 200.f));
    BoundingBodyf inter_circle(Circlef(125.f, 125.f, 40.f));
    BoundingBodyf small_circle(Circlef(150.f, 125.f, 10.f));

    ASSERT(big.Contains(100, 100));
    ASSERT(!big.Contains(300, 150));

    ASSERT(big.Contains(big));
    ASSERT(big.Contains(small_inside));
    ASSERT(!small_inside.Contains(big));
    ASSERT(big.Contains(edge_inside));
    ASSERT(!edge_inside.Contains(big));
    ASSERT(!big.Contains(edge_outside));
    ASSERT(!edge_outside.Contains(big));
    ASSERT(!big.Contains(intersecting1));
    ASSERT(!intersecting1.Contains(big));
    ASSERT(!big.Contains(intersecting2));
    ASSERT(!intersecting1.Contains(big));
    ASSERT(!intersecting1.Contains(intersecting2));
    ASSERT(!big.Contains(outside));
    ASSERT(!outside.Contains(big));

    ASSERT(big.Intersects(big));
    ASSERT(big.Intersects(small_inside));
    ASSERT(small_inside.Intersects(big));
    ASSERT(big.Intersects(edge_inside));
    ASSERT(edge_inside.Intersects(big));
    ASSERT(!big.Intersects(edge_outside));
    ASSERT(!edge_outside.Intersects(big));
    ASSERT(big.Intersects(intersecting1));
    ASSERT(intersecting1.Intersects(big));
    ASSERT(big.Intersects(intersecting2));
    ASSERT(intersecting2.Intersects(big));
    ASSERT(intersecting1.Intersects(intersecting2));
    ASSERT(!big.Intersects(outside));
    ASSERT(!outside.Intersects(big));

    ASSERT(big_circle.Contains(250.f, 125.f));
    ASSERT(big_circle.Contains(150.f, 225.f));
    ASSERT(big_circle.Contains(big));
    
    ASSERT(inter_circle.Intersects(big));
    ASSERT(!inter_circle.Contains(big));
    ASSERT(!big.Contains(inter_circle));

    ASSERT(big_circle.Contains(small_circle));
    ASSERT(!small_circle.Contains(big_circle));
    ASSERT(big_circle.Intersects(small_circle));

    ASSERT(big.Contains(small_circle));
}

void rotating_shape_test()
{
    RotatingRectf rect1(50, 50, 20, 100, 0);
    RotatingRectf rect2(75, 50, 20, 100, 0);
    Circlef circle(70, 50, 9);

    ASSERT(rect1.intersects(circle));

    sf::RenderWindow window{{740, 500}, "Platano Tests", sf::Style::Close | sf::Style::Titlebar};

    sf::RectangleShape rectShape1({rect1.width, rect1.height});
    rectShape1.setOrigin(rect1.getLocalOrigin());
    rectShape1.setPosition(rect1.getCenter());
    rectShape1.rotate(rect1.angle);

    sf::RectangleShape rectShape2({rect2.width, rect2.height});
    rectShape2.setOrigin(rect2.getLocalOrigin());
    rectShape2.setPosition(rect2.getCenter());
    // rectShape2.setFillColor(sf::Color::Red);
    rectShape2.rotate(rect2.angle);

    sf::RectangleShape nonRot = rectShape1;
    nonRot.setFillColor(sf::Color::Red);

    sf::CircleShape circleShape(circle.radius);
    circleShape.setOrigin(circle.radius, circle.radius);
    circleShape.setPosition(circle.center);

    sf::CircleShape inversedCircle(circle.radius);
    inversedCircle.setOrigin(circle.radius, circle.radius);
    inversedCircle.setPosition(circle.center);
    inversedCircle.setFillColor(sf::Color::Red);

    sf::Vector2f originalPoint = inversedCircle.getPosition();

    sf::Clock clock;

    while (window.isOpen()) {
        sf::Time eTime = clock.restart();

        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                window.close();
        }   


        //LOGIC 
        if (rect1.intersects(rect2)) {
            rectShape1.setFillColor(sf::Color::Red);
            rectShape2.setFillColor(sf::Color::Red);
        } else {
            rectShape1.setFillColor(sf::Color::Blue);
            rectShape2.setFillColor(sf::Color::Blue);
        }

        rect1.angle += 5 * eTime.asSeconds();
        rectShape1.setRotation(rect1.angle);
        rect2.angle += 5 * eTime.asSeconds();
        rectShape2.setRotation(rect2.angle);

//        sf::FloatRect globalBounds = rect1.getGlobalBounds();
//        rectShape2.setSize({globalBounds.width, globalBounds.height});
//        rectShape2.setPosition(globalBounds.left, globalBounds.top);

//        sf::Vector2f point = originalPoint;
//        dh::rotatePoint(point, -rect1.angle, rect1.getCenter());
//        inversedCircle.setPosition(point);

//        std::cout << rect1.contains(circle) << ' ' << Circlef(inversedCircle.getPosition(), inversedCircle.getRadius()).inside(rect1.getNonRotatingRect()) << std::endl;

        //RENDERING
        window.clear();

//        window.draw(inversedCircle);
//        window.draw(nonRot);
        window.draw(rectShape2);
        window.draw(rectShape1);
//        window.draw(circleShape);

        window.display();


    }
}

void rotating_and_circle_test()
{
    sf::RenderWindow window{{1200, 800}, "Platano Tests", sf::Style::Close | sf::Style::Titlebar};
    window.setFramerateLimit(60);
    
    using QuadtreeType = Quadtree<float, QuadtreeEntity<float>, SimpleExtractor<float>>;

    QuadtreeType quadtree;

    RotatingRectf rect({0.f, 0.f}, {2000.f, 10.f}, rand() % 360);

    sf::Clock clock;

    constexpr size_t NUM = 50;

    //////////////////////////////////////////////////////IMPORTANT////////////////////////////////////////////////////
    //it has to be stored dynamically
    //probably because of how the quadtree internally allocates memory
    using TestType = std::unique_ptr<QuadtreeEntity<float>>;

    std::vector<Circlef> circles;
    std::vector<TestType> entities;
    std::vector<u32> ids;
    std::vector<bool> hits;
    u32 lastId = 0;

    srand(time(0));

    //populate vectors
    for (int i = 0; i < NUM; ++i) {
        circles.push_back(Circlef(rand() % 1200, rand() % 800, rand() % 20 + 30));
        hits.push_back(false);
        entities.push_back(TestType(new QuadtreeEntity<float>(++lastId, circles[i])));
        
        quadtree.Insert(entities[i].get());

        ids.push_back(lastId);
    }

    while (window.isOpen()) {
        sf::Time eTime = clock.restart();

        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                window.close();
        }

        rect.angle += 10 * eTime.asSeconds();

        //clear current hits
        for (int i = 0; i < NUM; ++i) {
            hits[i] = false;
        }

        auto result = quadtree.QueryIntersectsRegion(BoundingBodyf(rect));
        int n = 0;

        while (!result.EndOfQuery()) {
            auto* entity = result.GetCurrent();

            n++;

            hits[entity->uniqueId - 1] = true;

            result.Next();
        }

        window.clear();

        sf::RectangleShape shape;
        shape.setSize({rect.width, rect.height});
        shape.setOrigin(rect.width/2.f, rect.height/2.f);
        shape.setPosition({rect.left + rect.width/2.f, rect.top + rect.height/2.f});
        shape.setRotation(rect.angle);
        shape.setFillColor(sf::Color::Green);
        window.draw(shape);

        for (int i = 0; i < NUM; ++i) {
            Circlef& circle = circles[i];

            sf::CircleShape circleShape;
            circleShape.setRadius(circle.radius);
            circleShape.setOrigin(circle.radius, circle.radius);
            circleShape.setPosition(circle.center);

            circleShape.setFillColor(hits[i] ? sf::Color::Red : sf::Color::Cyan);

            window.draw(circleShape);
        }

        window.display();
    }
}

void food_distribution_test()
{
    JsonParser json;
    json.loadAll("../../data/json");

    Food food;
    food.loadFromJson(*json.getDocument("food"));

    std::vector<int> counts(MAX_FOOD_RARITY_TYPES, 0);

    for (int i = 0; i < 100000; ++i) {
        u8 foodType = FoodBase::getRandomFood();

        food.setFoodType(foodType);

        ++counts[food.getRarity()];
    }

    //Answer should be ~ 60'000 30'000 9'000 1'000
    for (int n : counts) std::cout << n << " ";
    std::cout << std::endl;
}

int main()
{
    // rotating_shape_test();
    //rotating_and_circle_test();
    food_distribution_test();
    return 0;
}
