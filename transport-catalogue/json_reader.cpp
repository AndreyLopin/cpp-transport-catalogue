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

JsonReader::JsonReader(TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, std::istream& in, std::ostream& out)
    : catalogue_(catalogue)
    , renderer_(renderer) {
    json::Document doc = json::Load(in);
    const auto load_dict = doc.GetRoot().AsMap();
    base_requests_ = load_dict.at("base_requests").AsArray();
    stat_requests_ = load_dict.at("stat_requests").AsArray();
    render_settings_ = load_dict.at("render_settings").AsMap();
    renderer_.SetSettings(GetRenderSettings());
    AddStops();
    AddDistances();
    AddBuses();
    renderer_.RenderMap().Render(out);
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

    /*for(const auto& el : base_requests_) {
        if (el.AsMap().at("type") == "Bus") {
            result.routes_.insert(el.AsMap().at("name").AsString());
        }
    }*/
    
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
        for(int i = 0; i < 3; i++) {
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

void JsonDownload(TransportCatalogue& catalogue, map_renderer::MapRenderer& renderer, std::istream& in, std::ostream& out) {
    JsonReader reader = JsonReader(catalogue, renderer, in, out);
    //reader.ApplyCommands();
    //reader.AnswersRequests(out);
}

void JsonReader::ApplyCommands(void) const {
    AddStops();
    AddDistances();
    AddBuses();
}

void JsonReader::AnswersRequests(std::ostream& out) const {
    //json::Array doc;
    std::stringstream output;
    output << '[';
    for(auto& el : stat_requests_) {
        if(output.str().size() > 1) {
            output << ',';
        }
        output << '{';

        output << "\"request_id\": " << el.AsMap().at("id").AsInt() << '\n';

        if(el.AsMap().at("type").AsString() == "Bus") {
            domain::BusInfo bus = catalogue_.GetBusInfo(el.AsMap().at("name").AsString());
            output << ',';
            if(bus.name.empty()) {
                output << "\"error_message\": \"not found\"";
            } else {
                output << "\"curvature\": " << bus.route_curvature;
                output << ',' << "\"route_length\": " << bus.route_length;
                output << ',' << "\"stop_count\": " << bus.count_all_stops;
                output << ',' << "\"unique_stop_count\": " << bus.count_unique_stops;
            }
        }

        if(el.AsMap().at("type").AsString() == "Stop") {
            domain::StopInfo stop = catalogue_.GetStopInfo(el.AsMap().at("name").AsString());
            output << ',';
            if(stop.name.empty()) {
                output << "\"error_message\": \"not found\"";
            } else {               
                if(stop.buses.empty()) {
                    output << "\"buses\": ";
                    output << '[';
                    output << ']';
                } else {
                    output << "\"buses\": ";
                    output << '[';
                    bool is_not_first = false;
                    for(const auto bus : stop.buses) {
                        if(is_not_first) {
                            output << ',';
                        }
                        output << "\"" << bus << "\"";
                        is_not_first = true;
                    }
                    output << ']';
                }
            }
        }
        output << '}';
    }

    output << ']';
    
    json::Print(LoadJSON(output.str()), out);
    //out << output.str();
}

json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

/*std::string PrintBusInfo(const domain::BusInfo& bus_info) {
    std::stringstream output;
    using namespace std::string_literals;
    if (!bus_info.name.empty()) {
        int current_precesion = static_cast<int>(output.precision());
        std::setprecision(6);
        output << ": "s << bus_info.count_all_stops << " stops on route, "s;
        output << bus_info.count_unique_stops << " unique stops, " << bus_info.route_length << " route length, "
                << bus_info.route_curvature << " curvature\n";
        std::setprecision(current_precesion);
    } else {
        output << ": not found\n"s;
    }
    return output.str();
}

std::string PrintStopInfo(const domain::StopInfo& stop_info) {
    using namespace std::string_literals;
    std::stringstream output;
    if (!stop_info.name.empty()) {
        if (stop_info.buses.empty()) {
            output << ": no buses\n"s;
        } else {
            output << ": buses";
            for (const auto& bus : stop_info.buses) {
                output << ' ' << bus;
            }
            output << '\n';
        }
    } else {
        output << "\"request_id\": " << 
    }
    return output.str();
}*/

void JsonReader::AddStops(void) const {
    // Add all stops
    for(auto& el : base_requests_) {
        if (el.AsMap().at("type") == "Stop") {
            catalogue_.AddStop(el.AsMap().at("name").AsString(), {el.AsMap().at("latitude").AsDouble(), el.AsMap().at("longitude").AsDouble()});
        }
    }
}

void JsonReader::AddDistances(void) const {
    // Add all distances
    for(const auto& el : base_requests_) {
        if (el.AsMap().at("type") == "Stop") {
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
        if (el.AsMap().at("type") == "Bus") {
            domain::Bus bus;
            bus.name_ = el.AsMap().at("name").AsString();
            bool is_roundtrip = el.AsMap().at("is_roundtrip").AsBool();
            bus.is_roundtrip_ = is_roundtrip;
            

            for(const auto& stop : el.AsMap().at("stops").AsArray()) {
                bus.stops_.push_back(catalogue_.FindStop(stop.AsString()));
            }

            bus.end_stop_ = bus.stops_.back();

            if(!is_roundtrip) {
                for(int i = el.AsMap().at("stops").AsArray().size() - 2; i >= 0; --i) {
                    bus.stops_.push_back(catalogue_.FindStop(el.AsMap().at("stops").AsArray()[i].AsString()));
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