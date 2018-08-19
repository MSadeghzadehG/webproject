//
// main_https.cpp
// web_server
// created by changkun at changkun.de/modern-cpp
//
#include <iostream>
#include "server.https.hpp"
#include "handler.hpp"
using namespace LabexWeb;

int main() {
    // HTTPS server runs in port 12345, enable 4 threads
    // Use certificates for security
    Server<HTTPS> server(12345, 4, "server.crt", "server.key");
    std::cout << "Server starting at port: 12345" << std::endl;
    start_server<Server<HTTPS>>(server);
    return 0;
}
