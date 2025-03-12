#include "../include/header.hpp"

void Collection::displayCollection() const {
	try {

    		std::cout << "\nCollection: " << name<< std::endl;
    		std::cout << "Creator: " <<creator << std::endl;
    		std::cout << "\nNFTs in collection:" << std::endl;
    
    		if (nfts.empty()) {
        		std::cout << "No NFTs in this collection." << std::endl;
    		} else {
        		for (const auto& nft : nfts) {
            		nft.displayDetails();
        		}
    		}
	} catch (const std::exception& e) {
		std::cerr<<"Error"<<e.what()<<std::endl;
	}
}
