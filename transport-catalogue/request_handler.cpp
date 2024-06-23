#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Возвращает информацию о маршруте (запрос Bus)
std::optional<domain::Bus> RequestHandler::GetBusStat(const std::string_view& bus_name) const {

}

// Возвращает маршруты, проходящие через
/*const std::unordered_set<BusPtr>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {

}*/

// Этот метод будет нужен в следующей части итогового проекта
svg::Document RequestHandler::RenderMap() const {
    svg::Document result;
    
    return result;
}