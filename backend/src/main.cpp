#include "../include/header.hpp"
#include <crow.h>
#include <thread>
#include <chrono>

int main() {
    try {
        std::cout << "Starting NFT Marketplace API server..." << std::endl;

        crow::SimpleApp app;

        // Initialize the marketplace
        Marketplace* marketplace = Marketplace::getInstance();

        // Define API routes with CORS headers
        CROW_ROUTE(app, "/")([](){
            crow::response res;
            crow::json::wvalue response_data;
            response_data["status"] = "success";
            response_data["message"] = "NFT Marketplace API Server";
            
            res.write(response_data.dump());
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
            res.add_header("Access-Control-Allow-Headers", "*");
            res.set_header("Content-Type", "application/json");
            
            return res;
        });

        // Handle OPTIONS requests for CORS preflight
        CROW_ROUTE(app, "/<path>").methods("OPTIONS"_method)([](const crow::request&, const std::string&) {
            crow::response res;
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
            res.add_header("Access-Control-Allow-Headers", "*");
            return res;
        });

        // Start the server
        app.port(8080).run();

        delete marketplace;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}