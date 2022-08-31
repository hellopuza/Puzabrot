#ifndef ENGINE2D_H
#define ENGINE2D_H

#include "Base2D.h"
#include "Eventable.h"

class Engine2D : public sf::RenderWindow, public Base2D, public Eventable
{
public:
    Engine2D(const sf::Vector2u& size, const sf::String& title, sf::Uint32 style = sf::Style::Default);

    bool handleEvent(const sf::Event& event) override;
    void setZoomingRatio(double zooming_ratio);

    void drawPlane(sf::RenderTarget& target, sf::RenderStates states, const sf::Color& plane_color) const;
    void drawAxes(sf::RenderTarget& target, sf::RenderStates states, const sf::Color& axis_color) const;
    void drawGrid(sf::RenderTarget& target, sf::RenderStates states, const sf::Font& font,
        const sf::Color& grid_color, const sf::Color& text_color, unsigned symbol_size) const;

private:
    void updateSize(const sf::Vector2u& size);
    void toggleFullScreen();
    void WheelZooming(double wheel_delta, point_t point);
    void MouseMoving(const sf::Vector2i& movement);
    point_t getLabelPos(point_t point, unsigned symbol_size) const;
    std::pair<double, double> getRatio() const;

    sf::Vector2u default_size_;
    sf::String title_;
    sf::Uint32 style_;
    double zooming_ratio_ = 1.0;
};

#endif // ENGINE2D_H