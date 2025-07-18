#include "../include/header.hpp"
#include "../include/solana_config.hpp"
#include "../include/solana_integration.hpp"

Marketplace* Marketplace::instance = nullptr;

Marketplace* Marketplace::getInstance() {
    if (!instance) {
        instance = new Marketplace();
    }
    return instance;
}

bool Marketplace::listNFT(NFT& nft, double price) {
    try {
        if (nft.getOwner() != UserAccount::getCurrentUser()->getWalletAddress()) {
            throw std::runtime_error("You can only list NFTs that you own");
        }   

        // Check if NFT is already listed
        if (nft.getIsListed()) {
            throw std::runtime_error("NFT is already listed for sale");
        }

        // Set price and mark as listed
        nft.setPrice(price);
        nft.setIsListed(true);
        
        // Keep the original owner (seller) - don't transfer to marketplace
        // The NFT stays owned by the seller but is listed for sale
        listedNFTs.push_back(nft);
        
        // Update the original NFT in the user's collection
        UserAccount* currentUser = UserAccount::getCurrentUser();
        if (currentUser) {
            // Update in user's owned NFTs
            for (auto& userNFT : currentUser->getOwnedNFTs()) {
                if (userNFT.getTokenId() == nft.getTokenId()) {
                    userNFT.setIsListed(true);
                    userNFT.setPrice(price);
                    break;
                }
            }
            
            // Update in user's collections
            for (auto& collection : currentUser->getCollections()) {
                for (auto& collectionNFT : collection.getNFTs()) {
                    if (collectionNFT.getTokenId() == nft.getTokenId()) {
                        collectionNFT.setIsListed(true);
                        collectionNFT.setPrice(price);
                        break;
                    }
                }
            }
            
            // Save updated user data
            std::string safe_email = currentUser->getEmail();
            std::replace(safe_email.begin(), safe_email.end(), '@', '_');
            std::replace(safe_email.begin(), safe_email.end(), '.', '_');
            std::string keypair_dir = "keypairs/" + currentUser->getName() + "_" + safe_email;
            currentUser->saveCollections(keypair_dir);
        }
        
        // Save marketplace data to disk
        saveMarketplaceData();
        
        std::cout << "NFT " << nft.getTokenId() << " listed successfully at " << price << " SOL" << std::endl;
        std::cout << "Note: This is a local marketplace listing. For real Solana marketplace integration," << std::endl;
        std::cout << "you would need to deploy a marketplace program and use proper Solana instructions." << std::endl;
        
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error listing: " << e.what() << std::endl;
        return false;
    }
}

void Marketplace::buyNFT(const std::string& tokenId, UserAccount& buyer) {
    try {
        // Find the NFT first
        NFT* nftToBuy = nullptr;
        size_t nftIndex = 0;
        
        for (size_t i = 0; i < listedNFTs.size(); i++) {
            if (listedNFTs[i].getTokenId() == tokenId) {
                nftToBuy = &listedNFTs[i];
                nftIndex = i;
                break;
            }
        }

        if (!nftToBuy) {
            throw std::runtime_error("NFT not found");
        }

        double price = nftToBuy->getPrice();
        double platformFee = calculateFee(price);
        double totalCost = price + platformFee;
        std::string seller = nftToBuy->getOwner();

        // Add Solana balance check (including platform fee) - use devnet
        std::string balance_cmd = "solana balance " + buyer.getWalletAddress() + " --url https://api.devnet.solana.com";
        FILE* balance_pipe = popen(balance_cmd.c_str(), "r");
        double currentBalance = 0.0;
        if (balance_pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), balance_pipe) != nullptr) {
                std::string balance_str(buffer);
                // Remove " SOL" suffix and newline
                size_t sol_pos = balance_str.find(" SOL");
                if (sol_pos != std::string::npos) {
                    balance_str = balance_str.substr(0, sol_pos);
                }
                if (!balance_str.empty() && balance_str.back() == '\n') {
                    balance_str.pop_back();
                }
                try {
                    currentBalance = std::stod(balance_str);
                } catch (const std::exception& e) {
                    std::cout << "Failed to parse balance: " << e.what() << std::endl;
                }
            }
            pclose(balance_pipe);
        }
        
        if (currentBalance < totalCost) {
            throw std::runtime_error("Insufficient SOL balance. Need " + std::to_string(totalCost) + " SOL (price: " + std::to_string(price) + " SOL + fee: " + std::to_string(platformFee) + " SOL), but have " + std::to_string(currentBalance) + " SOL");
        }

        // Update ownership
        nftToBuy->setOwner(buyer.getWalletAddress());
        nftToBuy->setIsListed(false);

        // Update balances
        if (UserAccount* sellerAccount = UserAccount::findUserByWallet(seller)) {
            sellerAccount->updateBalance(price); // Seller gets full price
            
            // Save seller's updated balance to disk
            std::string seller_safe_email = sellerAccount->getEmail();
            std::replace(seller_safe_email.begin(), seller_safe_email.end(), '@', '_');
            std::replace(seller_safe_email.begin(), seller_safe_email.end(), '.', '_');
            std::string seller_keypair_dir = "keypairs/" + sellerAccount->getName() + "_" + seller_safe_email;
            
            // Save balance to balance.txt file
            std::string balance_path = seller_keypair_dir + "/balance.txt";
            std::ofstream balance_file(balance_path);
            if (balance_file.is_open()) {
                balance_file << sellerAccount->getBalance();
                balance_file.close();
            }
            
            // Save updated user info to info.json
            std::string info_path = seller_keypair_dir + "/info.json";
            std::ofstream info_file(info_path);
            if (info_file.is_open()) {
                info_file << "{\n";
                info_file << "  \"name\": \"" << sellerAccount->getName() << "\",\n";
                info_file << "  \"email\": \"" << sellerAccount->getEmail() << "\",\n";
                info_file << "  \"walletAddress\": \"" << sellerAccount->getWalletAddress() << "\",\n";
                info_file << "  \"balance\": \"" << sellerAccount->getBalance() << "\"\n";
                info_file << "}";
                info_file.close();
            }
        }
        buyer.updateBalance(-totalCost); // Buyer pays price + platform fee
        
        // Save buyer's updated balance to disk
        std::string buyer_safe_email = buyer.getEmail();
        std::replace(buyer_safe_email.begin(), buyer_safe_email.end(), '@', '_');
        std::replace(buyer_safe_email.begin(), buyer_safe_email.end(), '.', '_');
        std::string buyer_keypair_dir = "keypairs/" + buyer.getName() + "_" + buyer_safe_email;
        
        // Save balance to balance.txt file
        std::string buyer_balance_path = buyer_keypair_dir + "/balance.txt";
        std::ofstream buyer_balance_file(buyer_balance_path);
        if (buyer_balance_file.is_open()) {
            buyer_balance_file << buyer.getBalance();
            buyer_balance_file.close();
        }
        
        // Save updated user info to info.json
        std::string buyer_info_path = buyer_keypair_dir + "/info.json";
        std::ofstream buyer_info_file(buyer_info_path);
        if (buyer_info_file.is_open()) {
            buyer_info_file << "{\n";
            buyer_info_file << "  \"name\": \"" << buyer.getName() << "\",\n";
            buyer_info_file << "  \"email\": \"" << buyer.getEmail() << "\",\n";
            buyer_info_file << "  \"walletAddress\": \"" << buyer.getWalletAddress() << "\",\n";
            buyer_info_file << "  \"balance\": \"" << buyer.getBalance() << "\"\n";
            buyer_info_file << "}";
            buyer_info_file.close();
        }

        // Record transaction
        Transaction tx(tokenId, seller, buyer.getWalletAddress(), price);
        recordTransaction(tx);
        buyer.addTransaction(tx.getTransactionId());

        // Remove from listings
        V<NFT> updatedListings;
        for (size_t i = 0; i < listedNFTs.size(); i++) {
            if (i != nftIndex) {
                updatedListings.push_back(listedNFTs[i]);
            }
        }
        listedNFTs = updatedListings;

        // Update user collections
        // Remove from seller's collections
        std::cout << "DEBUG: Looking for seller account with wallet: " << seller << std::endl;
        std::cout << "DEBUG: Available users:" << std::endl;
        for (UserAccount* user : UserAccount::getAllUsers()) {
            if (user) {
                std::cout << "  - " << user->getName() << " (" << user->getEmail() << "): " << user->getWalletAddress() << std::endl;
            }
        }
        
        if (UserAccount* sellerAccount = UserAccount::findUserByWallet(seller)) {
            std::cout << "DEBUG: Found seller account: " << sellerAccount->getName() << std::endl;
            // Remove from seller's owned NFTs
            std::cout << "DEBUG: Removing NFT " << tokenId << " from seller's owned NFTs (had " << sellerAccount->getOwnedNFTs().size() << " NFTs)" << std::endl;
            V<NFT> updatedOwnedNFTs;
            for (const auto& userNFT : sellerAccount->getOwnedNFTs()) {
                if (userNFT.getTokenId() != tokenId) {
                    updatedOwnedNFTs.push_back(userNFT);
                } else {
                    std::cout << "DEBUG: Found and removing NFT " << tokenId << " from owned NFTs" << std::endl;
                }
            }
            sellerAccount->setOwnedNFTs(updatedOwnedNFTs);
            std::cout << "DEBUG: Seller now has " << sellerAccount->getOwnedNFTs().size() << " owned NFTs" << std::endl;
            
            // Remove from seller's collections
            std::cout << "DEBUG: Removing NFT " << tokenId << " from seller's collections" << std::endl;
            for (auto& collection : sellerAccount->getCollections()) {
                std::cout << "DEBUG: Processing collection: " << collection.getName() << " (has " << collection.getNFTs().size() << " NFTs)" << std::endl;
                V<NFT> updatedCollectionNFTs;
                for (const auto& collectionNFT : collection.getNFTs()) {
                    if (collectionNFT.getTokenId() != tokenId) {
                        updatedCollectionNFTs.push_back(collectionNFT);
                    } else {
                        std::cout << "DEBUG: Found and removing NFT " << tokenId << " from collection " << collection.getName() << std::endl;
                    }
                }
                collection.getNFTs() = updatedCollectionNFTs;
                std::cout << "DEBUG: Collection " << collection.getName() << " now has " << collection.getNFTs().size() << " NFTs" << std::endl;
            }
            
            // Save seller's updated data
            std::string seller_safe_email = sellerAccount->getEmail();
            std::replace(seller_safe_email.begin(), seller_safe_email.end(), '@', '_');
            std::replace(seller_safe_email.begin(), seller_safe_email.end(), '.', '_');
            std::string seller_keypair_dir = "keypairs/" + sellerAccount->getName() + "_" + seller_safe_email;
            sellerAccount->saveCollections(seller_keypair_dir);
        } else {
            std::cout << "DEBUG: WARNING - Seller account not found for wallet: " << seller << std::endl;
            std::cout << "DEBUG: This means the NFT removal from seller's collections was skipped!" << std::endl;
        }
        
        // Add to buyer's collections
        NFT boughtNFT = *nftToBuy;
        boughtNFT.setIsListed(false);
        buyer.getOwnedNFTs().push_back(boughtNFT);
        
        // Save buyer's updated collections (using existing buyer_keypair_dir)
        buyer.saveCollections(buyer_keypair_dir);

        // Save marketplace data
        saveMarketplaceData();

        std::cout << "NFT transferred successfully!" << std::endl;
        std::cout << "Transaction Summary:" << std::endl;
        std::cout << "  NFT: " << tokenId << " (" << nftToBuy->getName() << ")" << std::endl;
        std::cout << "  Seller: " << seller << " received " << price << " SOL" << std::endl;
        std::cout << "  Buyer: " << buyer.getWalletAddress() << " paid " << totalCost << " SOL (price: " << price << " SOL + fee: " << platformFee << " SOL)" << std::endl;
        
        // Final state summary
        if (UserAccount* sellerAccount = UserAccount::findUserByWallet(seller)) {
            std::cout << "\nFinal State:" << std::endl;
            std::cout << "  Seller (" << sellerAccount->getName() << "):" << std::endl;
            std::cout << "    - Owned NFTs: " << sellerAccount->getOwnedNFTs().size() << std::endl;
            std::cout << "    - Collections: " << sellerAccount->getCollections().size() << std::endl;
            for (const auto& collection : sellerAccount->getCollections()) {
                std::cout << "      * " << collection.getName() << ": " << collection.getNFTs().size() << " NFTs" << std::endl;
            }
        }
        std::cout << "  Buyer (" << buyer.getName() << "):" << std::endl;
        std::cout << "    - Owned NFTs: " << buyer.getOwnedNFTs().size() << std::endl;
        std::cout << "    - Collections: " << buyer.getCollections().size() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error buying NFT: " << e.what() << std::endl;
        throw;
    }
}

void Marketplace::displayListedNFTs() const {
    try {
        if (listedNFTs.empty()) {
            std::cout << "\nNo NFTs currently listed on the marketplace." << std::endl;
            return;
        }

        std::cout << "\n NFTs Available for Purchase: " << std::endl;
        std::cout << "Total listings: " << listedNFTs.size() << std::endl;

        int counter = 1;
        for (const auto& nft : listedNFTs) {
            std::cout << "\nListing #" << counter++ << std::endl;
            std::cout << "Token ID: " << nft.getTokenId() << std::endl;
            std::cout << "Name: " << nft.getName() << std::endl;
            std::cout << "Seller: " << nft.getOwner() << std::endl;
            std::cout << "Price: " << nft.getPrice() << " SOL" << std::endl;
            std::cout << "Platform Fee: " << calculateFee(nft.getPrice()) << " SOL" << std::endl;
            std::cout << "Total Cost: " << (nft.getPrice() + calculateFee(nft.getPrice())) << " SOL" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error displaying marketplace listings: " << e.what() << std::endl;
    }
}



void Marketplace::unlistNFT(const std::string& tokenId) {
    try {
        bool found = false;
        V<NFT> newListedNFTs;
        
        for (auto& nft : listedNFTs) {
            if (nft.getTokenId() == tokenId) {
                nft.setIsListed(false);
                found = true;
            } else {
                newListedNFTs.push_back(nft);
            }
        }
        
        if (!found) {
            throw std::runtime_error("NFT not found");
        }
        
        listedNFTs = newListedNFTs;
        std::cout << "NFT unlisted successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error unlisting NFT: " << e.what() << std::endl;
        throw;
    }
}

NFT* Marketplace::findNFTByTokenId(const std::string& tokenId) {
    	for (auto& nft : listedNFTs) {
        	if (nft.getTokenId() == tokenId) {
            		return &nft;
        	}
    	}
    	return nullptr;
}


void Marketplace::displayTransactionHistory() const {
    try {
        if (transactionHistory.empty()) {
            std::cout << "\nNo transactions recorded." << std::endl;
            return;
        }

        for (const auto& tx : transactionHistory) {
            tx.displayTransaction();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error displaying transaction history: " << e.what() << std::endl;
    }
}

void Marketplace::recordTransaction(const Transaction& transaction) {
    transactionHistory.push_back(transaction);
}

const Transaction* Marketplace::getTransaction(const std::string& transactionId) const {
    for (const auto& tx : transactionHistory) {
        if (tx.getTransactionId() == transactionId) {
            return &tx;
        }
    }
    return nullptr;
}

void Marketplace::saveMarketplaceData() {
    try {
        // Create marketplace directory if it doesn't exist
        std::string mkdir_cmd = "mkdir -p marketplace";
        system(mkdir_cmd.c_str());

        // Save listed NFTs
        std::string listings_path = "marketplace/listings.json";
        std::ofstream listings_file(listings_path);
        if (listings_file.is_open()) {
            listings_file << "{\n";
            listings_file << "  \"listings\": [\n";
            
            for (size_t i = 0; i < listedNFTs.size(); i++) {
                const auto& nft = listedNFTs[i];
                listings_file << "    {\n";
                listings_file << "      \"tokenId\": \"" << nft.getTokenId() << "\",\n";
                listings_file << "      \"name\": \"" << nft.getName() << "\",\n";
                listings_file << "      \"owner\": \"" << nft.getOwner() << "\",\n";
                listings_file << "      \"price\": " << nft.getPrice() << ",\n";
                listings_file << "      \"isListed\": " << (nft.getIsListed() ? "true" : "false") << ",\n";
                listings_file << "      \"mintAddress\": \"" << nft.getMintAddress() << "\",\n";
                listings_file << "      \"metadataUri\": \"" << nft.getMetadataUri() << "\"\n";
                listings_file << "    }";
                if (i < listedNFTs.size() - 1) listings_file << ",";
                listings_file << "\n";
            }
            
            listings_file << "  ]\n";
            listings_file << "}";
            listings_file.close();
        }

        // Save transaction history
        std::string transactions_path = "marketplace/transactions.json";
        std::ofstream transactions_file(transactions_path);
        if (transactions_file.is_open()) {
            transactions_file << "{\n";
            transactions_file << "  \"transactions\": [\n";
            
            for (size_t i = 0; i < transactionHistory.size(); i++) {
                const auto& tx = transactionHistory[i];
                transactions_file << "    {\n";
                transactions_file << "      \"transactionId\": \"" << tx.getTransactionId() << "\",\n";
                transactions_file << "      \"tokenId\": \"" << tx.getTokenId() << "\",\n";
                transactions_file << "      \"seller\": \"" << tx.getSeller() << "\",\n";
                transactions_file << "      \"buyer\": \"" << tx.getBuyer() << "\",\n";
                transactions_file << "      \"price\": " << tx.getPrice() << ",\n";
                transactions_file << "      \"timestamp\": \"" << tx.getTimestamp() << "\",\n";
                transactions_file << "      \"status\": \"" << tx.getStatus() << "\"\n";
                transactions_file << "    }";
                if (i < transactionHistory.size() - 1) transactions_file << ",";
                transactions_file << "\n";
            }
            
            transactions_file << "  ]\n";
            transactions_file << "}";
            transactions_file.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving marketplace data: " << e.what() << std::endl;
    }
}

void Marketplace::loadMarketplaceData() {
    try {
        // Load listed NFTs
        std::string listings_path = "marketplace/listings.json";
        std::ifstream listings_file(listings_path);
        if (listings_file.is_open()) {
            std::string line;
            bool inListings = false;
            bool inNFT = false;
            
            std::string tokenId, name, owner, mintAddress, metadataUri;
            double price = 0.0;
            bool isListed = false;

            while (std::getline(listings_file, line)) {
                // Remove whitespace
                line.erase(0, line.find_first_not_of(" \t"));
                if (line.empty()) continue;

                if (line.find("\"listings\":") != std::string::npos) {
                    inListings = true;
                } else if (line.find("\"tokenId\":") != std::string::npos && inNFT) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        tokenId = line.substr(start + 1, end - start - 1);
                    }
                } else if (line.find("\"name\":") != std::string::npos && inNFT) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        name = line.substr(start + 1, end - start - 1);
                    }
                } else if (line.find("\"owner\":") != std::string::npos && inNFT) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        owner = line.substr(start + 1, end - start - 1);
                    }
                } else if (line.find("\"price\":") != std::string::npos && inNFT) {
                    size_t colonPos = line.find(":");
                    if (colonPos != std::string::npos) {
                        std::string priceStr = line.substr(colonPos + 1);
                        if (!priceStr.empty() && priceStr.back() == ',') {
                            priceStr.pop_back();
                        }
                        price = std::stod(priceStr);
                    }
                } else if (line.find("\"isListed\":") != std::string::npos && inNFT) {
                    if (line.find("true") != std::string::npos) {
                        isListed = true;
                    }
                } else if (line.find("\"mintAddress\":") != std::string::npos && inNFT) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        mintAddress = line.substr(start + 1, end - start - 1);
                    }
                } else if (line.find("\"metadataUri\":") != std::string::npos && inNFT) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        metadataUri = line.substr(start + 1, end - start - 1);
                    }
                } else if (line.find("{") != std::string::npos && inListings && !inNFT) {
                    inNFT = true;
                    tokenId = "";
                    name = "";
                    owner = "";
                    price = 0.0;
                    isListed = false;
                    mintAddress = "";
                    metadataUri = "";
                } else if (line.find("}") != std::string::npos && inNFT) {
                    // Complete NFT
                    NFT nft(name, owner, price, isListed, metadataUri);
                    listedNFTs.push_back(nft);
                    inNFT = false;
                }
            }
            listings_file.close();
        }

        // Load transaction history (simplified - just count for now)
        std::string transactions_path = "marketplace/transactions.json";
        std::ifstream transactions_file(transactions_path);
        if (transactions_file.is_open()) {
            // For now, just acknowledge that transactions exist
            // Full transaction loading can be implemented later
            transactions_file.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading marketplace data: " << e.what() << std::endl;
    }
}
