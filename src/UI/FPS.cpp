#include "UI/FPS.h"
#include "Utils.h"

FPS::FPS(const sf::Font& font, float font_size, const vec2f& position) : Vidget(position)
{
    text_.setFont(font);
    text_.setCharacterSize(static_cast<unsigned>(DIP2Pixels(font_size)));

    text_.setFillColor(sf::Color::White);
    text_.setOutlineThickness(1.0);
    text_.setOutlineColor(sf::Color::Black);
    update();
}

void FPS::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (is_visible)
    {
        target.draw(text_, states);
    }
}

bool FPS::handleEvent(const sf::Event&)
{
    return false;
}

void FPS::update_frame()
{
    ++frame_num_;
    static float time = 0;
    time += dt_clock_.restart().asSeconds();

    if (frame_num_ == 10)
    {
        char fps_str[32] = "";
        current_ = static_cast<float>(frame_num_) / time;
        sprintf(fps_str, "FPS: %.0lf\n", current_);
        text_.setString(fps_str);
        frame_num_ = 0;
        time = 0;
    }
}

void FPS::keep(float fps)
{
    float max_time = 1.0F / fps;

    ++frame_num_;
    static float time = 0;

    float dt = dt_clock_.restart().asSeconds();
    if (dt < max_time)
    {
        sf::sleep(sf::seconds(max_time - dt));
        time += max_time;
    }
    else
    {
        time += dt;
    }

    if (frame_num_ == 10)
    {
        char fps_str[32] = "";
        current_ = static_cast<float>(frame_num_) / time;
        sprintf(fps_str, "FPS: %.0lf\n", current_);
        text_.setString(fps_str);
        frame_num_ = 0;
        time = 0;
    }
}

float FPS::current() const
{
    return current_;
}

void FPS::update()
{
    text_.setPosition(box_.getPosition());
    box_.setSize(vec((text_.getString().getSize() == 0) ? vec2f() : vec2f(text_.getLocalBounds().width, text_.getLocalBounds().height)));
}
