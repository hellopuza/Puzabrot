#ifndef APPLICATION_SHADERAPPLICATION_H
#define APPLICATION_SHADERAPPLICATION_H

#include "Application/Application.h"

class ShaderApplication : public Application
{
public:
    ShaderApplication(const vec2i& win_size, const char* font_location, double font_size, const char* win_title = "");

    void setRenderImageSize(const vec2u& size);
    sf::Sprite getRenderOutput() const;

protected:
    sf::Shader shader_;
    sf::Sprite sprite_;
    sf::RenderTexture render_texture_;
};

#endif // APPLICATION_SHADERAPPLICATION_H