#include <iostream>
#include <string>

#include "json_reader.h"
#include "map_renderer.h"

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;
    //transport_router::TransportRouter router;
    transport_catalogue::input::JsonReader reader(catalogue, renderer);
    LoadJSON(reader, cin, cout);

    return 0;
}