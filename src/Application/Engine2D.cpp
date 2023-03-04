#include "Application/Engine2D.h"
#include "Utils.h"

#include <algorithm>
#include <cmath>
#include <sstream>

Engine2D::Engine2D(const vec2u& size, const char* title) :
    sf::RenderWindow(sf::VideoMode(size.x, size.y), title, sf::Style::Default), Base2D(size),
    default_size_(size), title_(title), style_(sf::Style::Default)
{}

bool Engine2D::handleEvent(const sf::Event& event)
{
    static vec2i mouse_pos;
    static bool dragging = false;

    // Close window
    if ((event.type == sf::Event::Closed) || ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
    {
        close();
        return true;
    }

    // Toggle fullscreen
    if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::F11))
    {
        toggleFullScreen();
        return true;
    }

    // Resize window
    if (event.type == sf::Event::Resized)
    {
        sf::FloatRect visible_area(0.0F, 0.0F, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
        setView(sf::View(visible_area));
        updateSize(vec(getSize()));

        return true;
    }

    // Zooming
    if (event.type == sf::Event::MouseWheelMoved)
    {
        WheelZooming(static_cast<float>(event.mouseWheel.delta), Screen2Base(vec(sf::Mouse::getPosition(*this))));
        return true;
    }

    // Moving
    if ((event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
    {
        mouse_pos = vec2i(event.mouseButton.x, event.mouseButton.y);
        dragging = true;
    }
    else if ((event.type == sf::Event::MouseMoved) && sf::Mouse::isButtonPressed(sf::Mouse::Left) && dragging)
    {
        vec2i new_pos(event.mouseMove.x, event.mouseMove.y);
        MouseMoving(new_pos - mouse_pos);
        mouse_pos = new_pos;

        return true;
    }
    if ((event.type == sf::Event::MouseButtonReleased) && (event.mouseButton.button == sf::Mouse::Left))
    {
        dragging = false;
    }

    return false;
}

void Engine2D::setZoomingRatio(float zooming_ratio)
{
    zooming_ratio_ = zooming_ratio;
}

void Engine2D::drawBackground(const sf::Color& plane_color)
{
    sf::RectangleShape rectangle;
    rectangle.setSize(sf::Vector2f(getSize()));
    rectangle.setFillColor(plane_color);
    rectangle.setPosition(0.0F, 0.0F);
    draw(rectangle);
}

void Engine2D::drawGrid(const sf::Font& font, unsigned font_size)
{
    sf::Color color = sf::Color::White;

    vec2f ratio = getRatio();
    sf::VertexArray grid(sf::Lines);

    std::vector<sf::Text> labels;

    float ratio_div = ratio.x / ratio.y;

    const float min_label_pos = ratio_div * 0.01F;
    float x = 0.0F;
    auto makeLabel = [&](const vec2f& label_pos, const vec2f& v1, const vec2f& v2)
    {
        std::stringstream number_text;
        if (std::abs(x) > min_label_pos)
        {
            number_text << x;
        }

        labels.emplace_back(sf::Text(number_text.str(), font, font_size));
        labels.back().setFillColor(color);
        labels.back().setOutlineThickness(0.8F);
        labels.back().setPosition(vec(getLabelPos(label_pos, vec2f(labels.back().getLocalBounds().width, labels.back().getLocalBounds().height))));

        grid.append(sf::Vertex(vec(Base2Screen(v1)), color));
        grid.append(sf::Vertex(vec(Base2Screen(v2)), color));
        x += ratio_div;
    };

    x = borders_.bottom - std::fmod(borders_.bottom, ratio_div) + ratio_div * static_cast<float>(borders_.bottom > 0.0F);
    while (x < borders_.top)
    {
        makeLabel(vec2f(0.0F, x), vec2f(borders_.left, x), vec2f(borders_.right, x));
    }

    x = borders_.left - std::fmod(borders_.left, ratio_div) + ratio_div * static_cast<float>(borders_.left > 0.0F);
    while (x < borders_.right)
    {
        makeLabel(vec2f(x, 0.0F), vec2f(x, borders_.bottom), vec2f(x, borders_.top));
    }

    draw(grid);

    for (const auto& label : labels)
    {
        draw(label);
    }
}

void Engine2D::updateSize(const vec2u& size)
{
    setBaseSize(size);
}

void Engine2D::toggleFullScreen()
{
    if (style_ & sf::Style::Fullscreen)
    {
        style_ &= static_cast<sf::Uint32>(~sf::Style::Fullscreen);
        create(sf::VideoMode(default_size_.x, default_size_.y), title_, style_);
        updateSize(default_size_);
    }
    else
    {
        style_ |= static_cast<sf::Uint32>(sf::Style::Fullscreen);
        create(sf::VideoMode::getDesktopMode(), title_, style_);
        updateSize(vec2u(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height));
    }
}

void Engine2D::WheelZooming(float wheel_delta, const vec2f& point)
{
    float width = borders_.right - borders_.left;
    float height = borders_.top - borders_.bottom;

    float x_ratio = (point.x - borders_.left) / width;
    float y_ratio = (point.y - borders_.bottom) / height;

    Borders new_frame = {
        borders_.left + x_ratio * zooming_ratio_ * width * wheel_delta,
        borders_.right - (1 - x_ratio) * zooming_ratio_ * width * wheel_delta,
        borders_.bottom + y_ratio * zooming_ratio_ * height * wheel_delta,
        borders_.top - (1 - y_ratio) * zooming_ratio_ * height * wheel_delta,
    };

    borders_ = new_frame;
}

void Engine2D::MouseMoving(const vec2i& movement)
{
    float width = borders_.right - borders_.left;
    float height = borders_.top - borders_.bottom;
    vec2f delta = vec2f(movement) / vec(getSize());

    Borders new_frame = {
        borders_.left - width * delta.x,
        borders_.right - width * delta.x,
        borders_.bottom + height * delta.y,
        borders_.top + height * delta.y,
    };

    borders_ = new_frame;
}

vec2f Engine2D::getLabelPos(const vec2f& point, const vec2f& text_size) const
{
    return clamp(Base2Screen(point), vec2i(), vec2i(size_ - text_size * vec2f(1.0F, 1.5F)));
}

vec2f Engine2D::getRatio() const
{
    const float initial_ratio = 10.0F;
    static vec2f ratio(initial_ratio, initial_ratio);

    float ratio_div = ratio.x / ratio.y;

    auto size = getSize();
    float screen_square = ratio_div * ratio_div * static_cast<float>(size.x * size.y) /
        ((borders_.right - borders_.left) * (borders_.top - borders_.bottom));

    const float max_square = 50000.0F;
    const float min_square = 8000.0F;

    constexpr float TWO = 2.0F;
    constexpr float FIVE = 5.0F;
    constexpr float TEN = 10.0F;

    if (screen_square > max_square)
    {
        switch (static_cast<int>(ratio.y))
        {
        case static_cast<int>(TWO):
            ratio = vec2f(ratio.x, FIVE);
            break;
        case static_cast<int>(FIVE):
            ratio = vec2f(ratio.x, TEN);
            break;
        case static_cast<int>(TEN):
            ratio = vec2f(ratio.x / TEN, TWO);
            break;
        }
    }
    else if (screen_square < min_square)
    {
        switch (static_cast<int>(ratio.y))
        {
        case static_cast<int>(TWO):
            ratio = vec2f(ratio.x * TEN, TEN);
            break;
        case static_cast<int>(FIVE):
            ratio = vec2f(ratio.x, TWO);
            break;
        case static_cast<int>(TEN):
            ratio = vec2f(ratio.x, FIVE);
            break;
        }
    }
    return ratio;
}