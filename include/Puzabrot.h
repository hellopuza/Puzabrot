#ifndef PUZABROT_H
#define PUZABROT_H

#include "ComplexShader.h"
#include "InputBox.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

namespace puza {

constexpr size_t DEFAULT_WIDTH  = 640;
constexpr size_t DEFAULT_HEIGHT = 480;

constexpr size_t SCREENSHOT_WIDTH = 7680;

constexpr size_t AUDIO_BUFF_SIZE = 4096;
constexpr size_t SAMPLE_RATE     = 48000;
constexpr size_t MAX_FREQ        = 4000;

static const sf::String TITLE_STRING = "Puzabrot";

class Puzabrot final
{
public:
    Puzabrot();
    void run();

    enum ActionModes
    {
        ZOOMING,
        POINT_TRACING,
        SOUNDING,
    };

    enum DrawingModes
    {
        MAIN,
        JULIA,
    };

private:
    void    initCalculator(Calculator& calc, point_t z, point_t c) const;
    void    Mapping(Calculator& calc, ComplexShader::ExprTrees& expr_trees, point_t& mapped_point) const;
    void    updateWinSizes(size_t new_width, size_t new_height);
    void    toggleFullScreen();
    bool    InputBoxesHasFocus();
    bool    InputBoxesIsVisible();
    void    DrawSet();
    int     GetZoomingFrame(Frame& frame);
    point_t PointTrace(point_t point, point_t c_point);
    void    savePicture();
    void    drawHelpMenu();
    int     makeShader();

    ComplexHolder    holder_;
    ComplexShader    shader_;
    sf::RenderWindow window_;
    sf::Font font_;

    struct Options final
    {
        int  input_mode = ComplexShader::Z_INPUT;
        int  draw_mode  = MAIN;
        bool coloring   = false;
    } options_;

    struct InputBoxes
    {
        InputBox x;
        InputBox y;
        InputBox z;
    } input_boxes_;

    ComplexShader::ExprTrees expr_trees_;

    class Synth final : public sf::SoundStream
    {
    public:
        Synth(Puzabrot* app);

        virtual void onSeek(sf::Time) override {}
        virtual bool onGetData(Chunk& data) override;

        void SetPoint(point_t point);
        void copyTrees(ComplexShader::ExprTrees& expr_trees);

        bool   audio_reset_;
        bool   audio_pause_;
        bool   sustain_;
        double volume_;

    private:
        Puzabrot* app_;
        ComplexShader::ExprTrees expr_trees_;

        point_t point_;
        point_t c_point_;
        point_t new_point_;
        point_t prev_point_;

        int16_t m_samples[AUDIO_BUFF_SIZE] = {};
        int32_t m_audio_time               = 0;

        double mean_x = 0;
        double mean_y = 0;

        double dx  = 0;
        double dy  = 0;
        double dpx = 0;
        double dpy = 0;
    } synth_;
};

} // namespace puza

#endif // PUZABROT_H