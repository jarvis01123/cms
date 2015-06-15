#include "trading.h"

bool trading::order::operator==(const order& rhs) const {
    return this->id == rhs.id;
}

std::string trading::order::info_str() {
    std::stringstream message;

    message << id << " ";
    message << dealer_id << " ";
    message << trading::stringify(side) << " ";
    message << commodity << " ";
    message << amount << " ";
    message << price;

    return message.str();
}


std::string trading::stringify(trading::side side) {
    if (side == trading::side::BUY) {
        return "BUY";
    } else if (side == trading::side::SELL) {
        return "SELL";
    } else {
        return "ERROR";
    }
}

trading::side trading::sideify(std::string s) {
    if (s == "BUY") {
        return trading::side::BUY;
    } else if (s == "SELL") {
        return trading::side::SELL;
    } else {
        return trading::side::ERROR;
    }
}
