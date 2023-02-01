#ifndef APPLICATION_APPLICATION_H
#define APPLICATION_APPLICATION_H

#include "Application/Engine2D.h"

class Application : public Engine2D
{
public:
    Application(const vec2i& win_size, const char* font_location, double font_size, const char* win_title = "");

    void run();

    virtual void prerun() = 0;
    virtual void handleAppEvent(const sf::Event& event) = 0;
    virtual void activity() = 0;
    virtual void postrun() = 0;

    void drawGrid(const sf::Font& font, int font_size) = delete;
    void drawGrid();
    const sf::Font& getFont() const;

private:
    sf::Font font_;
    int font_size_;
};

#endif // APPLICATION_APPLICATION_H