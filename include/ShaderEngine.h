#ifndef SHADERENGINE_H
#define SHADERENGINE_H

#include <SFML/Graphics.hpp>

class ShaderEngine
{
public:
    ShaderEngine(const sf::Vector2u& image_size);
    virtual ~ShaderEngine() = default;

    void setImageSize(const sf::Vector2u& size);
    sf::Sprite getOutput() const;

protected:
    sf::Shader shader_;
    sf::Sprite sprite_;
    sf::RenderTexture render_texture_;
};

#endif // SHADERENGINE_H