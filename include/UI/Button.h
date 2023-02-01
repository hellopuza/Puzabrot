#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "UI/Vidget.h"

class Button : public Vidget
{
public:
    Button(const sf::Font& font, double font_size, const vec2d& position);
    Button(const vec2d& position, const vec2d& size);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    bool handleEvent(const sf::Event& event) override;

    void setText(const std::string& text);
    void setFont(const sf::Font& font, double font_size);
    void setSize(const vec2d& size);
    void setButtonColors(const sf::Color& off_color, const sf::Color& on_color);
    void setTextColors(const sf::Color& off_color, const sf::Color& on_color);

    bool pressed() const;

private:
    void update() override;
    void setColors();

    sf::Text text_;
    double font_size_;
    int offset_;

    sf::Color button_off_color_ = { 128, 128, 128, 128 };
    sf::Color button_on_color_ = { 200, 200, 200, 255 };
    sf::Color text_off_color_ = { 255, 255, 255, 255 };
    sf::Color text_on_color_ = { 50, 50, 50, 255 };

    bool pressed_ = false;
};

#endif // UI_BUTTON_H