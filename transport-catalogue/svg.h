#pragma once

#include <cstdint>
#include <variant>
#include <vector>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

namespace svg {

struct Rgb {
    Rgb (uint8_t r = 0, uint8_t g = 0, uint8_t b = 0)
        : red(r)
        , green(g)
        , blue(b) {
        };
    
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct Rgba {
    Rgba (uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, double a = 1.0)
        : red(r)
        , green(g)
        , blue(b) {
            if (a >= 0 && a <= 1.0) {
                opacity = a;
            } else {
                opacity = 1.0;
            }
        };
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    double opacity;
};
    
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{std::string("none")};
    
struct ColorPrinter {
    std::ostream& out;

    void operator()(std::monostate) const {
        using namespace std::literals;
        out << "none"sv;
    }
    void operator()(std::string color) const {
        out << color;
    }
    void operator()(Rgb color) {
        using namespace std::literals;
        out << "rgb("sv << std::to_string(color.red) << ',' << std::to_string(color.green) << ',' << std::to_string(color.blue) << ')';
    }
    void operator()(Rgba color) {
        using namespace std::literals;
        out << "rgba("sv << std::to_string(color.red) << ',' << std::to_string(color.green) << ',' << std::to_string(color.blue) << ',' << color.opacity << ')';
    }
};

// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, что она будет одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы
//inline const Color NoneColor{"none"};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<< (std::ostream& out, const StrokeLineCap& line_cap);
std::ostream& operator<< (std::ostream& out, const StrokeLineJoin& line_join);
std::ostream& operator<< (std::ostream& out, const Color& color);

struct Point {
    Point() = default;
    Point(double point_x, double point_y)
        : x(point_x)
        , y(point_y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out_(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out_(out)
        , indent_step_(indent_step)
        , indent_(indent) {
    }

    RenderContext Indented() const {
        return {out_, indent_step_, indent_ + indent_step_};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent_; ++i) {
            out_.put(' ');
        }
    }

    std::ostream& out_;
    int indent_step_ = 0;
    int indent_ = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width) {
        width_ = width;
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (width_) {
            out << " stroke-width=\""sv << *width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    void RenderObject(const RenderContext& context) const override;

    Point pos_ = {0.0, 0.0};
    Point offset_ = {0.0, 0.0};
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

// Интерфейс ObjectContainer предоставляет методы для рисования графических примитивов
class ObjectContainer {
public:
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    virtual ~ObjectContainer() = default;
};

class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    template <typename Obj>
    void Add(Obj obj) {
        objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
    }

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }
    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document
private:
    std::vector<std::unique_ptr<Object>> objects_;
};

// Интерфейс Drawable задаёт объекты, которые можно нарисовать с помощью ObjectContainer
class Drawable {
public:
    virtual void Draw(ObjectContainer& g) const = 0;
    
    virtual ~Drawable() = default;
};

}  // namespace svg