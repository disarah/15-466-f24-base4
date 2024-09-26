#pragma once

#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <map>

struct Locations {
  std::vector< std::string > timesSquare;
  std::vector< std::string > shopping;
  std::vector< std::string > trash;
  std::vector< std::string > endings;

  std::vector<int> visits{0, 0, 0};
};

struct Page {
    std::string option1;
    std::string option2;
    std::string option3;

    std::string newline= "                                                 ";

    std::string halfline = "                        ";

    Page(std::string o1, std::string o2, std::string o3)
    {
      option1 = o1;
      option2 = o2;
      option3 = o3;
    }

    std::string get_text(std::string body, unsigned int current_time);
};

struct Script {
	Script();
	
    virtual ~Script();

    unsigned int timeOfDay;

    Locations places;

    std::vector< Page > pages;

    virtual std::string get_next_line(u_int32_t action);
};