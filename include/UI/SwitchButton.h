#ifndef UI_SWITCHBUTTON_H
#define UI_SWITCHBUTTON_H

#include "UI/Vidget.h"

class SwitchButton : public Vidget
{
public:
    SwitchButton(const sf::Font& font, double font_size, const vec2d& position);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    bool handleEvent(const sf::Event& event) override;

    void addText(const std::string& text);
    void setButtonColor(const sf::Color& color);
    void setTextColor(const sf::Color& color);

    size_t value() const;

private:
    void update() override;

    sf::Text text_;
    float font_size_;
    int offset_;

    std::vector<std::string> texts_;
    size_t value_ = 0;
};

#endif // UI_SWITCHBUTTON_H