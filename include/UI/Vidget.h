#ifndef UI_VIDGET_H
#define UI_VIDGET_H

#include "Eventable.h"
#include "vec2.h"

class Vidget : public sf::Drawable, public Eventable
{
public:
    Vidget(const vec2d& position);

    vec2d getPosition() const;
    vec2d getSize() const;

    void setPosition(const vec2d& position);
    void hide();
    void show();

    bool is_visible = false;

protected:
    virtual void update() = 0;

    sf::RectangleShape box_;
};

#endif // UI_VIDGET_H