// Code modified from Qiru Hu's Font.hpp
#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

#include <hb.h>
#include <hb-ft.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

struct Text {
    std::string text;
    glm::vec2 start_pos;

    int line_length;
    int line_height;
};

struct Font {
	Font(std::string const &font_path,
        int font_size, 
        unsigned int width, 
        unsigned int height
		);
	
    virtual ~Font();

    int font_size;
    unsigned int width;
    unsigned int height;

    glm::u8vec3 color = glm::u8vec3(255);

    FT_Library ft_library;
    FT_Face ft_face;
    hb_font_t *hb_font;
    hb_buffer_t *hb_buffer;

    virtual void gen_texture(unsigned int& texture, std::vector<Text> const &texts);

    virtual std::vector<std::string> wrapText(const std::string& text, size_t line_length);
};
