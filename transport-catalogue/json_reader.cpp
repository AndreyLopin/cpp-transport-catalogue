#include "json_reader.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "json_builder.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport_catalogue {
namespace input {

JsonReader::JsonReader(TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, transport_router::TransportRouter& router)
    : catalogue_(catalogue)
    , renderer_(renderer)
    , router_(router) {
}

void JsonReader::ApplyCommands(void) {
    AddStops();
    AddDistances();
    AddBuses();
    renderer_.SetSettings(GetRenderSettings());
    //router_ = transport_router::TransportRouter(catalogue_, GetRoutingSettings());
} 

void JsonReader::AnswersRequests(std::ostream& out) {
    json::Array doc;
    std::stringstream output;

    
    //router_.FillGraphs(catalogue_, transport_graph);
    //graph::Router transport_router(transport_graph);

    for(auto& el : stat_requests_) {
        if(el.AsDict().at("type").AsString() == "Map") {
            doc.emplace_back(PrintMap(el));
        }

        if(el.AsDict().at("type").AsString() == "Bus") {
            doc.emplace_back(PrintBusInfo(el));
        }

        if(el.AsDict().at("type").AsString() == "Stop") {
            doc.emplace_back(PrintStopInfo(el));
        }

        if(el.AsDict().at("type").AsString() == "Route") {
            doc.emplace_back(PrintRoute(el));
        }
    }
    
    json::Print(json::Document(doc), out);
}

void LoadJSON(JsonReader& reader, std::istream& in, std::ostream& out) {
    json::Document doc = json::Load(in);
    const auto load_dict = doc.GetRoot().AsDict();
    reader.SetBaseRequest(load_dict.at("base_requests").AsArray());
    reader.SetStatRequest(load_dict.at("stat_requests").AsArray());
    reader.SetRenderSettings(load_dict.at("render_settings").AsDict());
    reader.SetRoutingSettings(load_dict.at("routing_settings").AsDict());
    reader.ApplyCommands();
    reader.AnswersRequests(out);
}

map_renderer::RenderSettings JsonReader::GetRenderSettings(void) {
    map_renderer::RenderSettings result;
    result.width_ = render_settings_.at("width").AsDouble();
    result.height_ = render_settings_.at("height").AsDouble();

    result.padding_ = render_settings_.at("padding").AsDouble();

    result.line_width_ = render_settings_.at("line_width").AsDouble();
    result.stop_radius_ = render_settings_.at("stop_radius").AsDouble();

    result.bus_label_font_size_ = render_settings_.at("bus_label_font_size").AsInt();
    result.bus_label_offset_ = {
        render_settings_.at("bus_label_offset").AsArray()[0].AsDouble(),
        render_settings_.at("bus_label_offset").AsArray()[1].AsDouble()
    };

    result.stop_label_font_size_ = render_settings_.at("stop_label_font_size").AsInt();
    result.stop_label_offset_ = {
        render_settings_.at("stop_label_offset").AsArray()[0].AsDouble(),
        render_settings_.at("stop_label_offset").AsArray()[1].AsDouble()
    };

    result.underlayer_color_ = GetColor(render_settings_.at("underlayer_color"));
    result.underlayer_width_ = render_settings_.at("underlayer_width").AsDouble();

    for(const auto& el : render_settings_.at("color_palette").AsArray()) {
        result.color_palette_.push_back(GetColor(el));
    }
    
    return result;
}

transport_router::RoutingSettings JsonReader::GetRoutingSettings(void) {
    transport_router::RoutingSettings result;

    result.bus_wait_time = routing_settings_.at("bus_wait_time").AsInt();
    result.bus_velocity = routing_settings_.at("bus_velocity").AsDouble();

    return result;
}

svg::Color JsonReader::GetColor(const json::Node& el) const {
    using namespace std::string_literals;
    std::stringstream color;
    if(el.IsString()) {
        color << el.AsString();
    } else if(el.IsArray()) {
        if(el.AsArray().size() == 3) {
            color << "rgb("s;
        } else {
            color << "rgba("s;
        }
        
        bool is_not_first = false;
        for(uint32_t i = 0; i < 3; i++) {
            if(is_not_first) {
                color << ","s;
            }
            color << el.AsArray()[i].AsInt();
            is_not_first = true;
        }

        if(el.AsArray().size() == 4) {
            color << ","s;
            color << el.AsArray()[3].AsDouble();
        }

        color << ")"s;
    }
    return color.str();
}

json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

json::Node JsonReader::PrintMap(const json::Node& request) {
    using namespace std::string_literals;
    std::stringstream output;

    renderer_.RenderMap().Render(output);
    return  json::Builder{}.StartDict()
                    .Key("request_id"s)
                    .Value(request.AsDict().at("id"s).AsInt())
                    .Key("map"s).Value(output.str())
                    .EndDict()
                    .Build();
}

json::Node JsonReader::PrintBusInfo(const json::Node& request) {
    using namespace std::string_literals;
    domain::BusInfo bus = catalogue_.GetBusInfo(request.AsDict().at("name"s).AsString());
    
    json::Builder answer = json::Builder{};
    answer.StartDict().Key("request_id"s).Value(request.AsDict().at("id"s).AsInt());
    if(bus.name.empty()) {
        answer.Key("error_message"s).Value("not found"s);
    } else {
        answer.Key("curvature"s).Value(bus.route_curvature);
        answer.Key("route_length"s).Value(bus.route_length);
        answer.Key("stop_count"s).Value(static_cast<int>(bus.count_all_stops));
        answer.Key("unique_stop_count"s).Value(static_cast<int>(bus.count_unique_stops));
    }

    return answer.EndDict().Build();
}

json::Node JsonReader::PrintStopInfo(const json::Node& request) {
    using namespace std::string_literals;
    domain::StopInfo stop = catalogue_.GetStopInfo(request.AsDict().at("name"s).AsString());

    json::Builder answer = json::Builder{};
    answer.StartDict().Key("request_id"s).Value(request.AsDict().at("id"s).AsInt());
    if(stop.name.empty()) {
        answer.Key("error_message"s).Value("not found"s);
    } else {
        answer.Key("buses"s).StartArray();
        for(const auto bus : stop.buses) {
            answer.Value(std::string(bus));
        }
        answer.EndArray();
    }
    
    return answer.EndDict().Build();
}

json::Node JsonReader::PrintRoute(const json::Node& request) {
    using namespace std::string_literals;

    json::Builder answer = json::Builder{};
    answer.StartDict();

    domain::Stop* from = catalogue_.FindStop(request.AsDict().at("from").AsString());
    domain::Stop* to = catalogue_.FindStop(request.AsDict().at("to").AsString());

    if (from == to) {
        answer.Key("total_time"s).Value(0)
            .Key("request_id"s).Value(request.AsDict().at("id").AsInt())
            .Key("items"s)
            .StartArray()
            .EndArray();
    } else {
        const auto route = router_.FindRoute(from->id, to->id);
        
        if (route.has_value()) {
            const auto& elem = route.value().edges;
            json::Array items;
            for (const auto& el : elem) {
                const auto& edge;// = transport_router.GetGraph().GetEdge(el);
                items.push_back(json::Builder().StartDict()
                    .Key("time").Value(router_.GetBusWaitTime())
                    .Key("type").Value("Wait")
                    .Key("stop_name").Value(catalogue_.GetAllStops()[edge.from].name)
                    .EndDict().Build());
                items.push_back(json::Builder().StartDict()
                    .Key("time"s).Value(edge.weight - router_.GetBusWaitTime())
                    .Key("span_count"s).Value(static_cast<int>(edge.span_count))
                    .Key("bus"s).Value(edge.bus)
                    .Key("type"s).Value("Bus"s)
                    .EndDict().Build());
            }
            answer.Key("total_time"s).Value(route.value().weight)
                .Key("request_id"s).Value(request.AsDict().at("id").AsInt())
                .Key("items"s).Value(items);
        } else {
            answer.Key("request_id"s).Value(request.AsDict().at("id").AsInt())
                .Key("error_message").Value("not found"s);
        }
    }

    return answer.EndDict().Build();
}

void JsonReader::AddStops(void) const {
    // Add all stops
    for(auto& el : base_requests_) {
        if (el.AsDict().at("type").AsString() == "Stop") {
            catalogue_.AddStop(el.AsDict().at("name").AsString(), {el.AsDict().at("latitude").AsDouble(), el.AsDict().at("longitude").AsDouble()});
        }
    }
}

void JsonReader::AddDistances(void) const {
    // Add all distances
    for(const auto& el : base_requests_) {
        if (el.AsDict().at("type").AsString() == "Stop") {
            for(const auto& [key, value] : el.AsDict().at("road_distances").AsDict()) {                
                catalogue_.SetDistanceStops(catalogue_.FindStop(el.AsDict().at("name").AsString()), catalogue_.FindStop(key), value.AsDouble());
            }
        }
    }
}

void JsonReader::AddBuses(void) const {
    // Add all buses
    std::vector<geo::Coordinates> all_coordinates;
    for(const auto& el : base_requests_) {
        if (el.AsDict().at("type").AsString() == "Bus") {
            domain::Bus bus;
            bus.name_ = el.AsDict().at("name").AsString();
            bool is_roundtrip = el.AsDict().at("is_roundtrip").AsBool();
            bus.is_roundtrip_ = is_roundtrip;
            

            for(const auto& stop : el.AsDict().at("stops").AsArray()) {
                bus.stops_.push_back(catalogue_.FindStop(stop.AsString()));
            }

            bus.end_stop_ = bus.stops_.back();

            if(!is_roundtrip) {
                for(int i = static_cast<int>(el.AsDict().at("stops").AsArray().size()) - 2; i >= 0; --i) {
                    bus.stops_.push_back(catalogue_.FindStop(el.AsDict().at("stops").AsArray()[static_cast<uint32_t>(i)].AsString()));
                }
            }

            for(const auto& stop : bus.stops_) {
                all_coordinates.push_back(stop->coordinates);
            }
            catalogue_.AddBus(bus);
            renderer_.AddRoute(catalogue_.FindBus(bus.name_));
        }
    }

    renderer_.SetSphereProjector(all_coordinates);
}

}; //namespace input
}; //namespace transport_catalogue