#include "Application/Application.h"
#include "Utils.h"

Application::Application(const vec2i& win_size, const char* font_location, double font_size, const char* win_title) :
    Engine2D(win_size, win_title), font_size_(DIP2Pixels(font_size))
{
    font_.loadFromFile(font_location);
}

void Application::run()
{
    prerun();

    while (isOpen())
    {
        sf::Event event;
        while (pollEvent(event))
        {
            handleAppEvent(event);
        }
        activity();
    }

    postrun();
}

void Application::drawGrid()
{
    Engine2D::drawGrid(font_, font_size_);
}

const sf::Font& Application::getFont() const
{
    return font_;
}