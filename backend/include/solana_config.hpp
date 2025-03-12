/*
 * Solana configuration settings
 */


#ifndef SOLANA_CONFIG_HPP
#define SOLANA_CONFIG_HPP

#include <string>

namespace SolanaConfig {
    // Network selection
    #ifdef DEVELOPMENT
        const std::string NETWORK_URL = "https://api.devnet.solana.com";
    #else
        const std::string NETWORK_URL = "https://api.mainnet-beta.solana.com";
    #endif

    // Program IDs
    const std::string NFT_PROGRAM_ID = "metaqbxxUerdq28cj1RbAWkYQm3ybzjb6a8bt518x1s";
    const std::string MARKETPLACE_PROGRAM_ID = "your_program_id_here";

    // Fee settings
    const double MIN_SOL_BALANCE = 0.05;
    const double MINT_FEE = 0.012;
}

#endif
