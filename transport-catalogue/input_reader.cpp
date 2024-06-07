#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

using namespace std::literals;

namespace transport_catalogue {
    namespace input {
        /**
         * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
         */
        distance::Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return {nan, nan};
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2)));

            return {lat, lng};
        }

        /**
         * Удаляет пробелы в начале и конце строки
         */
        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        /**
         * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
         */
        std::vector<std::string_view> Split(std::string_view string, char delim) {
            std::vector<std::string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }

            return result;
        }

        /**
         * Парсит маршрут.
         * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
         * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
         */
        std::vector<std::string_view> ParseRoute(std::string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        /**
         * Разбивает строку на две
         * Возвращает пару строк, строку с координатами и строку с расстояниями
        */
        std::pair<std::string_view, std::string_view> ParseStopDescription(std::string_view str) {
            auto comma_1 = str.find(',');
            
            if(comma_1 == str.npos) {
                return {};
            }
            auto comma_2 = str.find(',', comma_1 + 1);

            return {std::string(str.substr(0, comma_2)), std::string(str.substr(comma_2 + 1))};
        }

        CommandDescription ParseCommandDescription(std::string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return {std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1))};
        }

        void InputReader::ParseLine(std::string_view line) {
            auto command_description = ParseCommandDescription(line);
            if (command_description) {
                commands_.push_back(std::move(command_description));
            }
        }

        std::vector<std::pair<std::string_view, double>> ParseDistances(std::string_view str) {
            std::vector<std::pair<std::string_view, double>> result;
            std::vector<std::string_view> stops = Split(str, ',');

            for (std::string_view s : stops) {
                auto not_space = s.find_first_not_of(' ');
                auto delim = s.find('m');
                auto start_name = s.find('o', delim + 1);

                double distance = std::stod(std::string(s.substr(not_space, delim - not_space)));
                std::string_view name = Trim(std::string(s.substr(start_name + 1)));

                result.push_back({name, distance});
            }

            return result;
        }

        void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
            // Add all stops
            for (CommandDescription cmd : commands_) {
                if (cmd.command == "Stop"s) {
                    std::pair<std::string_view, std::string_view> stop_description = ParseStopDescription(cmd.description);
                    catalogue.AddStop(std::string(Trim(cmd.id)), ParseCoordinates(stop_description.first), ParseDistances(stop_description.second));
                }
            }

            // Add all buses
            for (CommandDescription cmd : commands_) {
                if (cmd.command == "Bus"s) {
                    catalogue.AddBus(std::string(Trim(cmd.id)), ParseRoute(cmd.description));
                }
            }
        }

        void ReadFromStream(std::istream& input, TransportCatalogue& catalogue) {
            int base_request_count;
            input >> base_request_count >> std::ws;

            transport_catalogue::input::InputReader reader;
            for (int i = 0; i < base_request_count; ++i) {
                std::string line;
                getline(input, line);
                reader.ParseLine(line);
            }
            reader.ApplyCommands(catalogue);
        }
    };
};