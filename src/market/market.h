#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <functional>


using namespace std;


namespace my {

    int MAX = 256;

    vector<string> split_on_white(string s) {

        vector<string> tbr;
        string temp;
        while (s.length() > 0) {
            temp = s.substr(0, s.find(' '));
            s.erase(0, temp.length() + 1);
            tbr.push_back(temp);
        }

        return tbr;
    }

    string strip_trailing_newline(string str) {
        string tbr = str;
        auto last = tbr.find_last_not_of('\n');
        if (last == tbr.length()-2) {
            tbr.erase(last + 1);
        }

        return tbr;
    }
};


namespace trading {

    enum Side {
        BUY,
        SELL
    };


    string stringify(Side side) {
        return side == BUY ? "BUY" : "SELL";
    }

    Side sideify(string s) {
        if (s == "BUY") {
            return BUY;
        } else {
            return SELL;
        }
    }

    struct order {
        Side side;
        int id;
        string dealer_id;
        string commodity;
        int amount;
        double price;

        bool operator==(const order& rhs) const {
            return this->id == rhs.id;
        }

        string info_str() {
            stringstream message;

            message << id << " ";
            message << dealer_id << " ";
            message << amount << " ";
            message << trading::stringify(side) << " ";
            message << commodity << " ";
            message << price;

            return message.str();
        }

    };

    const string UNKOWN_COMMODITY = "UNKNOWN_COMMODITY";

    const vector<string> DEALER_IDS{
        "DB",
        "JPM",
        "UBS",
        "RBC",
        "BARX",
        "MS",
        "CITI",
        "BOFA",
        "RBS",
        "HSBC"
    };

    const vector<string> COMMODITIES{
        "GOLD",
        "SILV",
        "PORK",
        "OIL",
        "RICE"
    };

    const vector<string> COMMANDS {
        "POST",
        "REVOKE",
        "CHECK",
        "LIST",
        "AGGRESS"
    };



    class market {

        using order = trading::order;
        using Side = trading::Side;

        // function pointer declaration for handlers
        typedef string (market::*handler)(vector<string>);


    public:
            market(): next_id{0} {
                handlers.emplace( make_pair("POST", &market::post) );
                handlers.emplace( make_pair("LIST", &market::list) );
                handlers.emplace( make_pair("CHECK", &market::check) );
                handlers.emplace( make_pair("AGGRESS", &market::aggress) );
                handlers.emplace( make_pair("REVOKE", &market::revoke) );
            }

            // execute a raw input string
            string str_exec(string message) {
                auto command = my::split_on_white(message);

                auto res = validate(command);

                if (!res.first) return res.second;

                handler x = handlers[command[1]];
                return (this->*x)(command);
            }

            vector<string> get_commands() {
                return COMMANDS;
            }

    private:

        pair<bool, string> validate(vector<string> command) {
            pair<bool, string> tbr;
            tbr.first = true;
            tbr.second = "default";

            if (!(tbr.first = is_valid_dealer(command[0]))) {
                tbr.second = unknown_dealer_err_msg();

            } else if (!(tbr.first = is_valid_command(command[1]))) {
                tbr.second = invalid_err_msg();
            }

            return tbr;
        }

        /* map of function pointers */
        map<string, handler> handlers;

        /* order_id -> order */
        map<int, order> orders;

        /* commodity -> list of orders */
        map<string, vector<order>> commodities;

        int next_id;

        int next_unique_id() {
            return ++next_id;
        }


        // ############### methods ######################

        order get_order(int order_id) {
            return orders[order_id];
        }

        // “POST” SIDE COMMODITY AMOUNT PRICE
        string post(vector<string> command) {

            order order;
            order.dealer_id = command[0];
            order.id = next_unique_id();
            order.side = trading::sideify(command[2]);
            order.commodity = command[3];
            order.amount = (int)atoi(command[4].c_str());
            order.price = stof(command[5]);

            orders[order.id] = order;
            commodities[order.commodity].push_back(order);

            return post_msg(order);
        }

        // “LIST” [ COMMODITY [ DEALER_ID ] ]
        string list(vector<string> command) {

            string commodity = command[2];

            if (command.size() == 4) {
                return list_for_dealer(commodity, command[0]);
            }

            if (!is_valid_commodity(commodity)) {
                return unknown_comm_err_msg();
            }

            stringstream message;
            for (auto ord : commodities[commodity]) {
                if (ord.id != 0) {
                    message << ord.info_str() << endl;
                }
            }

            message << eol_msg();

            return message.str();

        }

        string list_for_dealer(string commodity, string dealer_id) {

            if (!is_valid_commodity(commodity)) {
                return unknown_comm_err_msg();
            }

            stringstream message;
            for (auto ord : commodities[commodity]) {

                if (ord.id != 0 && ord.dealer_id == dealer_id) {
                    message << ord.info_str() << endl;
                }
            }

            return my::strip_trailing_newline(message.str());

        }

        // “AGGRESS” ( order_ID AMOUNT )+
        string aggress(vector<string> command) {

            stringstream message;

            int order_id, order_amount;

            if ((command.size() % 2) != 0) {
                return invalid_err_msg();
            }

            for (size_t i = 2; i < command.size(); i += 2) {

                order_id = (int)atoi(command[i].c_str());
                order_amount = (int)atoi(command[i + 1].c_str());

                auto order = get_order(order_id);

                if (order.id == 0) {
                    message << unknown_ord_err_msg() << endl;

                } else {

                    if (order.amount >= order_amount) {

                        auto new_amt = adjust_amount(order_id, (-1)*order_amount);

                        message << trade_report(command[0], order, order_amount) << endl;

                        if (new_amt == 0) {
                            message << filled_msg(order_id) << endl;
                            close_order(order_id);
                        }

                    } else {
                        message << unauthorized_err_msg() << endl;
                    }
                }

            }

            return my::strip_trailing_newline(message.str());

        }


        // “REVOKE” order_ID
        string revoke(vector<string> command) {

            auto order_id = (int)atoi(command[2].c_str());

            close_order(order_id);

            return revoke_msg(order_id);
        }

        void close_order(int order_id) {

            auto order = get_order(order_id);
            auto comms = &commodities[order.commodity];

            orders.erase(order_id);

            auto it = find(comms->begin(), comms->end(), order);
            comms->erase(it);
        }

        int adjust_amount(int order_id, int amount) {

            auto order = get_order(order_id);
            int new_amount = order.amount + amount;


            orders[order_id].amount += amount;
            auto comms = &commodities[order.commodity];

            auto it = find(comms->begin(), comms->end(), order);

            it->amount += amount;

            return new_amount;
        }

        // “CHECK” order_ID
        string check(vector<string> command) {
            int order_id = (int)atoi(command[2].c_str());
            auto order = get_order(order_id);
            return order.info_str();

        }


        // ### helper methods ###

        bool is_valid_dealer(string id) {
            return find(DEALER_IDS.begin(), DEALER_IDS.end(), id) != DEALER_IDS.end();
        }

        bool is_valid_commodity(string id) {
            return find(COMMODITIES.begin(), COMMODITIES.end(), id) != COMMODITIES.end();
        }

        bool is_valid_command(string id) {
            return find(COMMANDS.begin(), COMMANDS.end(), id) != COMMANDS.end();
        }

        inline string unknown_comm_err_msg() { return "UNKNOWN_COMMODITY"; }
        inline string unknown_dealer_err_msg() { return "UNKNOWN_DEALER"; }
        inline string invalid_err_msg() { return "INVALID"; }
        inline string unknown_ord_err_msg() { return "UNKOWN_ORDER"; }
        inline string unauthorized_err_msg() { return "UNAUTHORIZED"; }
        inline string eol_msg() { return "END_OF_LIST"; }

        inline string filled_msg(int order_id) {
            stringstream message;
            message << order_id << " HAS BEEN FILLED";
            return message.str();
        }

        inline string trade_report(string dealer_id, order order, int order_amount) {
            stringstream message;

            message << dealer_id << " ";
            message << (order.side == trading::SELL ? "BOUGHT " : "SOLD ");
            message << order_amount << " @ ";
            message << order.price;

            return message.str();
        }

        inline string post_msg(order order) {
            stringstream message;

            message << order.info_str();
            message << " HAS BEEN POSTED";
            return message.str();
        }

        inline string revoke_msg(int order_id) {
            stringstream message;
            message << order_id << " HAS BEEN REVOKED";
            return message.str();
        }

    };

}
