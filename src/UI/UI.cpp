#include "UI/UI.h"

UI::UI() : Vidget(vec2d()) {}

void UI::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (is_visible)
    {
        for (const auto& [name, vidget] : vidgets_)
        {
            target.draw(*vidget.get(), states);
        }
    }
}

bool UI::handleEvent(const sf::Event& event)
{
    for (const auto& [name, vidget] : vidgets_)
    {
        if (vidget->handleEvent(event))
        {
            return true;
        }
    }

    return false;
}

void UI::addVidget(const std::string& name, Vidget* vidget)
{
    vidgets_.emplace(name, vidget);
    vidgets_[name]->show();
}

Vidget* UI::getVidget(const std::string& name)
{
    return vidgets_[name].get();
}

void UI::update() {}