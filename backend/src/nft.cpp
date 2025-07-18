#include "../include/header.hpp"


void NFT::displayDetails() const {
    std::cout << "Token ID: " << tokenId << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Owner: " << owner << std::endl;
    std::cout << "Price: " << price << " SOL" << std::endl;
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
        std::cout << "NFT successfully listed for " << price << " SOL" << std::endl;
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

void UserAccount::viewOwnedNFTs() {
    if (!currentUser) {
        std::cout << "Please login first" << std::endl;
        return;
    }

    const V<NFT>& ownedNFTs = currentUser->getOwnedNFTs();
    
    if (ownedNFTs.empty()) {
        std::cout << "\n=== Your Owned NFTs ===" << std::endl;
        std::cout << "User: " << currentUser->getName() << " (" << currentUser->getEmail() << ")" << std::endl;
        std::cout << "Wallet Address: " << currentUser->getWalletAddress() << std::endl;
        std::cout << "Total owned NFTs: 0" << std::endl;
        std::cout << "========================\n" << std::endl;
        std::cout << "You don't own any NFTs yet." << std::endl;
        std::cout << "You can:" << std::endl;
        std::cout << "1. Create and mint NFTs (option 8)" << std::endl;
        std::cout << "2. Buy NFTs from the marketplace (option 12)" << std::endl;
        std::cout << "3. View your collections (option 17)" << std::endl;
        return;
    }

    std::cout << "\n=== Your Owned NFTs ===" << std::endl;
    std::cout << "User: " << currentUser->getName() << " (" << currentUser->getEmail() << ")" << std::endl;
    std::cout << "Wallet Address: " << currentUser->getWalletAddress() << std::endl;
    std::cout << "Total owned NFTs: " << ownedNFTs.size() << std::endl;
    std::cout << "========================\n" << std::endl;

    for (size_t i = 0; i < ownedNFTs.size(); i++) {
        const auto& nft = ownedNFTs[i];
        std::cout << "NFT #" << (i + 1) << ":" << std::endl;
        std::cout << "  Token ID: " << nft.getTokenId() << std::endl;
        std::cout << "  Name: " << nft.getName() << std::endl;
        std::cout << "  Owner: " << nft.getOwner() << std::endl;
        std::cout << "  Price: " << nft.getPrice() << " SOL" << std::endl;
        std::cout << "  Status: " << (nft.getIsListed() ? "Listed for sale" : "Not listed") << std::endl;
        std::cout << "  Mint Address: " << nft.getMintAddress() << std::endl;
        std::cout << "  Metadata URI: " << nft.getMetadataUri() << std::endl;
        std::cout << std::endl;
    }

    // Show summary
    int listedCount = 0;
    int notListedCount = 0;
    double totalValue = 0.0;

    for (const auto& nft : ownedNFTs) {
        if (nft.getIsListed()) {
            listedCount++;
        } else {
            notListedCount++;
        }
        totalValue += nft.getPrice();
    }

    std::cout << "=== Summary ===" << std::endl;
    std::cout << "Listed for sale: " << listedCount << " NFTs" << std::endl;
    std::cout << "Not listed: " << notListedCount << " NFTs" << std::endl;
    std::cout << "Total estimated value: " << totalValue << " SOL" << std::endl;
    std::cout << "================\n" << std::endl;
}
