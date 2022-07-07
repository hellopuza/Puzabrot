#include "ShaderEngine.h"

ShaderEngine::ShaderEngine(const sf::Vector2u & image_size)
{
    render_texture_.create(image_size.x, image_size.y);
    sprite_ = sf::Sprite(render_texture_.getTexture());
}

void ShaderEngine::setImageSize(const sf::Vector2u& size)
{
    render_texture_.create(size.x, size.y);
    sprite_ = sf::Sprite(render_texture_.getTexture());
}

sf::Sprite ShaderEngine::getOutput() const
{
    return sprite_;
}