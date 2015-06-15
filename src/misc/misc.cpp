#include "misc.h"
#include <sstream>

// split space delimited string into vector of strings
std::vector<std::string> misc::split_on_white(std::string s) {

    std::vector<std::string> tbr;
    std::string temp;
    std::stringstream ss(s);

    while (ss >> temp) {
        tbr.push_back(temp);
    }

    return tbr;
}

// hacky method, strip trailing newline from a string.
// wouldn't work if there were two (\n\n).
std::string misc::strip_trailing_newline(std::string str) {
    std::string tbr = str;
    auto last = tbr.find_last_not_of('\n');
    if (last == tbr.length()-2) {
        tbr.erase(last + 1);
    }

    return tbr;
}
