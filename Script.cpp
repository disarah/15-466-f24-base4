#include "Script.hpp"

std::string Page::get_text(){
    return newline + text + newline + "[R]" + option1 + newline + "[G]" +option2 + newline + "[B]" +option3;
}

Script::Script() {
    pages.push_back(Page(
            "Tired of your life back in the forest, you decide to try your luck in the big city. Luckily for you there was a discount graydog ticket. Now you're in the Big Apple.",
            "Take a look around Time Square",
            "Walk to the Subway",
            "Eat a food"
        ));

    current_index = 0;
}

Script::~Script() {
}

std::string Script::get_next_line(u_int32_t action){
    return pages[current_index].get_text();
}