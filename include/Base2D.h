#ifndef BASE2D_H
#define BASE2D_H

#include <SFML/Graphics.hpp>

class Base2D : public sf::Drawable
{
public:
    using point_t = sf::Vector2<double>;

    struct Borders final
    {
        double left = -1.0;
        double right = 1.0;
        double bottom = -1.0;
        double top = 1.0;

        Borders() = default;
        Borders(double left_, double right_, double bottom_, double top_);
    };

    Base2D(const sf::Vector2u& size);
    virtual ~Base2D() = default;

protected:
    void setSizeInPixels(const sf::Vector2u& size);
    sf::Vector2u getSizeInPixels() const;

public:
    void setBorders(double bottom_, double top_, double right_left_ratio = 1.0);
    Borders getBorders() const;

    point_t Screen2Base(sf::Vector2i pixel) const;
    sf::Vector2i Base2Screen(point_t point) const;

protected:
    Borders borders_;
    sf::Vector2u size_;
};

#endif // BASE2D_H