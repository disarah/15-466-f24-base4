#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

struct Script {
	Script();
	
    virtual ~Script();

    int font_size;
    unsigned int width;
    unsigned int height;

    virtual std::string get_next_line(u_int32_t action);
};