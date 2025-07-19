/*
 * Copyright (c) 2024 Margarita Kattsyna
 * NFT Marketplace using Solana technology
 * Licensed under MIT License
 * 
 * This software includes components from:
 * - Solana (Copyright (c) 2021 Solana Foundation) - Apache License 2.0
 * - Metaplex (Copyright (c) 2021 Metaplex Studios) - Apache License 2.0
 * - Crow (MIT License)
 * - Argon2 (Apache License 2.0)
 * - ASIO (Boost Software License)
 */


#ifndef HEADER_HPP
#define HEADER_HPP
#include "V.hpp"
#include "solana_config.hpp"
#include "solana_wallet.hpp"
#include "solana_integration.hpp"
#include <argon2.h>
#include <crow.h>

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <filesystem>

// API Server function declaration
void startApiServer();

class NFT;
class Collection;
class UserAccount;
class Marketplace;

class Transaction {
	private:
		std::string transactionId;
		std::string tokenId;
		std::string seller;
		std::string buyer;
		double price;
		std::string timestamp;
		std::string status;

	public:
		  Transaction() 
        		: transactionId(""), 
          		tokenId(""), 
          		seller(""), 
          		buyer(""), 
          		price(0.0), 
          		timestamp(""), 
          		status("Pending") {}

    		Transaction(std::string tokenId, std::string seller, std::string buyer, 
				double price, std::string status = "Completed") : tokenId(tokenId), seller(seller), buyer(buyer), price(price), status(status) {
        		std::random_device rd;
        		std::mt19937 gen(rd());
        		std::uniform_int_distribution<> dis(0, 0xFFFFFF);
        
        		std::stringstream ss;
        		ss << "TX-" << std::hex << std::uppercase << std::setfill('0') 
           		<< std::setw(6) << dis(gen);
        		transactionId = ss.str();

        		auto now = std::chrono::system_clock::now();
        		auto in_time_t = std::chrono::system_clock::to_time_t(now);
        		timestamp = std::ctime(&in_time_t);
    		}

 		Transaction(const Transaction& other) = default;

    		Transaction& operator=(const Transaction& other) = default;

    		Transaction(Transaction&& other) = default;

    		Transaction& operator=(Transaction&& other) = default;

    		std::string getTransactionId() const { return transactionId; }
    		std::string getTokenId() const { return tokenId; }
    		std::string getSeller() const { return seller; }
    		std::string getBuyer() const { return buyer; }
    		double getPrice() const { return price; }
    		std::string getTimestamp() const { return timestamp; }
    		std::string getStatus() const { return status; }

    		void displayTransaction() const;
};


std::string generateTokenId();

inline std::string generateTokenId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 0xFFFFFF);
    
    std::stringstream ss;
    ss << "NFT-" << std::hex << std::uppercase << std::setfill('0') 
       << std::setw(6) << dis(gen);
    
    return ss.str();
}
class LoginException : public std::exception {
	private:
		std::string message;
	public
		:
		LoginException(const std::string& msg) : message(msg) {}
    		const char* what() const noexcept override {
        	return message.c_str();
    }};


class UserAccount {
    	private:
		 static std::string SOLANA_PATH; 
 		 SolanaWallet wallet;
    		std::string walletAddress;
    		std::string name;
    		std::string email;
    		std::string password;
    		std::string walletBalance;
			std::string passwordHash;
 			std::string keypairPath;
	 		V<std::string> transactionHistory;
			V<NFT> ownedNFTs;
			V<Collection> collections;
			static UserAccount* currentUser;
			static V<UserAccount*> allUsers;

	std::string hashPassword(const std::string& password);
	bool verifyPassword(const std::string& password, const std::string& storedHashData);

    void saveUserData(const std::string& dir) {
        // Save address
        std::string address_path = dir + std::string("/address.txt");
        std::ofstream address_file(address_path);
        if (address_file.is_open()) {
            address_file << walletAddress;
            address_file.close();
        }

        // Save balance
        std::string balance_path = dir + std::string("/balance.txt");
        std::ofstream balance_file(balance_path);
        if (balance_file.is_open()) {
            balance_file << walletBalance;
            balance_file.close();
        }

        // Save user info
        std::string info_path = dir + std::string("/info.json");
        std::ofstream info_file(info_path);
        if (info_file.is_open()) {
            info_file << "{\n";
            info_file << "  \"name\": \"" << name << "\",\n";
            info_file << "  \"email\": \"" << email << "\",\n";
            info_file << "  \"walletAddress\": \"" << walletAddress << "\",\n";
            info_file << "  \"balance\": \"" << walletBalance << "\",\n";
            info_file << "  \"passwordHash\": \"" << passwordHash << "\"\n";
            info_file << "}";
            info_file.close();
        }

        // Initialize transaction history file
        std::string tx_path = dir + std::string("/transactions.txt");
        std::ofstream tx_file(tx_path);
        tx_file.close();
    }



    bool loadUserData(const std::string& email) {
        // Find the user's directory
        std::string cmd = "ls keypairs/";
        std::string dir_name;
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                std::string dir(buffer);
                if (dir.find(email) != std::string::npos) {
                    dir_name = dir;
                    if (!dir_name.empty() && dir_name[dir_name.length()-1] == '\n') {
                        dir_name.erase(dir_name.length()-1);
                    }
                    break;
                }
            }
            pclose(pipe);
        }

        if (dir_name.empty()) {
            return false;
        }

        std::string dir_path = std::string("keypairs/") + dir_name;
        
        // Load address
        std::string address_path = dir_path + std::string("/address.txt");
        std::ifstream address_file(address_path);
        if (address_file.is_open()) {
            std::getline(address_file, walletAddress);
            address_file.close();
        }

        // Load balance
        std::string balance_path = dir_path + std::string("/balance.txt");
        std::ifstream balance_file(balance_path);
        if (balance_file.is_open()) {
            std::getline(balance_file, walletBalance);
            balance_file.close();
        }

        return true;
    }


    static bool checkSolanaInstallation();
    static void installSolanaInstructions();
	public:
    	       UserAccount(std::string walletAddress = "", 
               std::string name = "", 
               std::string email = "",
               std::string password = "", 
               std::string walletBalance = "0", 
               V<std::string> transactionHistory = {});
    
    		~UserAccount();

    		std::string getKeypairPath() const { return keypairPath; }

		static void connectPhantomWallet();
    		static void requestTestSol();
    		static void checkSolBalance();

		static UserAccount* getCurrentUser() { return currentUser; }
		std::string getWalletAddress() const { return walletAddress; }
		double getBalance() const { return std::stod(walletBalance); }
		void updateBalance(double amount) {
			walletBalance = std::to_string(std::stod(walletBalance) + amount);
		}

    		void createAccount(std::vector<UserAccount>& users);
    		static void login(std::vector<UserAccount>& users);
    		static void loadExistingUsers(std::vector<UserAccount>& users);
    		static void logout();

		
			static void viewProfile();
        	static void checkWalletBalance();
        	static void viewTransactionHistory();
        	void addTransaction(const std::string& transactionId) {
            		transactionHistory.push_back(transactionId);
        	}
        	static void createNFTCollection(std::vector<Collection>& collections);
        	static void addNFTToCollection(std::vector<NFT>& nfts);
        	static void viewNFTsCollection();
        	static void viewAllCollections();
        	static void viewOwnedNFTs();

		
 		const V<Collection>& getCollections() const {
        		return collections;
    		}

    		V<Collection>& getCollections() {
        		return collections;
    		}

		bool connectToSolana() {
        		return wallet.connectPhantom();
    		}

    		double getSolanaBalance() const {
        		return SolanaIntegration::getBalance(walletAddress);
    		}

    		static void setAllUsers(V<UserAccount*>& users) {
        		allUsers = users;
    		}
    		static const V<UserAccount*>& getAllUsers() {
        		return allUsers;
    		}

    		static UserAccount* findUserByWallet(const std::string& walletAddress) {
        		if (allUsers.size() == 0) return nullptr;
        
        		for (UserAccount* user : allUsers) {
            			if (user && user->getWalletAddress() == walletAddress) {
                			return user;
            			}
        		}
        		return nullptr;
    		}

		 SolanaWallet& getWallet() { return wallet; }
		 
		 // Marketplace integration methods
		 V<NFT>& getOwnedNFTs() { return ownedNFTs; }
		 void setOwnedNFTs(const V<NFT>& nfts) { ownedNFTs = nfts; }
		 std::string getName() const { return name; }
		 std::string getEmail() const { return email; }
		 void saveCollections(const std::string& dir);
		 void loadCollections(const std::string& dir);
};


class NFT {
	private:
		std::string tokenId;
		std::string name;
		std::string owner;
		double price;
		bool isListed;

		std::string mintAddress;
		std::string metadataUri;
	public:
		NFT() : tokenId(""), name(""), owner(""), price(0.0), isListed(false) {}

		NFT(std::string name, std::string owner, double price, bool isListed = false, std::string metadata = "") 
        		: tokenId(generateTokenId()), name(name), owner(owner), price(price), isListed(isListed), metadataUri(metadata) {}

		// Constructor with explicit tokenId (for loading from file)
		NFT(std::string tokenId, std::string name, std::string owner, double price, bool isListed = false, std::string metadata = "") 
        		: tokenId(tokenId), name(name), owner(owner), price(price), isListed(isListed), metadataUri(metadata) {}

		// Copy constructor to preserve all data
		NFT(const NFT& other) : tokenId(other.tokenId), name(other.name), owner(other.owner), 
			price(other.price), isListed(other.isListed), mintAddress(other.mintAddress), metadataUri(other.metadataUri) {}

		// Assignment operator
		NFT& operator=(const NFT& other) {
			if (this != &other) {
				tokenId = other.tokenId;
				name = other.name;
				owner = other.owner;
				price = other.price;
				isListed = other.isListed;
				mintAddress = other.mintAddress;
				metadataUri = other.metadataUri;
			}
			return *this;
		}

 		~NFT() = default;

		bool mintOnSolana() {
			 try {
            			std::string newMintAddress = SolanaIntegration::mintNFT(metadataUri);
            			if (!newMintAddress.empty()) {
                			mintAddress = newMintAddress;
                			return true;
            			}
            			return false;
        		} catch (...) {
            			return false;	
			}
		}

		std::string getMintAddress() const {
    			return mintAddress;
		}

		std::string getTokenId() const {
			return tokenId;
		}

		std::string getName() const {
			return name;
		}

		std::string getOwner() const {
			return owner;
		}

		double getPrice() const {
			return price;
		}

		bool getIsListed() const {
			return isListed;
		}

		void setPrice(double newPrice) {
		       	price = newPrice; 
		}
    		void setOwner(const std::string& newOwner) { 
			owner = newOwner; 
		}
    		void setIsListed(bool listed) { 
			isListed = listed; 
		}
		void setMetadataUri(const std::string& uri) {
			metadataUri = uri;
		}
		std::string getMetadataUri() const {
			return metadataUri;
		}
		void setMintAddress(const std::string& address) {
			mintAddress = address;
		}	

		void listForSale(double newPrice);
		void unlist();

		void displayDetails() const;


};


class Collection {
	private:
		std::string name;
    		std::string creator;
    		V<NFT> nfts;

	public:
		Collection() = default;

    		Collection(std::string name, std::string creator)
        		: name(name),  creator(creator) {}

		Collection(const Collection& other) : name(other.name), creator(other.creator), nfts(other.nfts) {}

		Collection& operator=(const Collection& other) {
        		if (this != &other) {
            			name = other.name;
            			creator = other.creator;
            			nfts = other.nfts;
        		}
        	return *this;
    		}	    
    		std::string getName() const { return name; }
    		std::string getCreator() const { return creator; }
    
    		void addNFT(const NFT& nft) { nfts.push_back(nft); }
    		void displayCollection() const;

		V<NFT>& getNFTs() { return nfts; }
		const V<NFT>& getNFTs() const {return nfts; }
};

class Marketplace {
private:
    V<NFT> listedNFTs;
    V<Transaction> transactionHistory;
    static Marketplace* instance;
    static constexpr double PLATFORM_FEE = 0.025;

    Marketplace() {} 

public:
    Marketplace(const Marketplace&) = delete;
    Marketplace& operator=(const Marketplace&) = delete;

    static Marketplace* getInstance();
    void unlistNFT(const std::string& tokenId);
    void buyNFT(const std::string& tokenId, UserAccount& buyer);
    void recordTransaction(const Transaction& transaction);
    const Transaction* getTransaction(const std::string& transactionId) const;
    void displayListedNFTs() const;
    void displayTransactionHistory() const;
    NFT* findNFTByTokenId(const std::string& tokenId);
    double calculateFee(double price) const { return price * PLATFORM_FEE; }
    bool hasListedNFTs() const { return !listedNFTs.empty(); }
    bool listNFT(NFT& nft, double price);
    void saveMarketplaceData();
    void loadMarketplaceData();
};

void menu(std::vector<UserAccount>& users, std::vector<NFT>& nfts, std::vector<Collection>& collections);



#endif
