/*------------------------------------------------------------------------------
    * File:        Puzabrot.cpp                                                *
    * Description: Functions for application                                   *
    * Created:     27 mar 2021                                                 *
    * Author:      Artem Puzankov                                              *
    * Email:       puzankov.ao@phystech.edu                                    *
    * GitHub:      https://github.com/hellopuza                                *
    * Copyright © 2021 Artem Puzankov. All rights reserved.                    *
    *///------------------------------------------------------------------------

#include "Puzabrot.h"

//------------------------------------------------------------------------------

Puzabrot::Puzabrot () :
    winsizes_ ({DEFAULT_WIDTH, DEFAULT_HEIGHT})
{
    pointmap_ = new sf::VertexArray(sf::Points, winsizes_.x * winsizes_.y);
    window_   = new sf::RenderWindow(sf::VideoMode(winsizes_.x, winsizes_.y), title_string);
    
    borders_.Im_up   =  1.3;
    borders_.Im_down = -1.3;

    borders_.Re_left  = -(borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *3;
    borders_.Re_right =  (borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y / 5 *2;
}

//------------------------------------------------------------------------------

Puzabrot::~Puzabrot ()
{
    if (window_->isOpen()) window_->close();
    delete pointmap_;
    delete window_;
}

//------------------------------------------------------------------------------

void Puzabrot::run ()
{
    window_->setVerticalSyncEnabled(true);

    InputBox input_box(sf::Vector2f(10, 10), sf::Color(128, 128, 128, 128), sf::Color::White, 20);
    input_box.setInput(sf::String("z^2+c"));

    DrawSet();
    window_->display();

    while (window_->isOpen())
    {
        sf::Event event;
        while (window_->pollEvent(event))
        {
            if ( ( event.type == sf::Event::Closed) ||
                 ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)) )
            {
                window_->close();
                return;
            }

            if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::F11))
            {
                toggleFullScreen();
            }

            if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window_->setView(sf::View(visibleArea));
                updateWinSizes(window_->getSize().x, window_->getSize().y);
            }

            if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Backslash))
            {
                input_box.is_visible_ = 1 - input_box.is_visible_;
                if (not input_box.is_visible_)
                    input_box.has_focus_ = false;
            }

            if (input_box.is_visible_ && (event.type == sf::Event::MouseButtonPressed) && (event.mouseButton.button == sf::Mouse::Left))
            {
                if ((input_box.getPos().x < event.mouseButton.x) && (event.mouseButton.x < input_box.getPos().x + input_box.getSize().x) &&
                    (input_box.getPos().y < event.mouseButton.y) && (event.mouseButton.y < input_box.getPos().y + input_box.getSize().y))
                    input_box.has_focus_ = true;
                else
                    input_box.has_focus_ = false;
            }

            if (input_box.has_focus_ && (event.type == sf::Event::TextEntered) && (event.text.unicode < 128))
            {
                input_box.setInput(event.text.unicode);
            }
        }

        window_->draw(*pointmap_);
        if (input_box.is_visible_)
            input_box.draw(window_);
        else
            window_->display();
    }
}

//------------------------------------------------------------------------------

void Puzabrot::updateWinSizes (size_t width, size_t height)
{
    winsizes_.x = width;
    winsizes_.y = height;

    delete pointmap_;
    pointmap_ = new sf::VertexArray(sf::Points, winsizes_.x * winsizes_.y);

    borders_.Re_right = borders_.Re_left + (borders_.Im_up - borders_.Im_down) * winsizes_.x/winsizes_.y;

    window_->draw(*pointmap_);
    window_->display();
}

//------------------------------------------------------------------------------

void Puzabrot::toggleFullScreen ()
{
    if (window_->isOpen()) window_->close();
    delete window_;

    if ((winsizes_.x == sf::VideoMode::getDesktopMode().width) && (winsizes_.y == sf::VideoMode::getDesktopMode().height))
    {
        window_ = new sf::RenderWindow(sf::VideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT), title_string);
        updateWinSizes(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    }
    else
    {
        window_ = new sf::RenderWindow(sf::VideoMode::getDesktopMode(), title_string, sf::Style::Fullscreen);
        updateWinSizes(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
    }
}

//------------------------------------------------------------------------------

void Puzabrot::DrawSet ()
{
    assert(itrn_max_);
    assert(lim_);

    int width  = winsizes_.x;
    int height = winsizes_.y;

    double re_step = (borders_.Re_right - borders_.Re_left) / width;
    double im_step = (borders_.Im_up    - borders_.Im_down) / height;

    __m256d _m_lim = _mm256_set1_pd(lim_);
    __m128i ones   = _mm_set1_epi32(1);
    __m128i zeros  = _mm_set1_epi32(0);

    __m128i mask32_128_1 = _mm_setr_epi32( 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF );
    __m128i mask32_128_2 = _mm_setr_epi32( 0xFFFFFFFF, 0, 0xFFFFFFFF, 0xFFFFFFFF );
    __m128i mask32_128_3 = _mm_setr_epi32( 0xFFFFFFFF, 0xFFFFFFFF, 0, 0xFFFFFFFF );
    __m128i mask32_128_4 = _mm_setr_epi32( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0 );


    double im0 = borders_.Im_down;

    int x_offset = 0;

    for (int y = 0; y < height; (++y, im0 += im_step, x_offset += width))
    {
        __m256d _m_im0 = { im0, im0, im0, im0 };

        #pragma omp parallel for
        for (int x = 0; x < width; x += 4)
        {
            double re0 = x * re_step + borders_.Re_left;

            __m256d _m_re0 = { re0, re0 + re_step, re0 + 2*re_step, re0 + 3*re_step };

            __m256d _m_re2 = _mm256_mul_pd(_m_re0, _m_re0);
            __m256d _m_im2 = _mm256_mul_pd(_m_im0, _m_im0);

            __m256d _m_s = _mm256_mul_pd(_mm256_add_pd(_m_re0, _m_im0), _mm256_add_pd(_m_re0, _m_im0));

            __m128i iterations = _mm_set1_epi32(0);
            __m128i iter_mask  = _mm_set1_epi32(0xFFFFFFFF);

            for (int i = 0; i < itrn_max_; ++i)
            {
                __m256d _m_re1 = _mm256_add_pd(_mm256_sub_pd(_m_re2, _m_im2), _m_re0);
                __m256d _m_im1 = _mm256_add_pd(_mm256_sub_pd(_mm256_sub_pd(_m_s, _m_re2), _m_im2), _m_im0);
                _m_re2 = _mm256_mul_pd(_m_re1, _m_re1);
                _m_im2 = _mm256_mul_pd(_m_im1, _m_im1);

                __m256d _m_rad2 = _mm256_add_pd(_m_re2, _m_im2);

                __m128i rad_cmp = _mm_add_epi32(_mm256_cvtpd_epi32(_mm256_cmp_pd(_m_rad2, _m_lim, _CMP_GT_OS)), ones);
                rad_cmp = _mm_abs_epi32(_mm_cmpgt_epi32(rad_cmp, zeros));

                iterations = _mm_add_epi32(iterations, _mm_and_si128(iter_mask, rad_cmp));

                if (*((int32_t*)&rad_cmp + 0) == 0) iter_mask = _mm_and_si128(iter_mask, mask32_128_1);
                if (*((int32_t*)&rad_cmp + 1) == 0) iter_mask = _mm_and_si128(iter_mask, mask32_128_2);
                if (*((int32_t*)&rad_cmp + 2) == 0) iter_mask = _mm_and_si128(iter_mask, mask32_128_3);
                if (*((int32_t*)&rad_cmp + 3) == 0) iter_mask = _mm_and_si128(iter_mask, mask32_128_4);

                if (_mm_test_all_zeros(iter_mask, iter_mask)) break;

                _m_s  = _mm256_mul_pd(_mm256_add_pd(_m_re1, _m_im1), _mm256_add_pd(_m_re1, _m_im1));
            }

            for (int i = 0; i < 4; ++i)
            {
                (*pointmap_)[x_offset + x + i].position = sf::Vector2f(x + i, y);
                (*pointmap_)[x_offset + x + i].color = getColor(*((int32_t*)&iterations + i));
            }
        }
    }

    window_->draw(*pointmap_);
}

//------------------------------------------------------------------------------

sf::Color Puzabrot::getColor (int32_t itrn)
{
    if (itrn < itrn_max_)
    {
        itrn = itrn*4 % 1530;

             if (itrn < 256 ) return sf::Color( 255,       itrn,      0         );
        else if (itrn < 511 ) return sf::Color( 510-itrn,  255,       0         );
        else if (itrn < 766 ) return sf::Color( 0,         255,       itrn-510  );
        else if (itrn < 1021) return sf::Color( 0,         1020-itrn, 255       );
        else if (itrn < 1276) return sf::Color( itrn-1020, 0,         255       );
        else if (itrn < 1530) return sf::Color( 255,       0,         1529-itrn );
    }

    return sf::Color( 0, 0, 0 );
}

//------------------------------------------------------------------------------
