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

void UserAccount::viewAllCollections() {
    if (!currentUser) {
        std::cout << "Please login first\n" << std::endl;
        return;
    }

    if (currentUser->collections.empty()) {
        std::cout << "\nNo collections found. Create a collection first!\n" << std::endl;
        return;
    }

    std::cout << "\n=== Your Collections ===" << std::endl;
    std::cout << "User: " << currentUser->name << " (" << currentUser->email << ")" << std::endl;
    std::cout << "Total collections: " << currentUser->collections.size() << std::endl;
    std::cout << "========================\n" << std::endl;

    for (size_t i = 0; i < currentUser->collections.size(); i++) {
        const auto& collection = currentUser->collections[i];
        std::cout << "Collection " << (i + 1) << ":" << std::endl;
        std::cout << "  Name: " << collection.getName() << std::endl;
        std::cout << "  Creator: " << collection.getCreator() << std::endl;
        std::cout << "  NFTs: " << collection.getNFTs().size() << std::endl;
        
        if (!collection.getNFTs().empty()) {
            std::cout << "  NFT List:" << std::endl;
            for (const auto& nft : collection.getNFTs()) {
                std::cout << "    - " << nft.getName() << " (ID: " << nft.getTokenId() << ", Price: " << nft.getPrice() << " SOL)" << std::endl;
            }
        }
        std::cout << std::endl;
    }
}
