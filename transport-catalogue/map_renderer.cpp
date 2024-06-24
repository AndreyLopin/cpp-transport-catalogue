#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
namespace map_renderer {

bool compare_route_svg(const RouteSVG& lhs, const RouteSVG& rhs) {
    return lhs.name_ < rhs.name_;
}

bool compare_stop_svg(const StopSVG& lhs, const StopSVG& rhs) {
    return lhs.name_ < rhs.name_;
}

void MapRenderer::SetSettings(const RenderSettings& settings) {
    render_settings_ = settings;
}

void MapRenderer::AddRoute(domain::Bus* bus) {
    RouteSVG route;
    for(const auto& stop : bus->stops_) {
        StopSVG add_stop;
        add_stop.name_ = stop->name;
        add_stop.coordinates_ = stop->coordinates;
        route.stops_.push_back(add_stop);
    }

    route.end_stop_.name_ = bus->end_stop_->name;
    route.end_stop_.coordinates_ = bus->end_stop_->coordinates;

    route.name_ = bus->name_;
    route.is_roundtrip_ = bus->is_roundtrip_;
    if(route.stops_.size() > 0) {
        routes_.push_back(route);
    }
}

void MapRenderer::SetSphereProjector(std::vector<geo::Coordinates>& coordinates) {
    projector_ = SphereProjector(coordinates.begin(), coordinates.end(), 
                                render_settings_.width_, render_settings_.height_,
                                render_settings_.padding_);
}

std::vector<svg::Polyline> MapRenderer::GetRoutes() {
    std::vector<svg::Polyline> result;
    for(const auto& route : routes_) {
        svg::Polyline line;
        for(const auto& stop : route.stops_) {
            line.AddPoint(projector_(stop.coordinates_));
        }

        line.SetStrokeColor(NextColor())
            .SetFillColor(svg::NoneColor)            
            .SetStrokeWidth(render_settings_.line_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        result.push_back(std::move(line));
    }
    return result;
}

std::vector<svg::Text> MapRenderer::GetNameRoutes() {
    std::vector<svg::Text> result;

    for(const auto& route : routes_) {
        svg::Text text_underlayer;
        svg::Text text;
        svg::Color color = NextColor();

        text_underlayer.SetPosition(projector_(route.stops_[0].coordinates_))
            .SetOffset(render_settings_.bus_label_offset_)
            .SetFontSize(static_cast<uint32_t>(render_settings_.bus_label_font_size_))
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(std::string(route.name_))
            .SetFillColor(render_settings_.underlayer_color_)
            .SetStrokeColor(render_settings_.underlayer_color_)                
            .SetStrokeWidth(render_settings_.underlayer_width_)                
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                
        text.SetPosition(projector_(route.stops_[0].coordinates_))
            .SetOffset(render_settings_.bus_label_offset_)
            .SetFontSize(static_cast<uint32_t>(render_settings_.bus_label_font_size_))
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(std::string(route.name_))
            .SetFillColor(color);

        result.push_back(text_underlayer);
        result.push_back(text);

        if(!route.is_roundtrip_ && ((route.stops_[0].name_) != route.end_stop_.name_)) {
            svg::Text text_underlayer_end;
            svg::Text text_end;
            text_underlayer_end.SetPosition(projector_(route.end_stop_.coordinates_))
                .SetOffset(render_settings_.bus_label_offset_)
                .SetFontSize(static_cast<uint32_t>(render_settings_.bus_label_font_size_))
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(std::string(route.name_))
                .SetFillColor(render_settings_.underlayer_color_)
                .SetStrokeColor(render_settings_.underlayer_color_)                
                .SetStrokeWidth(render_settings_.underlayer_width_)                
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                
            text_end.SetPosition(projector_(route.end_stop_.coordinates_))
                .SetOffset(render_settings_.bus_label_offset_)
                .SetFontSize(static_cast<uint32_t>(render_settings_.bus_label_font_size_))
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(std::string(route.name_))
                .SetFillColor(color);

            result.push_back(text_underlayer_end);
            result.push_back(text_end);
        }
    }

    return result;
}

std::vector<svg::Circle> MapRenderer::GetStops(const std::set<StopSVG>& stops) {
     std::vector<svg::Circle> result;

    for(const auto& stop : stops) {
        svg::Circle stop_circle;
        stop_circle.SetCenter(projector_(stop.coordinates_))
            .SetRadius(render_settings_.stop_radius_)
            .SetFillColor("white");
        result.push_back(stop_circle);
    }

    return result;
}

std::vector<svg::Text> MapRenderer::GetNameStops(const std::set<StopSVG>& stops) {
    std::vector<svg::Text> result;
    for(const auto& stop : stops) {
        svg::Text text_underloader;
        svg::Text text;
        text_underloader.SetPosition(projector_(stop.coordinates_))
            .SetOffset(render_settings_.stop_label_offset_)
            .SetFontSize(render_settings_.stop_label_font_size_)
            .SetFontFamily("Verdana")
            .SetData(std::string(stop.name_))
            .SetFillColor(render_settings_.underlayer_color_)
            .SetStrokeColor(render_settings_.underlayer_color_)
            .SetStrokeWidth(render_settings_.underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text.SetPosition(projector_(stop.coordinates_))
            .SetOffset(render_settings_.stop_label_offset_)
            .SetFontSize(render_settings_.stop_label_font_size_)
            .SetFontFamily("Verdana")
            .SetData(std::string(stop.name_))
            .SetFillColor("black");

        result.push_back(text_underloader);
        result.push_back(text);
    }
    return result;
}

std::set<StopSVG> MapRenderer::GetStopsSVG() {
    std::set<StopSVG> result;

    for(const auto& route : routes_) {
        for(const auto& stop : route.stops_) {
            result.insert(stop);
        }
    }
    return result;
}

svg::Document MapRenderer::RenderMap() {
    svg::Document result;

    std::sort(routes_.begin(), routes_.end(), compare_route_svg);

    std::set<StopSVG> stops_svg = GetStopsSVG();

    std::vector<svg::Polyline> routes = GetRoutes();
    inst_color_ = 0;
    std::vector<svg::Text> name_routes = GetNameRoutes();

    std::vector<svg::Circle> stops = GetStops(stops_svg);
    std::vector<svg::Text> name_stops = GetNameStops(stops_svg);

    for(int i = 0; i < static_cast<int>(routes.size()); ++i) {
        result.Add(routes[i]);
    }
    for(int i = 0; i < static_cast<int>(name_routes.size()); ++i) {
        result.Add(name_routes[i]);
    }
    for(auto& stop : stops) {
        result.Add(stop);
    }

    for(auto& name_stop : name_stops) {
        result.Add(name_stop);
    }

    return result;
}

}; //namespace map_renderer