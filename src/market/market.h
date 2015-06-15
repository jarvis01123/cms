#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <functional>
#include "trading.h"
#include "../misc/misc.h"

namespace trading {


    class market {

        using order = trading::order;
        using side = trading::side;
        using id_type = trading::id_type;
        using money_type = trading::money_type;
        
        // function pointer declaration for handlers
        typedef std::string (market::*handler)(std::vector<std::string>);


    public:
            market();

            // execute a raw input std::string
            std::string execute(std::string message);

    private:

        // core command functionality
        std::string post(std::vector<std::string> command);
        std::string list(std::vector<std::string> command);
        std::string list_for_dealer(std::string commodity, std::string dealer_id);
        std::string list_for_commodity(std::string commodity);
        std::string aggress(std::vector<std::string> command);
        std::string revoke(std::vector<std::string> command);

        // write operations
        void close_order(id_type order_id);
        int adjust_amount(id_type order_id, int amount);
        void add_order(trading::order);
        std::string check(std::vector<std::string> command);
        void fill_order(id_type);
        void revoke_order(id_type);

        // predicates
        bool is_valid_dealer(std::string id);
        bool is_valid_commodity(std::string id);
        bool is_valid_command(std::string id);
        bool is_known_order(id_type ord);
        bool is_filled(id_type ord);
        bool is_revoked(id_type ord);
        bool is_active(id_type ord);

        // misc
        std::pair<bool, std::string> validate(std::vector<std::string> command);
        std::pair<bool, id_type> parse_id_type(std::string val);
        std::pair<bool, money_type> parse_money_type(std::string val);

        id_type next_unique_id();
        trading::order get_order(id_type order_id);

        // messages
        inline std::string unknown_comm_err_msg();
        inline std::string unknown_dealer_err_msg();
        inline std::string invalid_err_msg();
        inline std::string unknown_ord_err_msg();
        inline std::string unauthorized_err_msg();
        inline std::string eol_msg();
        inline std::string filled_msg(id_type order_id);
        inline std::string trade_report(std::string dealer_id, order order, int order_amount);
        inline std::string post_msg(order order);
        inline std::string revoke_msg(id_type order_id);

        // private data members
        std::map<std::string, handler> _handlers;
        std::map<id_type, order> _orders;
        std::map<std::string, std::vector<order>> _commodities;
        id_type _next_id;
        std::mutex _market_mutex;
    };

}
