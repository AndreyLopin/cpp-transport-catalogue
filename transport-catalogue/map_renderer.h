#pragma once

#include <algorithm>
#include <vector>

#include "svg.h"
#include "geo.h"
#include "domain.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
namespace map_renderer {

inline const double EPSILON = 1e-6;

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    SphereProjector() = default;

    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RenderSettings {
    double width_ = 0;
    double height_ = 0;

    double padding_ = 0;

    double line_width_ = 0;
    double stop_radius_ = 0;

    double bus_label_font_size_ = 0.0;
    svg::Point bus_label_offset_ = {0.0, 0.0};

    double stop_label_font_size_ = 0.0;
    svg::Point stop_label_offset_ = {0.0, 0.0};

    svg::Color underlayer_color_;
    double underlayer_width_ = 0.0;

    std::vector<svg::Color> color_palette_;
};

struct RouteSVG {
    RouteSVG() = default;

    /*RouteSVG(const RouteSVG& other)
        : color_(other.color_)
        , name_(other.name_)
        , stops_(other.stops_) {
    };

    RouteSVG(const RouteSVG&& other) {
        name_ = std::move(other.name_);
        stops_ = std::move(other.stops_);
        color_ = std::move(other.color_);
    }

    RouteSVG& operator=(RouteSVG&& other) {
        if(&other != this) {
            name_ = std::move(other.name_);
            stops_ = std::move(other.stops_);
            color_ = std::move(other.color_);
        }

        return *this;
    }

    void swap(RouteSVG& other) noexcept {
        std::swap(name_, other.name_);
        std::swap(color_, other.color_);
        std::swap(stops_, other.stops_);
    }

    RouteSVG& operator=(const RouteSVG& other) {
        RouteSVG copy(other);
        swap(copy);
        return *this;
    }*/

    std::string_view name_;
    std::vector<geo::Coordinates> stops_;
    svg::Color color_;
};

class MapRenderer {
public:
    MapRenderer() = default;

    MapRenderer(const RenderSettings& settings)
        : render_settings_(settings) {};

    void AddRoute(domain::Bus* bus);
    void SetSettings(const RenderSettings& settings);

    svg::Document RenderMap();

    void SetSphereProjector(std::vector<geo::Coordinates>& coordinates);
private:
    svg::Color NextColor(void) {
        if(inst_color_ >= render_settings_.color_palette_.size()) {
            inst_color_ = 0;
        }
        return render_settings_.color_palette_[inst_color_++];
    }
    RenderSettings render_settings_;
    uint32_t inst_color_ = 0;
    std::vector<RouteSVG> routes_;
    SphereProjector projector_;
};

}; //map_renderer