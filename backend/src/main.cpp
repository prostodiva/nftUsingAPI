#include "../include/header.hpp"
#include <crow.h>
#include <thread>
#include <chrono>

int main() {
    try {
        std::vector<UserAccount>users;
        std::vector<NFT> nfts;
        std::vector<Collection> collections;

        // Load existing users from keypairs directory
        std::cout << "Main: About to load existing users..." << std::endl;
        UserAccount::loadExistingUsers(users);
        std::cout << "Main: Finished loading users. Total users: " << users.size() << std::endl;

        std::cout << "Starting NFT Marketplace API server..." << std::endl;

        // Initialize the marketplace
        Marketplace* marketplace = Marketplace::getInstance();

        // Start the API server
        std::cout << "Starting API server on port 3000..." << std::endl;
        std::thread api_thread([]() {
            startApiServer();
        });
        api_thread.detach(); // Let it run independently

        // Give the server a moment to start
        std::this_thread::sleep_for(std::chrono::seconds(1));

        menu(users, nfts, collections);

        delete marketplace;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}


