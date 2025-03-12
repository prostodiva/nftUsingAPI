#include "../include/header.hpp"


void NFT::displayDetails() const {
    std::cout << "Token ID: " << tokenId << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Owner: " << owner << std::endl;
    std::cout << "Price: " << price << " ETH" << std::endl;
    std::cout << "Status: " << (isListed ? "Listed for sale" : "Not listed") << std::endl;
}

void NFT::listForSale(double newPrice) {
    try {
        if (newPrice <= 0) {
            throw std::runtime_error("Price must be greater than 0");
        }
        if (isListed) {
            throw std::runtime_error("NFT is already listed for sale");
        }
        price = newPrice;
        isListed = true;
        std::cout << "NFT successfully listed for " << price << " ETH" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error listing NFT: " << e.what() << std::endl;
        throw;
    }
}

void NFT::unlist() {
    try {
        if (!isListed) {
            throw std::runtime_error("NFT is not currently listed");
        }
        isListed = false;
        std::cout << "NFT successfully unlisted" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error unlisting NFT: " << e.what() << std::endl;
        throw;
    }

 }
