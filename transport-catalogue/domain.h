#pragma once

#include <set>
#include <string>
#include <vector>

#include "geo.h"
/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace domain {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;

    bool operator==(const Stop& other) const {
        return coordinates == other.coordinates && name == other.name;
    }
    bool operator!=(const Stop& other) const {
        return !(*this == other);
    }
};

struct Bus {
    std::string name_;
    std::vector<Stop*> stops_;
    bool is_roundtrip_;
    Stop* end_stop_;

    bool operator==(const Bus& other) const {
        return stops_ == other.stops_ && name_ == other.name_;
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

}; //namespace domain