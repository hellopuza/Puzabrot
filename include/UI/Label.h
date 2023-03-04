#ifndef UI_LABEL_H
#define UI_LABEL_H

#include "UI/Vidget.h"

class Label : public Vidget
{
public:
    Label(const sf::Font& font, float font_size, const vec2f& position);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    bool handleEvent(const sf::Event& event) override;

    void setText(const std::string& text);

private:
    void update() override;

    sf::Text text_;
};

#endif // UI_LABEL_H