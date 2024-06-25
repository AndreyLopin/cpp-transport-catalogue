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
    JsonReader(TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer);

    map_renderer::RenderSettings GetRenderSettings(void);

    void ApplyCommands(void);

    void AnswersRequests(std::ostream& out);

    void SetBaseRequest(const json::Array& base_requests) {
        base_requests_ = base_requests;
    }

    void SetStatRequest(const json::Array& stat_requests) {
        stat_requests_ = stat_requests;
    }

    void SetRenderSettings(const json::Dict& render_settings) {
        render_settings_ = render_settings;
    }
private:
    void AddStops(void) const;
    void AddDistances(void) const;
    void AddBuses(void) const;
    svg::Color GetColor(const json::Node& el) const;

    json::Node PrintMap(const json::Node& request);
    json::Node PrintBusInfo(const json::Node& request);
    json::Node PrintStopInfo(const json::Node& request);

    TransportCatalogue& catalogue_;
    map_renderer::MapRenderer& renderer_;
    json::Array base_requests_;
    json::Array stat_requests_;
    json::Dict render_settings_;
};

void LoadJSON(JsonReader& reader, std::istream& in, std::ostream& out);

}; //namespace input
}; //namespace transport_catalogue