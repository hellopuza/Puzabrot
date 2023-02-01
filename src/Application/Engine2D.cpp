#include "Application/Engine2D.h"
#include "Utils.h"

#include <algorithm>
#include <cmath>
#include <sstream>

Engine2D::Engine2D(const vec2i& size, const char* title) :
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
        WheelZooming(static_cast<double>(event.mouseWheel.delta), Screen2Base(vec(sf::Mouse::getPosition(*this))));
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

void Engine2D::setZoomingRatio(double zooming_ratio)
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

void Engine2D::drawGrid(const sf::Font& font, int font_size)
{
    sf::Color color = sf::Color::White;

    vec2d ratio = getRatio();
    sf::VertexArray grid(sf::Lines);

    std::vector<sf::Text> labels;

    double ratio_div = ratio.x / ratio.y;

    const double min_label_pos = ratio_div * 0.01;
    double x = 0.0;
    auto makeLabel = [&](const vec2d& label_pos, const vec2d& v1, const vec2d& v2)
    {
        std::stringstream number_text;
        if (std::abs(x) > min_label_pos)
        {
            number_text << x;
        }

        labels.emplace_back(sf::Text(number_text.str(), font, font_size));
        labels.back().setFillColor(color);
        labels.back().setOutlineThickness(0.8F);
        labels.back().setPosition(vec(getLabelPos(label_pos, vec2d(labels.back().getLocalBounds().width, labels.back().getLocalBounds().height))));

        grid.append(sf::Vertex(vec(Base2Screen(v1)), color));
        grid.append(sf::Vertex(vec(Base2Screen(v2)), color));
        x += ratio_div;
    };

    x = borders_.bottom - std::fmod(borders_.bottom, ratio_div) + ratio_div * static_cast<double>(borders_.bottom > 0.0);
    while (x < borders_.top)
    {
        makeLabel(vec2d(0.0, x), vec2d(borders_.left, x), vec2d(borders_.right, x));
    }

    x = borders_.left - std::fmod(borders_.left, ratio_div) + ratio_div * static_cast<double>(borders_.left > 0.0);
    while (x < borders_.right)
    {
        makeLabel(vec2d(x, 0.0), vec2d(x, borders_.bottom), vec2d(x, borders_.top));
    }

    draw(grid);

    for (const auto& label : labels)
    {
        draw(label);
    }
}

void Engine2D::updateSize(const vec2i& size)
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
        updateSize(vec2i(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height));
    }
}

void Engine2D::WheelZooming(double wheel_delta, const vec2d& point)
{
    double width = borders_.right - borders_.left;
    double height = borders_.top - borders_.bottom;

    double x_ratio = (point.x - borders_.left) / width;
    double y_ratio = (point.y - borders_.bottom) / height;

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
    double width = borders_.right - borders_.left;
    double height = borders_.top - borders_.bottom;

    Borders new_frame = {
        borders_.left - width * movement.x / getSize().x,
        borders_.right - width * movement.x / getSize().x,
        borders_.bottom + height * movement.y / getSize().y,
        borders_.top + height * movement.y / getSize().y,
    };

    borders_ = new_frame;
}

vec2d Engine2D::getLabelPos(const vec2d& point, const vec2d& text_size) const
{
    return clamp(Base2Screen(point), vec2i(), size_ - text_size * vec2d(1.0, 1.5));
}

vec2d Engine2D::getRatio() const
{
    const double initial_ratio = 10.0;
    static vec2d ratio(initial_ratio, initial_ratio);

    double ratio_div = ratio.x / ratio.y;

    auto size = getSize();
    double screen_square = ratio_div * ratio_div * static_cast<double>(size.x * size.y) /
        ((borders_.right - borders_.left) * (borders_.top - borders_.bottom));

    const double max_square = 50000.0;
    const double min_square = 8000.0;

    constexpr double TWO = 2.0;
    constexpr double FIVE = 5.0;
    constexpr double TEN = 10.0;

    if (screen_square > max_square)
    {
        switch (static_cast<int>(ratio.y))
        {
        case static_cast<int>(TWO):
            ratio = vec2d(ratio.x, FIVE);
            break;
        case static_cast<int>(FIVE):
            ratio = vec2d(ratio.x, TEN);
            break;
        case static_cast<int>(TEN):
            ratio = vec2d(ratio.x / TEN, TWO);
            break;
        }
    }
    else if (screen_square < min_square)
    {
        switch (static_cast<int>(ratio.y))
        {
        case static_cast<int>(TWO):
            ratio = vec2d(ratio.x * TEN, TEN);
            break;
        case static_cast<int>(FIVE):
            ratio = vec2d(ratio.x, TWO);
            break;
        case static_cast<int>(TEN):
            ratio = vec2d(ratio.x, FIVE);
            break;
        }
    }
    return ratio;
}