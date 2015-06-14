#include "market.h"
#include "../misc/misc.h"

trading::market::market(): _next_id{0} {
    // register handlers
    _handlers.emplace( std::make_pair("POST", &trading::market::post) );
    _handlers.emplace( std::make_pair("LIST", &trading::market::list) );
    _handlers.emplace( std::make_pair("CHECK", &trading::market::check) );
    _handlers.emplace( std::make_pair("AGGRESS", &trading::market::aggress) );
    _handlers.emplace( std::make_pair("REVOKE", &trading::market::revoke) );
}

std::string trading::market::execute(std::string message) {
    std::vector<std::string> command = misc::split_on_white(message);
    std::pair<bool, std::string> res = validate(command);

    if (!res.first) return res.second;

    handler x = _handlers[command[1]];
    return (this->*x)(command);
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

std::pair<bool, int> trading::market::parse_int(std::string val) {

    bool is_int = true;
    for (auto x : val) {
        if (!isdigit(x)) {
            is_int = false;
            break;
        }
    }

    return std::make_pair(is_int, atoi(val.c_str()));
}

std::pair<bool, float> trading::market::parse_float(std::string val) {

    // returns 0 if not a float. this is fine to test for, since 0 is a
    char* end;
    float tbr = strtof(val.c_str(), &end);

    // strtof sets end to str in case of conversion failure
    return std::make_pair(val.c_str() != end, tbr);
}

// “POST” side COMMODITY AMOUNT PRICE
std::string trading::market::post(std::vector<std::string> command) {

    if (command.size() != 6) return invalid_err_msg();

    order order;
    order.dealer_id = command[0];
    order.id = next_unique_id();
    order.side = trading::sideify(command[2]);
    order.commodity = command[3];
    std::pair<bool, int> t1 = parse_int(command[4]);
    std::pair<bool, float> t2 = parse_float(command[5]);

    if (!t1.first || !t2.first) return invalid_err_msg();

    order.amount = t1.second;
    order.price = t2.second;

    add_order(order);

    return post_msg(order);
}

void trading::market::add_order(trading::order order) {
    _orders[order.id] = order;
    _commodities[order.commodity].push_back(order);
}

// “LIST” [ COMMODITY [ DEALER_ID ] ]
std::string trading::market::list(std::vector<std::string> command) {

    std::string commodity = command[2];

    if (command.size() == 4) {
        return list_for_dealer(commodity, command[3]);
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

    message << eol_msg();
    return message.str();
}

// “AGGRESS” ( order_ID AMOUNT )+
std::string trading::market::aggress(std::vector<std::string> command) {

    std::stringstream message;

    // b.c. aggress orders come in pairs
    if ((command.size() % 2) != 0) {
        return invalid_err_msg();
    }

    for (size_t i = 2; i < command.size(); i += 2) {

        std::pair<bool, int> t1 = parse_int(command[i]);
        std::pair<bool, int> t2 = parse_int(command[i + 1]);

        if (!t1.first || !t2.first) return invalid_err_msg();

        auto order_id = t1.second;
        auto order_amount = t2.second;

        if (!is_known_order(order_id)) {
            message << unknown_ord_err_msg() << std::endl;
        } else {
            auto order = get_order(order_id);
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

    auto order_id = parse_int(command[2].c_str());
    if (!order_id.first) return invalid_err_msg();
    if (!is_known_order(order_id.second)) return unknown_ord_err_msg();

    close_order(order_id.second);
    return revoke_msg(order_id.second);
}

void trading::market::close_order(int order_id) {

    auto order = get_order(order_id);
    auto comms = &_commodities[order.commodity];
    auto it = find(comms->begin(), comms->end(), order);

    _orders.erase(order_id);
    comms->erase(it);

}

int trading::market::adjust_amount(int order_id, int amount) {

    auto order = get_order(order_id);
    int new_amount = order.amount + amount;
    auto comms = &_commodities[order.commodity];
    auto it = find(comms->begin(), comms->end(), order);

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

bool trading::market::is_known_order(int order_id) {
    auto it(_orders.lower_bound(order_id));
    return (it != _orders.end());
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
