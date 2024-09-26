#include "Script.hpp"

#include "gl_errors.hpp"

std::string Page::get_text(std::string body, unsigned int current_time){
    return newline + body + " It's " + std::to_string(current_time) + ":00." + halfline + "[R] " + option1 + halfline + "[G] " + option2 + halfline + "[B] " +option3;
}

Script::Script() {
    pages.push_back(Page(
        "Look around Time Square?",
        "Go shopping!",
        "Dig around in the trash..."
    ));
    pages.push_back(Page(
        "Try again?",
        "Try again?",
        "Try again?"
    ));

    places.timesSquare.insert(places.timesSquare.end(), {
        "Your eyes hurt a little from staring at the neon signs.",
        "You haven't blinked for so long your eyes become numb. But the lights are so pretty!",
        "Color leaves your vision. Things are starting to look a little fuzzy...",
        "You blinked and your vision returns. Goodie me! Tourists start taking pictures of you.",
        "You charge 3 dollars per pawprint and 5 dollars per photo. You are rolling in cash.",
        "Tourists follow you everywhere. You feel a little less alone.",
    });

    places.shopping.insert(places.shopping.end(), {
        "You are broke. You get kicked out of all the stores you enter.",
        "You meet an old lady while waiting around the Tiffany's store.",
        "You spend more time chatting with the lady. Her name is Cathy. She's nice!",
        "Cathy lets you know that her grandson is stranded on a deserted island with a Nigerian prince and they 10000 dollars sent through Western Union. Oh No!",
        "You tell Cathy about common scams. She thanks you for your companionship. She gives you a business card.",
        "You hand the business card to the people working at Hermes. They faint on the spot. Oh No!",
    });

    places.trash.insert(places.trash.end(), {
        "You find an apple! How ironic, huh? Finding an apple in the Big Apple.",
        "You find a very fashionable scarf. Wearing this around gets you many compliments!",
        "You find nothing. You're just digging around in the trash.",
        "You find nothing. You're just digging around in the trash.",
        "You find nothing. You're just digging around in the trash.",
        "Oh My God. You just found a Banksy painting! You sell it for millions.",
    });

    places.endings.insert(places.endings.end(), {
        "You frequent Time Square so much you become a local legend. People call you the Times Square Raccoon. They start leaving offerings to you. Maybe you could use this to your advantage.",
        "You're a shopaholic. Local shoppers start taking pity on you. They're reminded of their humble beginnings. They start giving you gifts. You've broken into the New York elite.",
        "You reconnect with your roots in the trash. You feel at peace. This is the best vacation ever!"
    });

    timeOfDay = 0;
}

Script::~Script() {
}

std::string Script::get_next_line(u_int32_t action){
    std::string bodyText;
    if (timeOfDay == 3){
        bodyText = "You take an overnight greyhound bus and now you're in the Big Apple. Carpe Diem!";
    } else if(action == 0){
        bodyText = places.timesSquare[places.visits[action]];
        places.visits[action]+=1;
    } else if (action == 1){
        bodyText = places.shopping[places.visits[action]];
        places.visits[action]+=1;
    } else {
        bodyText = places.trash[places.visits[action]];
        places.visits[action]+=1;
    }

    timeOfDay +=3;
    if(timeOfDay >= 24){
        timeOfDay = 0;
        bodyText = places.endings[std::distance(places.visits.begin(), std::max_element(places.visits.begin(), places.visits.end()))];
        places.visits[0] = 0;
        places.visits[1] = 0;
        places.visits[2] = 0;
        return pages[1].get_text(bodyText, timeOfDay);
    }
    return pages[0].get_text(bodyText, timeOfDay);
}