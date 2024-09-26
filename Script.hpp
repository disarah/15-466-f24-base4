#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

struct Page {
    std::string text;
    std::string option1;
    std::string option2;
    std::string option3;

    std::string newline= "                                                                                ";

    Page(std::string t, std::string o1, std::string o2, std::string o3)
    {
      text = t;
      option1 = o1;
      option2 = o2;
      option3 = o3;
    }

    std::string get_text();
};

struct Script {
	Script();
	
    virtual ~Script();

    unsigned int current_index;

    std::vector< Page > pages;

    virtual std::string get_next_line(u_int32_t action);
};