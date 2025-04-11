#include "../include/header.hpp"
#include <crow.h>
#include <thread>
#include <chrono>

int main() {
    try {
        std::cout << "Starting NFT Marketplace API server..." << std::endl;

        // Initialize the marketplace
        Marketplace* marketplace = Marketplace::getInstance();

        // Start the API server
        startApiServer();

        delete marketplace;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}