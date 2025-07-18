#ifndef SOLANA_WALLET_HPP
#define SOLANA_WALLET_HPP

#include <string>
#include <cstdlib>
#include <iostream>
#include "solana_config.hpp"
#include <ctime>

class SolanaWallet {
private:
    std::string publicKey;
    double balance;
    bool isConnected;

    const std::string DEVNET_URL = "https://api.devnet.solana.com";
    const double AIRDROP_AMOUNT = 1.0;

    time_t lastAirdropTime;
    const int MIN_AIRDROP_INTERVAL = 20;  
    int dailyAirdropCount;
    time_t lastAirdropDate;
    const int MAX_DAILY_AIRDROPS = 2;

public:
    SolanaWallet() : balance(0.0), isConnected(false), lastAirdropTime(0), dailyAirdropCount(0), lastAirdropDate(0) {}

    bool connectPhantom();
    std::string getPublicKey() const;
    double getBalance() const;
    bool requestAirdrop();
    void setBalance(double newBalance) {balance = newBalance; }
	bool updateBalance() {
    try {
        std::string cmd = "solana balance";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return false;
        
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string balance_str(buffer);
            if (!balance_str.empty() && balance_str[balance_str.length()-1] == '\n') {
                balance_str.erase(balance_str.length()-1);
            }
            balance = std::stod(balance_str);
            pclose(pipe);
            return true;
        }
        pclose(pipe);
        return false;
    } catch (...) {
        return false;
    }
}

};

#endif
