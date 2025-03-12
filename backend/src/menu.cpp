#include "../include/header.hpp"

void menu(std::vector<UserAccount>& users, std::vector<NFT>& nfts, std::vector<Collection>& collections) {

    	int choice = 0;
    	Marketplace* marketplace = Marketplace::getInstance();
		V<UserAccount*> userPointers;
    	for (auto& user : users) {
        	userPointers.push_back(&user);
    	}
    	UserAccount::setAllUsers(userPointers);

	std::cout << "Welcome to the NFT store!\n"<<std::endl; 

    	do {
		std::cout<<"1 - create a new account\n"
                	"2 - login to an existing account\n"
                	"3 - logout\n"
                	"4 - view profile\n"
                	"5 - check wallet balance\n"
                	"6 - view transaction history\n"
                	"7 - create a new NFT collection\n"
                	"8 - create and add NFTs to the collection\n"
                	"9 - view the NFTs in the collection\n"
                	"10 - list NFT for sale\n"
					"11 - View marketplace listings\n"
                	"12 - buy NFT\n"
                	"13 - exit\n" 
					"14 - connect Phanton Wallet\n"
					"15 - Request test sol\n"
					"16 - check sol balance\n"<<std::endl;
    
	if (!(std::cin>>choice))  {
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout<<"Invalid input"<<std::endl;
		continue;
	}	

    switch (choice) {
        case 1: {
                UserAccount newUser("","","","","0", V<std::string>());
                newUser.createAccount(users);
                break;
        }
        case 2: 
            UserAccount::login(users);
            break;
        case 3:
            UserAccount::logout();
            break;
        case 4:
            UserAccount::viewProfile();
            break;
        case 5:
            UserAccount::checkWalletBalance();
            break;
        case 6:
            UserAccount::viewTransactionHistory();
            break;
        case 7:
	    UserAccount::createNFTCollection(collections);
             break;
        case 8:
	     UserAccount::addNFTToCollection(nfts, collections);
             break;
        case 9:
	     UserAccount::viewNFTsCollection();
             break;
        case 10: {
	     if (!UserAccount::getCurrentUser()) {
		     std::cout<<"Login first\n"<<std::endl;
		     break;
	     }

	      	std::cout << "\nYour NFTs:" << std::endl;
    	     	bool hasNFTs = false;

		for (const auto& collection : UserAccount::getCurrentUser()->getCollections()) {
        		std::cout << "Checking collection: " << collection.getName() << std::endl;  // Debug line
        		const V<NFT>& collectionNFTs = collection.getNFTs();
        			for (const auto& nft : collectionNFTs) {
            				if (!nft.getIsListed()) {
                				nft.displayDetails();
                				hasNFTs = true;
            				}
        			}
    		}


		if (!hasNFTs) {
        		std::cout << "You don't have any NFTs to list" << std::endl;
        		break;
    		}	

	     std::string tokenId;
	     double price;
	     std::cout<<"Enter NFT token ID to list: ";
	     std::cin>>tokenId;
	     std::cout<<"Enter price (ETH): ";
	     std::cin>>price;

		try {
        		bool found = false;
        		for (auto& collection : UserAccount::getCurrentUser()->getCollections()) {
            			V<NFT>& collectionNFTs = collection.getNFTs();
            			for (auto& nft : collectionNFTs) {
                			if (nft.getTokenId() == tokenId) {
                    				marketplace->listNFT(nft, price);
                    				found = true;
                    				std::cout << "NFT listed successfully!" << std::endl;
                    				break;
                			}
            			}
            			if (found) break;
        			}
        
        		if (!found) {
            			std::cout << "NFT not found with token ID: " << tokenId << std::endl;
        		}
    		} catch (const std::exception& e) {
        		std::cout << "Error: " << e.what() << std::endl;
    		}
    		break;
}
		
	 case 11: {
		marketplace->displayListedNFTs();
		break;
		  }
         case 12: {
		if (!UserAccount::getCurrentUser()) {
                    std::cout << "Please login first" << std::endl;
                    break;
                }
                std::string tokenId;
                std::cout << "Enter NFT token ID to buy: ";
                std::cin >> tokenId;
                try {
                    marketplace->buyNFT(tokenId, *UserAccount::getCurrentUser());
                } catch (const std::exception& e) {
                    std::cout << "Error: " << e.what() << std::endl;
                }
                break;
            }
        case 13:
            std::cout << "Exiting the program..." << std::endl;
            break;

	 case 14:

	 	if (!UserAccount::getCurrentUser()) {
		     std::cout<<"Login first\n"<<std::endl;
		     break;
	     	}

        	if (UserAccount::getCurrentUser()) {
            		if (UserAccount::getCurrentUser()->connectToSolana()) {  
                		std::cout << "Wallet connected successfully!" << std::endl;
            		}
        	}
        break;
    case 15: {

		if (!UserAccount::getCurrentUser()) {
		     std::cout<<"Login and connect Phantom wallet first\n"<<std::endl;
		     break;
	     }

           	 if (UserAccount::getCurrentUser() == nullptr) {
        		std::cout << "Please login first" << std::endl;
       		 break;
    		}
    
   		 if (UserAccount::getCurrentUser()->getWallet().requestAirdrop()) {
        // Success message handled by requestAirdrop()
   		 }
    break;
	}
	case 16:
        if (UserAccount::getCurrentUser()) {
            double balance = SolanaIntegration::getBalance(UserAccount::getCurrentUser()->getWalletAddress());
            std::cout << "SOL Balance: " << balance << std::endl;
        }
        break;

        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
            break;
    		}

	} while (choice != 13);


}
