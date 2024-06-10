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
        std::vector<Stop*> stops;

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
        double geo_length;
        double route_length;
        double route_curvature;

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

    struct DistanceHasher {
        size_t operator() (const std::pair<Stop*, Stop*>& distance) const {
            return hasher_p_(distance.first) + 31 * hasher_p_(distance.second);
        }
    private:
        std::hash<Stop*> hasher_p_;
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string& name, const distance::Coordinates& coordinates);
        Stop* FindStop(const std::string_view& name) const;
        void AddBus(const std::string& name, const std::vector<std::string_view>& stops);
        const Bus* FindBus(const std::string_view& name) const;
        BusInfo GetBusInfo(const std::string_view& name) const;
        StopInfo GetStopInfo(const std::string_view& name) const;

        void SetDistanceStops(const std::string_view& from, const std::string_view& to, const double& distance);
        double GetDistanceStops(Stop* from, Stop* to) const;

    private:
        double GetBusGeoLength(const std::string_view& name) const;
        double GetBusRouteLength(const std::string_view& name) const;

        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop*> ptr_stops_;

        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> ptr_buses_;

        std::unordered_map<std::string_view, std::unordered_set<const Bus*>> buses_for_stop_;

        std::unordered_map<std::pair<Stop*, Stop*>, double, DistanceHasher> distances_;
    };
};