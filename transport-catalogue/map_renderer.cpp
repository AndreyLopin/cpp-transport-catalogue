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

void MapRenderer::SetSettings(const RenderSettings& settings) {
    render_settings_ = settings;
}

void MapRenderer::AddRoute(domain::Bus* bus) {
    RouteSVG route;
    for(const auto& stop : bus->stops) {
        route.stops_.push_back(stop->coordinates);
    }
    route.name_ = bus->name;
    if(route.stops_.size() > 0) {
        routes_.push_back(route);
    }
}

void MapRenderer::SetSphereProjector(std::vector<geo::Coordinates>& coordinates) {
    projector_ = SphereProjector(coordinates.begin(), coordinates.end(), 
                                render_settings_.width_, render_settings_.height_,
                                render_settings_.padding_);
}

svg::Document MapRenderer::RenderMap() {
    svg::Document result;

    std::sort(routes_.begin(), routes_.end(), compare_route_svg);
    
    for(const auto& route : routes_) {
        svg::Polyline line;
        for(const auto& point : route.stops_) {
            line.AddPoint(projector_(point));
        }

        line.SetStrokeColor(NextColor())
            .SetFillColor(svg::NoneColor)            
            .SetStrokeWidth(render_settings_.line_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        result.Add(line);
    }

    return result;
}

}; //namespace map_renderer