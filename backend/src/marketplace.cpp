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

        // Add Solana listing
        std::cout << "Creating Solana listing..." << std::endl;
        std::string cmd = "solana program call --program-id " + 
                         SolanaConfig::MARKETPLACE_PROGRAM_ID +  // Added SolanaConfig::
                         " list_token " + nft.getMintAddress() + 
                         " --url devnet";
        
        if (system(cmd.c_str()) == 0) {
            nft.setPrice(price);
            nft.setIsListed(true);
            listedNFTs.push_back(nft);
            std::cout << "NFT " << nft.getTokenId() << " listed successfully at " << price << " ETH" << std::endl;
            return true;
        } else {
            throw std::runtime_error("Failed to list NFT on Solana");
        }

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
        std::string seller = nftToBuy->getOwner();

        // Add Solana balance check
        if (buyer.getSolanaBalance() < price) {
            throw std::runtime_error("Insufficient SOL balance");
        }

        // Add Solana transfer
        std::cout << "Executing Solana transfer..." << std::endl;
        if (SolanaIntegration::transferNFT(buyer.getWalletAddress(), nftToBuy->getMintAddress())) {
            // Update ownership
            nftToBuy->setOwner(buyer.getWalletAddress());
            nftToBuy->setIsListed(false);

            // Update balances
            if (UserAccount* sellerAccount = UserAccount::findUserByWallet(seller)) {
                sellerAccount->updateBalance(price * (1 - PLATFORM_FEE));
            }
            buyer.updateBalance(-price);

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

            std::cout << "NFT transferred successfully on Solana devnet!" << std::endl;
        } else {
            throw std::runtime_error("Failed to transfer NFT on Solana");
        }
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
            std::cout << "Price: " << nft.getPrice() << " ETH" << std::endl;
            std::cout << "Platform Fee: " << calculateFee(nft.getPrice()) << " ETH" << std::endl;
            std::cout << "Total Cost: " << (nft.getPrice() + calculateFee(nft.getPrice())) << " ETH" << std::endl;
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
