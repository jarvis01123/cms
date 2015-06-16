#include "market.h"
#include "../misc/misc.h"

using namespace trading::constants;
using namespace trading::constants::positions;

using money_type = trading::money_type;
using id_type = trading::id_type;

trading::market::market(): _next_id{0} {
    // register handlers
    _handlers.emplace(std::make_pair("POST", &trading::market::post));
    _handlers.emplace(std::make_pair("LIST", &trading::market::list));
    _handlers.emplace(std::make_pair("CHECK", &trading::market::check));
    _handlers.emplace(std::make_pair("AGGRESS", &trading::market::aggress));
    _handlers.emplace(std::make_pair("REVOKE", &trading::market::revoke));
}

std::string trading::market::execute(std::string message) {
    std::vector<std::string> command = misc::split_on_white(message);
    std::pair<bool, std::string> res = validate(command);

    if (!res.first) return res.second;

    handler x = _handlers[command[COMMAND]];
    return (this->*x)(command);
}

std::pair<bool, std::string> trading::market::validate(std::vector<std::string> command) {
    std::pair<bool, std::string> tbr;
    tbr.first = true;
    tbr.second = "default";

    if (!(tbr.first = is_valid_dealer(command[DEALER]))) {
        tbr.second = unknown_dealer_err_msg();
    } else if (!(tbr.first = is_valid_command(command[COMMAND]))) {
        tbr.second = invalid_err_msg();
    }

    return tbr;
}

id_type trading::market::next_unique_id() {
    return ++_next_id;
}

trading::order trading::market::get_order(id_type order_id) {
    return _orders[order_id];
}

std::pair<bool, id_type> trading::market::parse_id_type(std::string val) {

    bool is_id_type = true;
    for (auto x : val) {
        if (!isdigit(x)) {
            is_id_type = false;
            break;
        }
    }

    return std::make_pair(is_id_type, atoi(val.c_str()));
}

std::pair<bool, money_type> trading::market::parse_money_type(std::string val) {

    char* end;
    money_type tbr = strtod(val.c_str(), &end);

    // strtof sets end to str in case of conversion failure
    return std::make_pair(val.c_str() != end, tbr);
}

// “POST” side COMMODITY AMOUNT PRICE
std::string trading::market::post(std::vector<std::string> command) {

    if (command.size() != POST_NUM_ARGS) return invalid_err_msg();

    trading::order order;
    order.dealer_id = command[DEALER];
    order.side = trading::sideify(command[POST_SIDE]);
    order.commodity = command[POST_COMMODITY];
    std::pair<bool, id_type> t1 = parse_id_type(command[POST_AMOUNT]);
    std::pair<bool, money_type> t2 = parse_money_type(command[POST_PRICE]);
    order.amount = t1.second;
    order.price = t2.second;
    order.status = trading::order_status::ACTIVE;

    if (!is_valid_commodity(order.commodity)) {
        return unknown_comm_err_msg();
    } else if ((!t1.first || !t2.first) ||
               (order.side == trading::side::ERROR)) {

        return invalid_err_msg();
    }

    order.id = next_unique_id();
    add_order(order);

    return post_msg(order);
}

void trading::market::add_order(trading::order order) {
    _orders[order.id] = order;
    _commodities[order.commodity].push_back(order);
}

// “LIST” COMMODITY [ DEALER_ID ]
std::string trading::market::list(std::vector<std::string> command) {

    if (command.size() > LIST_NUM_ARGS) {
        std::string commodity = command[LIST_COMMODITY];

        if (command.size() == LIST_COMM_NUM_ARGS) {
            return list_for_commodity(commodity);
        } else if (command.size() == LIST_DEALER_NUM_ARGS) {
            return list_for_dealer(commodity, command[LIST_DEALER]);
        } else if (!is_valid_commodity(commodity)) {
            return unknown_comm_err_msg();
        }
    }

    std::stringstream message;
    for (auto comm : _commodities) {
        for (auto ord : comm.second) {
            if (ord.id != EMPTY_ORDER) {
                message << ord.info_str() << std::endl;
            }
        }
    }

    message << eol_msg();

    return message.str();

}

std::string trading::market::list_for_commodity(std::string commodity) {
    std::stringstream message;

    for (auto ord : _commodities[commodity]) {
        if (ord.id != EMPTY_ORDER) {
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

        if (ord.id != EMPTY_ORDER && ord.dealer_id == dealer_id) {
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

        std::pair<bool, id_type> t1 = parse_id_type(command[i]);
        std::pair<bool, id_type> t2 = parse_id_type(command[i + 1]);

        if (!t1.first || !t2.first) return invalid_err_msg();

        auto order_id = t1.second;
        auto order_amount = t2.second;

        if (!is_known_order(order_id)) {
            message << unknown_ord_err_msg() << std::endl;
        } else {
            auto order = get_order(order_id);
            if (order.amount >= order_amount) {

                adjust_amount(order_id, (-1)*order_amount);
                message << trade_report(command[DEALER], order, order_amount) << std::endl;

            } else {
                message << unauthorized_err_msg() << std::endl;
            }
        }
    }

    return misc::strip_trailing_newline(message.str());
}


// “REVOKE” order_ID
std::string trading::market::revoke(std::vector<std::string> command) {

    auto order_id = parse_id_type(command[REVOKE_ORDER].c_str());

    if (!order_id.first) return invalid_err_msg();

    if (!is_known_order(order_id.second) || is_revoked(order_id.second)) {
        return unknown_ord_err_msg();
    }

    if (is_filled(order_id.second)) return filled_msg(order_id.second);

    if (get_order(order_id.second).dealer_id != command[DEALER]) {
        return unauthorized_err_msg();
    }

    revoke_order(order_id.second);

    return revoke_msg(order_id.second);
}

// remove order from active market, leave in internal records
// can blow up, check for valid order elsewhere
void trading::market::close_order(id_type order_id) {
    auto order = get_order(order_id);

    if (is_active(order.id)) {
        auto comms = &_commodities[order.commodity];
        auto it = find(comms->begin(), comms->end(), order);

        comms->erase(it);
    } else { /* should throw */ }

}

void trading::market::fill_order(id_type order_id) {
    close_order(order_id);
    _orders[order_id].status = trading::order_status::FILLED;
}

void trading::market::revoke_order(id_type order_id) {
    close_order(order_id);
    _orders[order_id].status = trading::order_status::REVOKED;
}

int trading::market::adjust_amount(id_type order_id, int amount) {

    auto order = get_order(order_id);
    int new_amount = order.amount + amount;
    auto comms = &_commodities[order.commodity];
    auto it = find(comms->begin(), comms->end(), order);

    _orders[order_id].amount += amount;
    it->amount += amount;


    if (new_amount == 0) {
        fill_order(order_id);
    }

    return new_amount;
}

// “CHECK” order_ID
std::string trading::market::check(std::vector<std::string> command) {
    std::pair<bool, id_type> order_id = parse_id_type(command[CHECK_ORDER]);
    if (!order_id.first) return invalid_err_msg();

    auto order = get_order(order_id.second);
    if (order.dealer_id != command[DEALER]) return unauthorized_err_msg();
    if (!is_known_order(order.id) || is_revoked(order.id)) return unknown_ord_err_msg();
    if (is_filled(order.id)) return filled_msg(order.id);

    return order.info_str();
}

bool trading::market::is_filled(id_type order_id) {
    return get_order(order_id).status == trading::order_status::FILLED;
}

bool trading::market::is_revoked(id_type order_id) {
    return get_order(order_id).status == trading::order_status::REVOKED;
}

bool trading::market::is_active(id_type order_id) {
    return get_order(order_id).status == trading::order_status::ACTIVE;
}

bool trading::market::is_valid_dealer(std::string id) {
    return find(DEALER_IDS.begin(), DEALER_IDS.end(), id) != DEALER_IDS.end();
}

bool trading::market::is_valid_commodity(std::string id) {
    return find(COMMODITIES.begin(), COMMODITIES.end(), id) != COMMODITIES.end();
}

bool trading::market::is_valid_command(std::string id) {
    return find(COMMANDS.begin(), COMMANDS.end(), id) != COMMANDS.end();
}

bool trading::market::is_known_order(id_type order_id) {
    auto it(_orders.lower_bound(order_id));
    return (it != _orders.end());
}

inline std::string trading::market::unknown_comm_err_msg() { return "UNKNOWN_COMMODITY"; }
inline std::string trading::market::unknown_dealer_err_msg() { return "UNKNOWN_DEALER"; }
inline std::string trading::market::invalid_err_msg() { return "INVALID_MESSAGE"; }
inline std::string trading::market::unknown_ord_err_msg() { return "UNKOWN_ORDER"; }
inline std::string trading::market::unauthorized_err_msg() { return "UNAUTHORIZED"; }
inline std::string trading::market::eol_msg() { return "END_OF_LIST"; }

inline std::string trading::market::filled_msg(id_type order_id) {
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
    message << " FROM ";
    message << order.dealer_id;

    return message.str();
}

inline std::string trading::market::post_msg(order order) {
    std::stringstream message;

    message << order.info_str();
    message << " HAS BEEN POSTED";
    return message.str();
}

inline std::string trading::market::revoke_msg(id_type order_id) {
    std::stringstream message;
    message << order_id << " HAS BEEN REVOKED";
    return message.str();
}
