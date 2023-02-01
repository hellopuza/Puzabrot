#include "Application/ShaderApplication.h"

ShaderApplication::ShaderApplication(const vec2i& win_size, const char* font_location, double font_size, const char* win_title) :
    Application(win_size, font_location, font_size, win_title)
{
    setRenderImageSize(win_size);
}

void ShaderApplication::setRenderImageSize(const vec2i& size)
{
    render_texture_.create(size.x, size.y);
    sprite_ = sf::Sprite(render_texture_.getTexture());
}

sf::Sprite ShaderApplication::getRenderOutput() const
{
    return sprite_;
}