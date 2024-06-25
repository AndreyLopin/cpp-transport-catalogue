#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<< (std::ostream& out, const StrokeLineCap& line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            out << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"s;
            break;
    }
    return out;
}

std::ostream& operator << (std::ostream& out, const StrokeLineJoin& line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"s;
            break;
    }
    return out;
}

std::ostream& operator<< (std::ostream& out, const Color& color) {
    visit(ColorPrinter{out}, color);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out_ << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out_;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out_);
    out << "/>"sv;
}

// ------------- Polyline -----------------
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out_;
    out << "<polyline points=\""sv;
    bool is_not_first = false;
    for(const Point& p : points_) {
        if(is_not_first) {
            out << ' ';
        }
        is_not_first = true;
        out << p.x << ',' << p.y;        
    }
    out << '\"';
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out_);
    out << "/>"sv;
}

//-------------- Text ------------------------
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}


Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out_;
    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y;
    out << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y;
    out << "\" font-size=\""sv << font_size_ << "\" "sv;
    if(!font_family_.empty()) {
        out << "font-family=\""sv << font_family_ << "\" "sv;
    }
    if(!font_weight_.empty()) {
        out << "font-weight=\""sv << font_weight_ << "\" "sv;
    }

    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out_);

    out << '>';

    for(const char c : data_) {
        if (c == '"') {
            out << "&quot;";
        } else if (c == '\'') {
            out << "&apos;";
        } else if (c == '<') {
            out << "&lt;";
        } else if (c == '>') {
            out << "&gt;";
        } else if (c == '&') {
            out << "&amp;";
        } else {
            out << c;
        }
    }
    out << "</text>"sv;
}

// ----------- Document -------------------
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    for (const auto& el : objects_) {
        el->Render(out);
    }

    out << "</svg>"sv;
}

}  // namespace svg