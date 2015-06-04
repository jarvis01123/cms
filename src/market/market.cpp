#include "market.h"
#include "../misc/misc.h"

trading::market::market(): _next_id{0} {
    _handlers.emplace( std::make_pair("POST", &trading::market::post) );
    _handlers.emplace( std::make_pair("LIST", &trading::market::list) );
    _handlers.emplace( std::make_pair("CHECK", &trading::market::check) );
    _handlers.emplace( std::make_pair("AGGRESS", &trading::market::aggress) );
    _handlers.emplace( std::make_pair("REVOKE", &trading::market::revoke) );
}


// execute a raw input std::string
std::string trading::market::execute(std::string message) {
    auto command = misc::split_on_white(message);

    auto res = validate(command);

    if (!res.first) return res.second;

    handler x = _handlers[command[1]];
    return (this->*x)(command);
}

std::vector<std::string> trading::market::get_commands() {
    return COMMANDS;
}

std::pair<bool, std::string> trading::market::validate(std::vector<std::string> command) {
    std::pair<bool, std::string> tbr;
    tbr.first = true;
    tbr.second = "default";

    if (!(tbr.first = is_valid_dealer(command[0]))) {
        tbr.second = unknown_dealer_err_msg();

    } else if (!(tbr.first = is_valid_command(command[1]))) {
        tbr.second = invalid_err_msg();
    }

    return tbr;
}

int trading::market::next_unique_id() {
    return ++_next_id;
}


// ############### methods ######################

trading::order trading::market::get_order(int order_id) {
    return _orders[order_id];
}

// “POST” side COMMODITY AMOUNT PRICE
std::string trading::market::post(std::vector<std::string> command) {

    order order;
    order.dealer_id = command[0];
    order.id = next_unique_id();
    order.side = trading::sideify(command[2]);
    order.commodity = command[3];
    order.amount = (int)atoi(command[4].c_str());
    order.price = stof(command[5]);

    add_order(order);

    return post_msg(order);
}

void trading::market::add_order(trading::order order) {
    static std::mutex mt;

    // enforce mutual exclusive access to market modify
    std::lock_guard<std::mutex> lk(mt);

    _orders[order.id] = order;
    _commodities[order.commodity].push_back(order);
}

// “LIST” [ COMMODITY [ DEALER_ID ] ]
std::string trading::market::list(std::vector<std::string> command) {

    std::string commodity = command[2];

    if (command.size() == 4) {
        return list_for_dealer(commodity, command[0]);
    }

    if (!is_valid_commodity(commodity)) {
        return unknown_comm_err_msg();
    }

    std::stringstream message;
    for (auto ord : _commodities[commodity]) {
        if (ord.id != 0) {
            message << ord.info_str() << std::endl;
        }
    }

    message << eol_msg();

    return message.str();

}

std::string trading::market::list_for_dealer(std::string commodity, std::string dealer_id) {

    if (!is_valid_commodity(commodity)) {
        return unknown_comm_err_msg();
    }

    std::stringstream message;
    for (auto ord : _commodities[commodity]) {

        if (ord.id != 0 && ord.dealer_id == dealer_id) {
            message << ord.info_str() << std::endl;
        }
    }

    return misc::strip_trailing_newline(message.str());

}

// “AGGRESS” ( order_ID AMOUNT )+
std::string trading::market::aggress(std::vector<std::string> command) {

    std::stringstream message;

    int order_id, order_amount;

    if ((command.size() % 2) != 0) {
        return invalid_err_msg();
    }

    for (size_t i = 2; i < command.size(); i += 2) {

        order_id = (int)atoi(command[i].c_str());
        order_amount = (int)atoi(command[i + 1].c_str());

        auto order = get_order(order_id);

        if (order.id == 0) {
            message << unknown_ord_err_msg() << std::endl;

        } else {

            if (order.amount >= order_amount) {

                auto new_amt = adjust_amount(order_id, (-1)*order_amount);

                message << trade_report(command[0], order, order_amount) << std::endl;

                if (new_amt == 0) {
                    message << filled_msg(order_id) << std::endl;
                    close_order(order_id);
                }

            } else {
                message << unauthorized_err_msg() << std::endl;
            }
        }

    }

    return misc::strip_trailing_newline(message.str());

}


// “REVOKE” order_ID
std::string trading::market::revoke(std::vector<std::string> command) {

    auto order_id = (int)atoi(command[2].c_str());

    close_order(order_id);

    return revoke_msg(order_id);
}

void trading::market::close_order(int order_id) {

    auto order = get_order(order_id);
    auto comms = &_commodities[order.commodity];
    auto it = find(comms->begin(), comms->end(), order);

    static std::mutex mt;

    // enforce mutual exclusive access to market write
    std::lock_guard<std::mutex> lk(mt);

    _orders.erase(order_id);
    comms->erase(it);

}

int trading::market::adjust_amount(int order_id, int amount) {

    auto order = get_order(order_id);
    int new_amount = order.amount + amount;
    auto comms = &_commodities[order.commodity];
    auto it = find(comms->begin(), comms->end(), order);

    static std::mutex mt;

    // enforce mutual exclusive access to market write
    std::lock_guard<std::mutex> lk(mt);

    _orders[order_id].amount += amount;
    it->amount += amount;

    return new_amount;
}

// “CHECK” order_ID
std::string trading::market::check(std::vector<std::string> command) {
    int order_id = (int)atoi(command[2].c_str());
    auto order = get_order(order_id);
    return order.info_str();

}


    // ### helper methods ###

bool trading::market::is_valid_dealer(std::string id) {
    return find(DEALER_IDS.begin(), DEALER_IDS.end(), id) != DEALER_IDS.end();
}

bool trading::market::is_valid_commodity(std::string id) {
    return find(COMMODITIES.begin(), COMMODITIES.end(), id) != COMMODITIES.end();
}

bool trading::market::is_valid_command(std::string id) {
    return find(COMMANDS.begin(), COMMANDS.end(), id) != COMMANDS.end();
}

inline std::string trading::market::unknown_comm_err_msg() { return "UNKNOWN_COMMODITY"; }
inline std::string trading::market::unknown_dealer_err_msg() { return "UNKNOWN_DEALER"; }
inline std::string trading::market::invalid_err_msg() { return "INVALID"; }
inline std::string trading::market::unknown_ord_err_msg() { return "UNKOWN_ORDER"; }
inline std::string trading::market::unauthorized_err_msg() { return "UNAUTHORIZED"; }
inline std::string trading::market::eol_msg() { return "END_OF_LIST"; }

inline std::string trading::market::filled_msg(int order_id) {
    std::stringstream message;
    message << order_id << " HAS BEEN FILLED";
    return message.str();
}

inline std::string trading::market::trade_report(std::string dealer_id, order order, int order_amount) {
    std::stringstream message;

    message << dealer_id << " ";
    message << (order.side == trading::side::SELL ? "BOUGHT " : "SOLD ");
    message << order_amount << " @ ";
    message << order.price;

    return message.str();
}

inline std::string trading::market::post_msg(order order) {
    std::stringstream message;

    message << order.info_str();
    message << " HAS BEEN POSTED";
    return message.str();
}

inline std::string trading::market::revoke_msg(int order_id) {
    std::stringstream message;
    message << order_id << " HAS BEEN REVOKED";
    return message.str();
}
