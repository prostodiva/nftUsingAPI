/*
 * Copyright (c) 2024 Margarita Kattsyna
 * Solana integration for NFT Marketplace
 */


#ifndef SOLANA_INTEGRATION_HPP
#define SOLANA_INTEGRATION_HPP

#include <string>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <fstream>
#include "solana_config.hpp"

class SolanaIntegration {
public:
    // Only define this once
    static constexpr double LAMPORTS_PER_SOL = 1000000000.0;
    
    static bool connectWallet() {
        std::string cmd = "solana address";
        return system(cmd.c_str()) == 0;
    }

    static bool disconnectWallet() {
        return true;  // Placeholder for now
    }

    static std::string getPublicKey() {
        FILE* pipe = popen("solana address", "r");
        if (!pipe) return "";
        
        char buffer[128];
        std::string result;
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result = buffer;
            if (!result.empty() && result[result.length()-1] == '\n') {
                result.erase(result.length()-1);
            }
        }
        pclose(pipe);
        return result;
    }
    
    static std::string mintNFT(const std::string& metadata) {
        // Create keypair for the NFT
        std::string cmd = "solana-keygen new --no-bip39-passphrase -o nft-keypair.json";
        if (system(cmd.c_str()) != 0) return "";
        
        // Get mint address
        FILE* pipe = popen("solana address -k nft-keypair.json", "r");
        char buffer[128];
        std::string mintAddress;
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            mintAddress = buffer;
            if (!mintAddress.empty() && mintAddress[mintAddress.length()-1] == '\n') {
                mintAddress.erase(mintAddress.length()-1);
            }
        }
        pclose(pipe);
        
        // Create metadata JSON file if metadata is provided
        if (!metadata.empty()) {
            std::string metadataFile = "nft-metadata.json";
            std::ofstream file(metadataFile);
            if (file.is_open()) {
                file << metadata;
                file.close();
                
                // TODO: Upload metadata to IPFS or similar service
                // TODO: Use the metadata URI in the actual minting command
                std::cout << "Metadata saved to " << metadataFile << std::endl;
            }
        }
        
        return mintAddress;
    }
    
    static bool transferNFT(const std::string& to, const std::string& mint) {
        std::string cmd = "spl-token transfer " + mint + " 1 " + to + " --url devnet";
        return system(cmd.c_str()) == 0;
    }
    
    static bool sendTransaction(const std::string& signature) {
        return system(("solana confirm " + signature).c_str()) == 0;
    }

    static double getBalance(const std::string& address) {
        std::string cmd = "solana balance " + address;
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return 0.0;
        
        char buffer[128];
        double balance = 0.0;
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            balance = std::stod(buffer);
        }
        pclose(pipe);
        return balance;
    }

    static void airdropDevnet(const std::string& address) {
        std::string cmd = "solana airdrop 1 " + address + " --url devnet";
        system(cmd.c_str());
    }
    
    static std::string createMetadataJSON(const std::string& name, const std::string& description, const std::string& imageUrl) {
        return "{\n"
               "  \"name\": \"" + name + "\",\n"
               "  \"description\": \"" + description + "\",\n"
               "  \"image\": \"" + imageUrl + "\",\n"
               "  \"attributes\": []\n"
               "}";
    }
};

#endif
