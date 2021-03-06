#ifndef INPUTBOX_H
#define INPUTBOX_H

#include <SFML/Graphics.hpp>

class InputBox
{
public:
    InputBox();
    InputBox(sf::Vector2f box_pos, sf::Color box_color, sf::Color text_color, float font_size);

    void draw(sf::RenderWindow& window);

    void setInput(const sf::String& text);
    void setLabel(const sf::String& text);
    void setOutput(const sf::String& text);

    const sf::String& getInput() const;
    const sf::Vector2f& getPosition() const;
    const sf::Vector2f& getSize() const;

    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void hide();
    void show();
    bool isVisible() const;

    bool handleEvent(const sf::Event& event);
    bool TextEntered() const;
    bool hasFocus() const;

private:
    sf::Text label_;
    sf::Text input_text_;
    sf::Text output_text_;

    sf::Vector2f box_pos_  = { 0, 0 };
    sf::Vector2f box_size_ = { 0, 0 };
    sf::Color box_color_;
    sf::Color text_color_;
    sf::Font font_;
    float font_size_;

    bool has_focus_ = false;
    bool is_visible_ = false;
    bool text_entered_ = false;
};

#endif // INPUTBOX_H