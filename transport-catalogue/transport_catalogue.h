#pragma once

#include <deque>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "domain.h"

namespace transport_catalogue {    

    struct DistanceHasher {
        size_t operator() (const std::pair<domain::Stop*, domain::Stop*>& distance) const {
            return hasher_p_(distance.first) + 31 * hasher_p_(distance.second);
        }
    private:
        std::hash<domain::Stop*> hasher_p_;
    };

    class TransportCatalogue {
    public:
        void AddStop(const std::string& name, const geo::Coordinates& coordinates);
        domain::Stop* FindStop(const std::string_view& name) const;
        void AddBus(const std::string& name, const std::vector<std::string_view>& stops);
        void AddBus(const domain::Bus& bus);
        domain::Bus* FindBus(const std::string_view& name) const;
        domain::BusInfo GetBusInfo(const std::string_view& name) const;
        domain::StopInfo GetStopInfo(const std::string_view& name) const;

        void SetDistanceStops(const std::string_view& from, const std::string_view& to, const double& distance);
        void SetDistanceStops(domain::Stop* from, domain::Stop* to, const double& distance);
        double GetDistanceStops(domain::Stop* from, domain::Stop* to) const;

        size_t GetStopsCount() const {
            return stops_.size();
        }

    private:
        double GetBusGeoLength(const std::string_view& name) const;
        double GetBusRouteLength(const std::string_view& name) const;

        std::deque<domain::Stop> stops_;
        std::unordered_map<std::string_view, domain::Stop*> ptr_stops_;

        std::deque<domain::Bus> buses_;
        std::unordered_map<std::string_view, domain::Bus*> ptr_buses_;

        std::unordered_map<std::string_view, std::unordered_set<const domain::Bus*>> buses_for_stop_;

        std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, double, DistanceHasher> distances_;
    };
};