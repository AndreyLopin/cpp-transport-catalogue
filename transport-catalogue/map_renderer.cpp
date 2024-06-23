#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
namespace map_renderer {

void MapRenderer::SetSettings(const RenderSettings& settings) {
    render_settings_ = settings;
}

void MapRenderer::AddRoute(const domain::Bus* bus) {
    RouteSVG route;
    std::vector<geo::Coordinates> coordinates;
    for(const auto& stop : bus->stops) {
        coordinates.push_back(stop->coordinates);
    }
}

void MapRenderer::SetSphereProjector(std::vector<geo::Coordinates>& coordinates) {
    projector_ = SphereProjector(coordinates.begin(), coordinates.end(), 
                                render_settings_.width_, render_settings_.height_,
                                render_settings_.padding_);
}

svg::Document MapRenderer::RenderMap() const {
    svg::Document result;
    
    return result;
}

}; //namespace map_renderer