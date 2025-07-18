#include "../include/header.hpp"

bool SolanaWallet::connectPhantom() {

	    try {
        // First set to testnet since we know it works
       // std::string config_cmd = "solana config set --url " + TESTNET_URL;
       // system(config_cmd.c_str());
        
        std::string cmd = "solana address";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return false;
        
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            publicKey = std::string(buffer);
            publicKey = publicKey.substr(0, publicKey.find('\n'));
            
          // Verify the connection by checking balance
            std::string balance_cmd = "solana balance";
            FILE* balance_pipe = popen(balance_cmd.c_str(), "r");
            if (balance_pipe) {
                char balance_buffer[128];
                if (fgets(balance_buffer, sizeof(balance_buffer), balance_pipe) != nullptr) {
                    std::string balance_str(balance_buffer);
                    balance = std::stod(balance_str);
                }
                pclose(balance_pipe);
            }
            
            isConnected = true;
            return true;
        }
        pclose(pipe);
        return false;
    } catch (...) {
        return false;
    }
}

bool SolanaWallet::requestAirdrop() {
    try {
        if (!isConnected || publicKey.empty()) {
            std::cout << "Error: Wallet not properly connected. Please connect wallet first (option 14)" << std::endl;
            return false;
        }

        // Check time since last airdrop
        std::time_t currentTime = std::time(nullptr);
        int timeSinceLastAirdrop = difftime(currentTime, lastAirdropTime);
        
        if (timeSinceLastAirdrop < MIN_AIRDROP_INTERVAL) {
            std::cout << "Please wait " << (MIN_AIRDROP_INTERVAL - timeSinceLastAirdrop) 
                     << " seconds before requesting another airdrop" << std::endl;
            return false;
        }

        // Check daily limit
        if (lastAirdropDate == 0 || difftime(currentTime, lastAirdropDate) >= 24*60*60) {
            dailyAirdropCount = 0;
            lastAirdropDate = currentTime;
        }
        
        if (dailyAirdropCount >= MAX_DAILY_AIRDROPS) {
            std::cout << "Daily airdrop limit reached. Please try again tomorrow." << std::endl;
            return false;
        }

        std::cout << "Current balance: " << balance << " SOL" << std::endl;
        std::cout << "Requesting airdrop of " << AIRDROP_AMOUNT << " SOL to " << publicKey << std::endl;

        std::string cmd = "solana airdrop " + std::to_string(AIRDROP_AMOUNT) + 
                         " " + publicKey + " --url " + DEVNET_URL + " 2>&1";
        
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            std::cout << "Error: Failed to execute airdrop command" << std::endl;
            return false;
        }

        char buffer[256];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);

        if (result.find("Signature: ") != std::string::npos) {
            lastAirdropTime = currentTime;
            dailyAirdropCount++;
            balance += AIRDROP_AMOUNT;
            std::cout << "Airdrop successful! " << AIRDROP_AMOUNT << " SOL added to your wallet" << std::endl;
            std::cout << "New balance: " << balance << " SOL" << std::endl;
            std::cout << "Remaining airdrops today: " << (MAX_DAILY_AIRDROPS - dailyAirdropCount) << std::endl;
            return true;
        } else {
            if (result.find("Rate limit") != std::string::npos || result.find("rate limit") != std::string::npos) {
                std::cout << "Rate limit reached. Please wait at least " 
                         << MIN_AIRDROP_INTERVAL << " seconds before trying again." << std::endl;
                std::cout << "Alternative: Try using a different RPC endpoint or wait a few minutes." << std::endl;
            } else if (result.find("airdrop request failed") != std::string::npos) {
                std::cout << "Airdrop request failed. This can happen when:" << std::endl;
                std::cout << "1. Rate limit is reached (try again in a few minutes)" << std::endl;
                std::cout << "2. Network is congested" << std::endl;
                std::cout << "3. RPC endpoint is temporarily unavailable" << std::endl;
                std::cout << "Please try again later or use a different RPC endpoint." << std::endl;
            } else {
                std::cout << "Airdrop failed: " << result << std::endl;
            }
            return false;
        }

    } catch (const std::exception& e) {
        std::cout << "Error in requestAirdrop: " << e.what() << std::endl;
        return false;
    }
}

std::string SolanaWallet::getPublicKey() const {
    return publicKey;
}

double SolanaWallet::getBalance() const {
    return balance;
}
