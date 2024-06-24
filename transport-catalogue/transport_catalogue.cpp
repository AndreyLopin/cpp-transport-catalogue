#include "transport_catalogue.h"

namespace transport_catalogue {
    void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates& coordinates) {
        stops_.push_back(std::move(domain::Stop{name, std::move(coordinates)}));
        ptr_stops_.emplace(std::string_view(stops_.back().name), &stops_.back());
    }

    domain::Stop* TransportCatalogue::FindStop(const std::string_view& name) const {
        auto result = ptr_stops_.find(name);
        return result == ptr_stops_.end() ? nullptr : result->second;
    }

    void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string_view>& stops) {
        domain::Bus bus;
        bus.name = std::move(name);
        for (const std::string_view& stop : stops) {
            bus.stops.push_back(ptr_stops_.find(stop)->second);
        }
        buses_.push_back(std::move(bus));
        ptr_buses_.emplace(std::string_view(buses_.back().name), &buses_.back());

        for (const std::string_view& stop : stops) {
            buses_for_stop_[ptr_stops_.find(stop)->first].insert(&buses_.back());
        }
    }

    void TransportCatalogue::AddBus(const domain::Bus& bus) {
        buses_.push_back(std::move(bus));
        ptr_buses_.emplace(std::string_view(buses_.back().name), &buses_.back());

        for (const auto* stop: buses_.back().stops) {
            buses_for_stop_[stop->name].insert(&buses_.back());
        }
    }

    domain::Bus* TransportCatalogue::FindBus(const std::string_view& name) const {
        auto result = ptr_buses_.find(name);
        return result == ptr_buses_.end() ? nullptr : result->second;
    }

    domain::BusInfo TransportCatalogue::GetBusInfo(const std::string_view& name) const {
        domain::BusInfo result;

        if (ptr_buses_.find(name) != ptr_buses_.end()) {
            result.name = name;
            result.count_all_stops = ptr_buses_.at(name)->stops.size();
            std::unordered_set<std::string_view> unique_stops;
            for (const auto stop : ptr_buses_.at(name)->stops) {
                unique_stops.insert(stop->name);
            }
            result.count_unique_stops = unique_stops.size();
            result.geo_length = GetBusGeoLength(name);
            result.route_length = GetBusRouteLength(name);
            result.route_curvature = result.route_length / result.geo_length;
        }

        return result;
    }

    domain::StopInfo TransportCatalogue::GetStopInfo(const std::string_view& name) const {
        domain::StopInfo result;

        if (ptr_stops_.find(name) != ptr_stops_.end()) {
            result.name = name;
            if (buses_for_stop_.find(ptr_stops_.find(name)->first) != buses_for_stop_.end()) {
                for (const domain::Bus* bus : buses_for_stop_.find(ptr_stops_.find(name)->first)->second) {
                    result.buses.insert(std::string_view(bus->name));
                }
            }
        }

        return result;
    }

    double TransportCatalogue::GetBusGeoLength(const std::string_view& name) const {
        double route_length = 0.0;
        geo::Coordinates from {-1.0, -1.0}, to {-1.0, -1.0};
        for (const auto stop : ptr_buses_.at(name)->stops) {
            if (from == geo::Coordinates {-1.0, -1.0}) { 
                from = stop->coordinates;
                continue;
            }
            if (to == geo::Coordinates {-1.0, -1.0}) {
                to = stop->coordinates;
                route_length += ComputeDistance(from, to);
                continue;
            }
            
            from = to;
            to = stop->coordinates;
            route_length += ComputeDistance(from, to);
        }

        return route_length;
    }

    double TransportCatalogue::GetBusRouteLength(const std::string_view& name) const {
        double route_length = 0.0;
        domain::Stop* from = nullptr;
        domain::Stop* to = nullptr;
        for (domain::Stop* stop : ptr_buses_.at(name)->stops) {
            if (from == nullptr) {
                from = stop;
                continue;
            }
            if (to == nullptr) {
                to = stop;
                route_length += GetDistanceStops(from, to);
                continue;
            }
            
            from = to;
            to = stop;

            route_length += GetDistanceStops(from, to);
        }

        return route_length;
    }

    void TransportCatalogue::SetDistanceStops(const std::string_view& from, const std::string_view& to, const double& distance) {
        domain::Stop* ptr_from = FindStop(from);
        domain::Stop* ptr_to = FindStop(to);

        if (ptr_from != nullptr && ptr_to != nullptr) {
            distances_.emplace(std::pair<domain::Stop*, domain::Stop*>(ptr_from, ptr_to), distance);
        }
    }

     void TransportCatalogue::SetDistanceStops(domain::Stop* from, domain::Stop* to, const double& distance) {
        if (from != nullptr && to != nullptr) {
            distances_.emplace(std::pair<domain::Stop*, domain::Stop*>(from, to), distance);
        }
     }

    double TransportCatalogue::GetDistanceStops(domain::Stop* from, domain::Stop* to) const {
        if (distances_.count(std::pair<domain::Stop*, domain::Stop*>(from, to)) > 0) {
            return distances_.at(std::pair<domain::Stop*, domain::Stop*>(from, to));
        } else if (distances_.count(std::pair<domain::Stop*, domain::Stop*>(to, from)) > 0) {
            return distances_.at(std::pair<domain::Stop*, domain::Stop*>(to, from));
        }

        return 0;
    }
};