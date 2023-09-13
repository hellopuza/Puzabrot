#ifndef UI_FPS_H
#define UI_FPS_H

#include "UI/Vidget.h"

class FPS : public Vidget
{
public:
    FPS(const sf::Font& font, float font_size, const vec2f& position);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    bool handleEvent(const sf::Event& event) override;

    void update_frame();
    void keep(float fps);
    float current() const;

private:
    void update() override;

    sf::Clock dt_clock_;
    sf::Text  text_;

    float current_ = 0.0F;
    size_t frame_num_ = 0;
};

#endif // UI_FPS_H