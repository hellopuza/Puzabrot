#ifndef APPLICATION_ENGINE2D_H
#define APPLICATION_ENGINE2D_H

#define SFML_DEFINE_DISCRETE_GPU_PREFERENCE

#include "Application/Base2D.h"
#include "Eventable.h"

class Engine2D : public sf::RenderWindow, public Base2D, public Eventable
{
public:
    Engine2D(const vec2u& size, const char* title = "");

    bool handleEvent(const sf::Event& event) override;
    void setZoomingRatio(float zooming_ratio);

    void drawBackground(const sf::Color& background_color);
    void drawGrid(const sf::Font& font, unsigned font_size);

private:
    void updateSize(const vec2u& size);
    void toggleFullScreen();
    void WheelZooming(float wheel_delta, const vec2f& point);
    void MouseMoving(const vec2i& movement);
    vec2f getLabelPos(const vec2f& point, const vec2f& text_size) const;
    vec2f getRatio() const;

    vec2u default_size_;
    const char* title_;
    sf::Uint32 style_;
    float zooming_ratio_ = 1.0F;
};

#endif // APPLICATION_ENGINE2D_H