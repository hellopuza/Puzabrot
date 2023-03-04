#ifndef UI_INPUTBOX_H
#define UI_INPUTBOX_H

#include "UI/Vidget.h"

class InputBox : public Vidget
{
public:
    InputBox(const sf::Font& font, float font_size, const vec2f& position);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    bool handleEvent(const sf::Event& event) override;

    void setLabel(const std::string& text);
    void setInput(const std::string& text);
    void setOutput(const std::string& text);
    std::string getInput() const;

    bool TextEntered() const;
    bool hasFocus() const;

private:
    void update() override;
    void setColors();

    vec2f font_size_;

    std::string label_;
    std::string input_;
    std::string output_;

    sf::Text label_text_;
    sf::Text input_text_;
    sf::Text output_text_;

    sf::RectangleShape input_box_;
    float offset_;

    bool has_focus_ = false;
    bool text_entered_ = false;
};

#endif // UI_INPUTBOX_H