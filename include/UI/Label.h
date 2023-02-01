#ifndef UI_LABEL_H
#define UI_LABEL_H

#include "UI/Vidget.h"

class Label : public Vidget
{
public:
    Label(const sf::Font& font, double font_size, const vec2d& position);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    bool handleEvent(const sf::Event& event) override;

    void setText(const std::string& text);

private:
    void update() override;

    vec2f font_size_;

    sf::Text text_;
    int offset_;
};

#endif // UI_LABEL_H