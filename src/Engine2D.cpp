#include "Engine2D.h"

#include <algorithm>
#include <cmath>
#include <sstream>

Engine2D::Engine2D(const sf::Vector2u& size, const sf::String& title, sf::Uint32 style) :
    sf::RenderWindow(sf::VideoMode(size.x, size.y), title, style), Base2D(size), default_size_(size), title_(title), style_(style)
{}

bool Engine2D::handleEvent(const sf::Event& event)
{
    static sf::Vector2i mouse_pos;
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
        updateSize(sf::Vector2u(getSize().x, getSize().y));

        return true;
    }

    // Zooming
    if (event.type == sf::Event::MouseWheelMoved)
    {
        WheelZooming(static_cast<double>(event.mouseWheel.delta), Screen2Base(sf::Mouse::getPosition(*this)));
        return true;
    }

    // Moving
    if ((event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
    {
        mouse_pos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
        dragging = true;
    }
    else if ((event.type == sf::Event::MouseMoved) && sf::Mouse::isButtonPressed(sf::Mouse::Left) && dragging)
    {
        sf::Vector2i new_pos(event.mouseMove.x, event.mouseMove.y);
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

void Engine2D::drawPlane(sf::RenderTarget& target, sf::RenderStates states, const sf::Color& plane_color) const
{
    sf::RectangleShape rectangle;
    rectangle.setSize(sf::Vector2f(getSize()));
    rectangle.setFillColor(plane_color);
    rectangle.setPosition(0.0F, 0.0F);
    target.draw(rectangle, states);
}

void Engine2D::drawAxes(sf::RenderTarget& target, sf::RenderStates states, const sf::Color& axis_color) const
{
    sf::Vertex horizontal[] = {
        sf::Vertex(sf::Vector2f(Base2Screen(point_t(borders_.left,  0.0))), axis_color),
        sf::Vertex(sf::Vector2f(Base2Screen(point_t(borders_.right, 0.0))), axis_color)
    };

    sf::Vertex vertical[] = {
        sf::Vertex(sf::Vector2f(Base2Screen(point_t(0.0, borders_.bottom))), axis_color),
        sf::Vertex(sf::Vector2f(Base2Screen(point_t(0.0, borders_.top))), axis_color)
    };

    target.draw(horizontal, 2, sf::Lines, states);
    target.draw(vertical, 2, sf::Lines, states);
}

void Engine2D::drawGrid(sf::RenderTarget& target, sf::RenderStates states, const sf::Font& font,
    const sf::Color& grid_color, const sf::Color& text_color, unsigned symbol_size) const
{
    auto ratio = getRatio();
    sf::VertexArray grid(sf::Lines);

    std::vector<sf::Text> labels;

    double ratio_div = ratio.first / ratio.second;
    double x = borders_.bottom - std::fmod(borders_.bottom, ratio_div) + ratio_div * static_cast<double>(borders_.bottom > 0.0);

    const double min_label_pos = ratio_div * 0.01;
    while (x < borders_.top)
    {
        std::stringstream number_text;
        if (std::abs(x) > min_label_pos)
        {
            number_text << x;
        }

        labels.emplace_back(sf::Text(number_text.str(), font, symbol_size));
        labels.back().setFillColor(text_color);
        labels.back().setPosition(sf::Vector2f(getLabelPos(point_t(0.0, x), symbol_size)));

        grid.append(sf::Vertex(sf::Vector2f(Base2Screen(point_t(borders_.left, x))), grid_color));
        grid.append(sf::Vertex(sf::Vector2f(Base2Screen(point_t(borders_.right, x))), grid_color));
        x += ratio_div;
    }

    x = borders_.left - std::fmod(borders_.left, ratio_div) + ratio_div * static_cast<double>(borders_.left > 0.0);

    while (x < borders_.right)
    {
        std::stringstream number_text;
        if (std::abs(x) > min_label_pos)
        {
            number_text << x;
        }

        labels.emplace_back(sf::Text(number_text.str(), font, symbol_size));
        labels.back().setFillColor(text_color);
        labels.back().setPosition(sf::Vector2f(getLabelPos(point_t(x, 0.0), symbol_size)));

        grid.append(sf::Vertex(sf::Vector2f(Base2Screen(point_t(x, borders_.bottom))), grid_color));
        grid.append(sf::Vertex(sf::Vector2f(Base2Screen(point_t(x, borders_.top))), grid_color));
        x += ratio_div;
    }

    target.draw(grid, states);

    for (const auto& label : labels)
    {
        target.draw(label, states);
    }
}

void Engine2D::updateSize(const sf::Vector2u& size)
{
    setSizeInPixels(size);
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
        updateSize(sf::Vector2u(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height));
    }
}

void Engine2D::WheelZooming(double wheel_delta, point_t point)
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

void Engine2D::MouseMoving(const sf::Vector2i& movement)
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

Base2D::point_t Engine2D::getLabelPos(point_t point, unsigned symbol_size) const
{
    point_t new_point(Base2Screen(point));
    new_point = point_t(
        std::clamp(new_point.x, 0.0, static_cast<double>(getSize().x - symbol_size * 4)),
        std::clamp(new_point.y, 0.0, static_cast<double>(getSize().y - symbol_size * 2))
    );
    return new_point;
}

std::pair<double, double> Engine2D::getRatio() const
{
    const double initial_ratio = 10.0;
    static std::pair<double, double> ratio(initial_ratio, initial_ratio);

    double ratio_div = ratio.first / ratio.second;

    auto size = getSize();
    double screen_square = ratio_div * ratio_div * static_cast<double>(size.x * size.y) /
        ((borders_.right - borders_.left) * (borders_.top - borders_.bottom));

    const double max_square = 14400.0;
    const double min_square = 2025.0;

    constexpr double TWO = 2.0;
    constexpr double FIVE = 5.0;
    constexpr double TEN = 10.0;

    if (screen_square > max_square)
    {
        switch (static_cast<int>(ratio.second))
        {
        case static_cast<int>(TWO):
            ratio = std::pair<double, double>(ratio.first, FIVE);
            break;
        case static_cast<int>(FIVE):
            ratio = std::pair<double, double>(ratio.first, TEN);
            break;
        case static_cast<int>(TEN):
            ratio = std::pair<double, double>(ratio.first / TEN, TWO);
            break;
        }
    }
    else if (screen_square < min_square)
    {
        switch (static_cast<int>(ratio.second))
        {
        case static_cast<int>(TWO):
            ratio = std::pair<double, double>(ratio.first * TEN, TEN);
            break;
        case static_cast<int>(FIVE):
            ratio = std::pair<double, double>(ratio.first, TWO);
            break;
        case static_cast<int>(TEN):
            ratio = std::pair<double, double>(ratio.first, FIVE);
            break;
        }
    }
    return ratio;
}