#pragma once

#include <vector>
#include <string>
#include <sstream>

namespace trading {
    enum side {
        BUY,
        SELL
    };


    std::string stringify(side side);
    trading::side sideify(std::string s);

    struct order {
        side side;
        int id;
        std::string dealer_id;
        std::string commodity;
        int amount;
        double price;

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



};
