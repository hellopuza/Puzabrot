#ifndef APPLICATION_BASE2D_H
#define APPLICATION_BASE2D_H

#include "vec2.h"

class Base2D
{
public:
    struct Borders final
    {
        double left = -1.0;
        double right = 1.0;
        double bottom = -1.0;
        double top = 1.0;

        Borders() = default;
        Borders(double left_, double right_, double bottom_, double top_);
    };

    Base2D(const vec2i& size = {});
    virtual ~Base2D() = default;

    void setBaseSize(const vec2i& size);
    void setBorders(double bottom_, double top_, double right_left_ratio = 1.0);
    Borders getBorders() const;

    vec2d Screen2Base(const vec2i& pixel) const;
    vec2i Base2Screen(const vec2d& point) const;

protected:
    Borders borders_;
    vec2i size_;
};

#endif // APPLICATION_BASE2D_H