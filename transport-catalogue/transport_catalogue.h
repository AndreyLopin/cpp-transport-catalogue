#pragma once

#include <deque>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

namespace transport_catalogue {
    struct Stop {
        std::string name;
        distance::Coordinates coordinates;

        bool operator==(const Stop& other) const {
            return coordinates == other.coordinates && name == other.name;
        }
        bool operator!=(const Stop& other) const {
            return !(*this == other);
        }
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> stops;

        bool operator==(const Bus& other) const {
            return stops == other.stops && name == other.name;
        }
        bool operator!=(const Bus& other) const {
            return !(*this == other);
        }
    };

    struct BusInfo {
        std::string name;
        size_t count_unique_stops;
        size_t count_all_stops;
        double route_length;

        bool operator==(const BusInfo& other) const {
            return name == other.name && count_unique_stops == other.count_unique_stops
                && count_all_stops == other.count_all_stops && route_length == other.route_length;
        }
        bool operator!=(const BusInfo& other) const {
            return !(*this == other);
        }
    };

    struct StopInfo {
        std::string name;
        std::set<std::string_view> buses;
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string& name, const distance::Coordinates& coordinates);
        const Stop* FindStop(const std::string_view& name) const;
        void AddBus(const std::string& name, const std::vector<std::string_view>& stops);
        const Bus* FindBus(const std::string_view& name) const;
        BusInfo GetBusInfo(const std::string_view& name) const;
        StopInfo GetStopInfo(const std::string_view& name) const;

    private:
        double GetBusRouteLength(const std::string_view& name) const;

        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, const Stop*> ptr_stops_;

        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, const Bus*> ptr_buses_;

        std::unordered_map<std::string_view, std::unordered_set<const Bus*>> buses_for_stop_;
    };
};