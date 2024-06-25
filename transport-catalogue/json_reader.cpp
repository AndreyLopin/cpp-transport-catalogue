#include "json_reader.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport_catalogue {
namespace input {

JsonReader::JsonReader(TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer)
    : catalogue_(catalogue)
    , renderer_(renderer) {
}

void JsonReader::ApplyCommands(void) {
    renderer_.SetSettings(GetRenderSettings());
    AddStops();
    AddDistances();
    AddBuses();
}

void JsonReader::AnswersRequests(std::ostream& out) {
    json::Array doc;
    std::stringstream output;

    for(auto& el : stat_requests_) {
        if(el.AsMap().at("type").AsString() == "Map") {
            doc.emplace_back(PrintMap(el));
        }

        if(el.AsMap().at("type").AsString() == "Bus") {
            doc.emplace_back(PrintBusInfo(el));
        }

        if(el.AsMap().at("type").AsString() == "Stop") {
            doc.emplace_back(PrintStopInfo(el));
        }
    }
    
    json::Print(json::Document(doc), out);
}

void LoadJSON(JsonReader& reader, std::istream& in, std::ostream& out) {
    json::Document doc = json::Load(in);
    const auto load_dict = doc.GetRoot().AsMap();
    reader.SetBaseRequest(load_dict.at("base_requests").AsArray());
    reader.SetStatRequest(load_dict.at("stat_requests").AsArray());
    reader.SetRenderSettings(load_dict.at("render_settings").AsMap());
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
    json::Dict answer;
    std::stringstream output;

    answer.emplace("request_id"s, json::Node{request.AsMap().at("id"s).AsInt()});
    renderer_.RenderMap().Render(output);
    answer.emplace("map"s, json::Node{output.str()});

    return json::Node{answer};
}

json::Node JsonReader::PrintBusInfo(const json::Node& request) {
    using namespace std::string_literals;
    json::Dict answer;

    domain::BusInfo bus = catalogue_.GetBusInfo(request.AsMap().at("name"s).AsString());
    answer.emplace("request_id"s, json::Node{request.AsMap().at("id"s).AsInt()});
    if(bus.name.empty()) {
        answer.emplace("error_message"s, json::Node{"not found"s});
    } else {
        answer.emplace("curvature"s, json::Node{bus.route_curvature});
        answer.emplace("route_length"s, json::Node{bus.route_length});
        answer.emplace("stop_count"s, json::Node{static_cast<int>(bus.count_all_stops)});
        answer.emplace("unique_stop_count"s, json::Node{static_cast<int>(bus.count_unique_stops)});
    }

    return json::Node{answer};
}

json::Node JsonReader::PrintStopInfo(const json::Node& request) {
    using namespace std::string_literals;
    json::Dict answer;
    domain::StopInfo stop = catalogue_.GetStopInfo(request.AsMap().at("name"s).AsString());

    answer.emplace("request_id"s, json::Node{request.AsMap().at("id"s).AsInt()});
    if(stop.name.empty()) {
        answer.emplace("error_message"s, json::Node{"not found"s});
    } else {   
        json::Array buses;
        for(const auto bus : stop.buses) {
            buses.push_back(json::Node{std::string(bus)});
        }
        answer.emplace("buses"s, json::Node{buses});
    }

    return json::Node{answer};
}

void JsonReader::AddStops(void) const {
    // Add all stops
    for(auto& el : base_requests_) {
        if (el.AsMap().at("type").AsString() == "Stop") {
            catalogue_.AddStop(el.AsMap().at("name").AsString(), {el.AsMap().at("latitude").AsDouble(), el.AsMap().at("longitude").AsDouble()});
        }
    }
}

void JsonReader::AddDistances(void) const {
    // Add all distances
    for(const auto& el : base_requests_) {
        if (el.AsMap().at("type").AsString() == "Stop") {
            for(const auto& [key, value] : el.AsMap().at("road_distances").AsMap()) {                
                catalogue_.SetDistanceStops(catalogue_.FindStop(el.AsMap().at("name").AsString()), catalogue_.FindStop(key), value.AsDouble());
            }
        }
    }
}

void JsonReader::AddBuses(void) const {
    // Add all buses
    std::vector<geo::Coordinates> all_coordinates;
    for(const auto& el : base_requests_) {
        if (el.AsMap().at("type").AsString() == "Bus") {
            domain::Bus bus;
            bus.name_ = el.AsMap().at("name").AsString();
            bool is_roundtrip = el.AsMap().at("is_roundtrip").AsBool();
            bus.is_roundtrip_ = is_roundtrip;
            

            for(const auto& stop : el.AsMap().at("stops").AsArray()) {
                bus.stops_.push_back(catalogue_.FindStop(stop.AsString()));
            }

            bus.end_stop_ = bus.stops_.back();

            if(!is_roundtrip) {
                for(int i = static_cast<int>(el.AsMap().at("stops").AsArray().size()) - 2; i >= 0; --i) {
                    bus.stops_.push_back(catalogue_.FindStop(el.AsMap().at("stops").AsArray()[static_cast<uint32_t>(i)].AsString()));
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