#pragma once

#include <vector>
#include <string>
#include <sstream>

namespace trading {

    typedef double money_type;
    typedef int id_type;

    enum side {
        BUY,
        SELL,
        ERROR
    };

    enum order_status {
        STATUS_ERROR,
        ACTIVE,
        FILLED,
        REVOKED
    };

    std::string stringify(side side);
    trading::side sideify(std::string s);

    struct order {
        side side;
        id_type id;
        std::string dealer_id;
        std::string commodity;
        int amount;
        money_type price;
        order_status status;

        bool operator==(const order& rhs) const;

        std::string info_str();

    };

    const std::string UNKOWN_COMMODITY = "UNKNOWN_COMMODITY";

    const std::vector<std::string> DEALER_IDS {
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

    const std::vector<std::string> COMMODITIES{
        "GOLD",
        "SILV",
        "PORK",
        "OIL",
        "RICE"
    };

    const std::vector<std::string> COMMANDS {
        "POST",
        "REVOKE",
        "CHECK",
        "LIST",
        "AGGRESS"
    };

    namespace constants {

        namespace positions {
            const int DEALER = 0;
            const int COMMAND = 1;
            const int POST_SIDE = 2;
            const int POST_COMMODITY = 3;
            const int POST_AMOUNT = 4;
            const int POST_PRICE = 5;
            const int LIST_COMMODITY = 2;
            const int LIST_DEALER = 3;
            const int CHECK_ORDER = 2;
            const int REVOKE_ORDER = 2;
        };

        const int EMPTY_ORDER = 0;
        const int POST_NUM_ARGS = 6;
        const int LIST_DEALER_NUM_ARGS = 4;
        const int LIST_COMM_NUM_ARGS = 3;
        const int LIST_NUM_ARGS = 2;
    };
};
