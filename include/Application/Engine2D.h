#ifndef APPLICATION_ENGINE2D_H
#define APPLICATION_ENGINE2D_H

#include "Application/Base2D.h"
#include "Eventable.h"

class Engine2D : public sf::RenderWindow, public Base2D, public Eventable
{
public:
    Engine2D(const vec2i& size, const char* title = "");

    bool handleEvent(const sf::Event& event) override;
    void setZoomingRatio(double zooming_ratio);

    void drawBackground(const sf::Color& background_color);
    void drawGrid(const sf::Font& font, int font_size);

private:
    void updateSize(const vec2i& size);
    void toggleFullScreen();
    void WheelZooming(double wheel_delta, const vec2d& point);
    void MouseMoving(const vec2i& movement);
    vec2d getLabelPos(const vec2d& point, const vec2d& text_size) const;
    vec2d getRatio() const;

    vec2u default_size_;
    const char* title_;
    sf::Uint32 style_;
    double zooming_ratio_ = 1.0;
};

#endif // APPLICATION_ENGINE2D_H