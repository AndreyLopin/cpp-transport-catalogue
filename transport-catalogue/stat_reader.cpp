#include "stat_reader.h"

#include <iomanip>
#include <sstream>

using namespace std::literals;

namespace transport_catalogue {
    namespace output {
        StatCommandDescription ParseStatCommandDescription(std::string_view line) {
            auto space_pos = line.find(' ');
            return {std::string(line.substr(0, space_pos)),
                    std::string(line.substr(space_pos + 1))};
        }

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                            std::ostream& output) {
            StatCommandDescription command = ParseStatCommandDescription(request);
            
            if (command.name == "Bus"s) {
                BusInfo bus_info = tansport_catalogue.GetBusInfo(command.id);
                PrintBusInfo(output, bus_info);
            } else if (command.name == "Stop"s) {
                StopInfo stop_info = tansport_catalogue.GetStopInfo(command.id);
                PrintStopInfo(output, stop_info);
            }
        }

        void PrintBusInfo(std::ostream& output, const BusInfo& bus_info) {
            output << "Bus "s << bus_info.name;

            if (!bus_info.name.empty()) {
                int current_precesion = static_cast<int>(output.precision());          
                std::setprecision(6);
                output << ": "s << bus_info.count_all_stops << " stops on route, "s;
                output << bus_info.count_unique_stops << " unique stops, " << bus_info.route_length << " route length\n";
                std::setprecision(current_precesion);
            } else {
                output << ": not found\n"s;
            }
        }

        void PrintStopInfo(std::ostream& output, const StopInfo& stop_info) {
            output << "Stop "s << stop_info.name;

            if (!stop_info.name.empty()) {
                if (stop_info.buses.empty()) {
                    output << ": no buses\n"s;
                } else {
                    output << ": buses";
                    for (const auto& bus : stop_info.buses) {
                        output << ' ' << bus;
                    }
                    output << '\n';
                }
            } else {
                output << ": not found\n"s;
            }
        }

        void ReadFromStreamAndWriteToStream(const TransportCatalogue& tansport_catalogue, std::istream& input, std::ostream& output) {
            int stat_request_count;
            input >> stat_request_count >> std::ws;
            for (int i = 0; i < stat_request_count; ++i) {
                std::string line;
                getline(input, line);
                transport_catalogue::output::ParseAndPrintStat(tansport_catalogue, line, output);
            }
        }
    };
};