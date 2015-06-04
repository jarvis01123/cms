#include "misc.h"

std::vector<std::string> misc::split_on_white(std::string s) {

    std::vector<std::string> tbr;
    std::string temp;
    while (s.length() > 0) {
        temp = s.substr(0, s.find(' '));
        s.erase(0, temp.length() + 1);
        tbr.push_back(temp);
    }

    return tbr;
}

std::string misc::strip_trailing_newline(std::string str) {
    std::string tbr = str;
    auto last = tbr.find_last_not_of('\n');
    if (last == tbr.length()-2) {
        tbr.erase(last + 1);
    }

    return tbr;
}
