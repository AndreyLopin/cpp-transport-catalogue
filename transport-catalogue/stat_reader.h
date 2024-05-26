#pragma once

#include <iosfwd>
#include <string>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue {
    namespace output {
        struct StatCommandDescription {
            std::string name;
            std::string id;
        };

        StatCommandDescription ParseStatCommandDescription(std::string_view line);

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                            std::ostream& output);
    };
};