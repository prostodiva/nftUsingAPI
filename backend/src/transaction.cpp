#include "../include/header.hpp"

void Transaction::displayTransaction() const {
    std::cout << "Transaction ID: " << transactionId << std::endl;
    std::cout << "NFT Token ID: " << tokenId << std::endl;
    std::cout << "Seller: " << seller << std::endl;
    std::cout << "Buyer: " << buyer << std::endl;
    std::cout << "Price: " << price << " ETH" << std::endl;
    std::cout << "Time: " << timestamp;
    std::cout << "Status: " << status << std::endl;
}
