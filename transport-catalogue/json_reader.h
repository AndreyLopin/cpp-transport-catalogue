#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace transport_catalogue {
namespace input {

class JsonReader {
public:
    JsonReader(TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, std::istream& in);
    map_renderer::RenderSettings GetRenderSettings(void);
    void ApplyCommands(void) const;
    void AnswersRequests(std::ostream& out) const;
private:
    void AddStops(void) const;
    void AddDistances(void) const;
    void AddBuses(void) const;
    svg::Color GetColor(const json::Node& el) const;

    TransportCatalogue& catalogue_;
    map_renderer::MapRenderer& renderer_;
    json::Array base_requests_;
    json::Array stat_requests_;
    json::Dict render_settings_;
};

std::string PrintBusInfo(const domain::BusInfo& bus_info);
std::string PrintStopInfo(const domain::StopInfo& stop_info);

json::Document LoadJSON(const std::string& s);

void JsonDownload(TransportCatalogue& catalogue, std::istream& in, std::ostream& out);

}; //namespace input
}; //namespace transport_catalogue