#ifndef EVENTABLE_H
#define EVENTABLE_H

#include <SFML/Graphics.hpp>

struct Eventable
{
    Eventable() = default;
    virtual ~Eventable() = default;

    virtual bool handleEvent(const sf::Event& event) = 0;
};

#endif // EVENTABLE_H