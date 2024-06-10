#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    transport_catalogue::input::ReadFromStream(cin, catalogue);
    transport_catalogue::output::ReadFromStreamAndWriteToStream(catalogue, cin, cout);
    return 0;
}