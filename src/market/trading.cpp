#include "trading.h"

bool trading::order::operator==(const order& rhs) const {
    return this->id == rhs.id;
}

std::string trading::order::info_str() {
    std::stringstream message;

    message << id << " ";
    message << dealer_id << " ";
    message << amount << " ";
    message << trading::stringify(side) << " ";
    message << commodity << " ";
    message << price;

    return message.str();
}


std::string trading::stringify(trading::side side) {
    return side == trading::side::BUY ? "BUY" : "SELL";
}

trading::side trading::sideify(std::string s) {
    if (s == "BUY") {
        return trading::side::BUY;
    } else {
        return trading::side::SELL;
    }
}
