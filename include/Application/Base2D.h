#ifndef APPLICATION_BASE2D_H
#define APPLICATION_BASE2D_H

#include "vec2.h"

class Base2D
{
public:
    struct Borders final
    {
        float left = -1.0F;
        float right = 1.0F;
        float bottom = -1.0F;
        float top = 1.0F;

        Borders() = default;
        Borders(float left_, float right_, float bottom_, float top_);
    };

    Base2D(const vec2u& size = {});
    virtual ~Base2D() = default;

    void setBaseSize(const vec2u& size);
    void setBorders(float bottom_, float top_, float right_left_ratio = 1.0F);
    Borders getBorders() const;

    vec2f Screen2Base(const vec2i& pixel) const;
    vec2i Base2Screen(const vec2f& point) const;

protected:
    Borders borders_;
    vec2u size_;
};

#endif // APPLICATION_BASE2D_H