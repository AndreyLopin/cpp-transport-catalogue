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

        void PrintBusInfo(std::ostream& output, const domain::BusInfo& bus_info);
        void PrintStopInfo(std::ostream& output, const domain::StopInfo& stop_info);

        void ReadFromStreamAndWriteToStream(const TransportCatalogue& tansport_catalogue,
                                        std::istream& input, std::ostream& output);
    };
};