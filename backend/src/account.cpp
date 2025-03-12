#include "../include/header.hpp"
#include <fstream>

UserAccount* UserAccount::currentUser = nullptr;
V<UserAccount*> UserAccount::allUsers;
std::string UserAccount::SOLANA_PATH = "";

UserAccount::UserAccount(std::string walletAddress, std::string name, std::string email,
			std::string password, std::string walletBalance, V<std::string> transactionHistory) {
	this->walletAddress = walletAddress;
	this->name = name;
	this->email = email;
	this->password = password;
	this->walletBalance = walletBalance;
	this->transactionHistory = transactionHistory;
}

UserAccount::~UserAccount() {

}

/*
 * hashPassword(); veryfyPassword() was made using Argon2.
 */
std::string UserAccount::hashPassword(const std::string& password) {
    const uint32_t t_cost = 2;            // 2 iterations
    const uint32_t m_cost = 1 << 16;      // 64 MiB memory cost
    const uint32_t parallelism = 1;       // 1 thread
    const uint32_t hash_length = 32;      // 32-byte hash
    const uint32_t salt_length = 16;      // 16-byte salt

    // Generate random salt
    std::vector<uint8_t> salt(salt_length);
    std::random_device rd;
    std::generate(salt.begin(), salt.end(), std::ref(rd));

    // Allocate memory for the hash
    std::vector<uint8_t> hash(hash_length);

    // Perform the hash
    int result = argon2id_hash_raw(
        t_cost,
        m_cost,
        parallelism,
        password.c_str(),
        password.length(),
        salt.data(),
        salt_length,
        hash.data(),
        hash_length
    );

    if (result != ARGON2_OK) {
        throw std::runtime_error("Error hashing password: " + std::string(argon2_error_message(result)));
    }

    // Combine salt and hash into a single string
    std::string combined;
    combined.reserve(salt_length + hash_length);
    combined.append(reinterpret_cast<char*>(salt.data()), salt_length);
    combined.append(reinterpret_cast<char*>(hash.data()), hash_length);

    return combined;
}

bool UserAccount::verifyPassword(const std::string& password, const std::string& stored) {
    const uint32_t t_cost = 2;
    const uint32_t m_cost = 1 << 16;
    const uint32_t parallelism = 1;
    const uint32_t hash_length = 32;
    const uint32_t salt_length = 16;

    // Extract salt from stored string
    std::vector<uint8_t> salt(salt_length);
    std::copy_n(stored.begin(), salt_length, salt.begin());

    // Compute hash with same parameters
    std::vector<uint8_t> computed_hash(hash_length);
    int result = argon2id_hash_raw(
        t_cost,
        m_cost,
        parallelism,
        password.c_str(),
        password.length(),
        salt.data(),
        salt_length,
        computed_hash.data(),
        hash_length
    );

    if (result != ARGON2_OK) {
        throw std::runtime_error("Error verifying password: " + std::string(argon2_error_message(result)));
    }

    // Compare computed hash with stored hash
    return std::equal(
        computed_hash.begin(),
        computed_hash.end(),
        stored.begin() + salt_length
    );
}



void UserAccount::createAccount(std::vector<UserAccount>& users) {
    try {

 	if (!checkSolanaInstallation()) {
            installSolanaInstructions();
            throw std::runtime_error("Solana CLI tools not installed");
        }        
	
	std::cout << "Enter name: ";
        std::cin >> name;
        std::cout << "Enter email: ";
        std::cin >> email;
        std::cout << "Enter password: ";
        std::cin >> password;

        for(const auto& user : users) {
            if(user.email == email) {
                throw std::runtime_error("Email already exists\n");
            }
        }

        walletBalance = "0";
        transactionHistory = {};

    	passwordHash = hashPassword(password);

        // Sanitize both name and email for directory name
        std::string safe_name = name;
        std::string safe_email = email;
        std::replace(safe_email.begin(), safe_email.end(), '@', '_');
        std::replace(safe_email.begin(), safe_email.end(), '.', '_');
        std::string keypair_dir = "keypairs/" + safe_name + "_" + safe_email;

        // Create fresh directory
        std::string mkdir_cmd = "mkdir -p " + keypair_dir;
        system(mkdir_cmd.c_str());

        // Create a valid Solana keypair file
        std::string keypair_path = keypair_dir + "/id.json";

        // Generate new keypair
    	std::string keygen_cmd = SOLANA_PATH + "solana-keygen new --no-bip39-passphrase --force -o " + keypair_path;
		if (system(keygen_cmd.c_str()) != 0) {
            throw std::runtime_error("Failed to generate keypair");
        }

        // Set Solana configuration
        std::string config_cmd = SOLANA_PATH + "solana config set --url https://api.testnet.solana.com --keypair " + keypair_path;
        system(config_cmd.c_str());

	// Get the actual wallet address from the keypair
        std::string address_cmd = SOLANA_PATH + "solana address -k " + keypair_path;
        FILE* pipe = popen(address_cmd.c_str(), "r");
		if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                walletAddress = std::string(buffer);
					if (!walletAddress.empty() && walletAddress[walletAddress.length()-1] == '\n') {
						walletAddress.erase(walletAddress.length()-1);
					}
            }
            pclose(pipe);
        }

        // Save user data
        saveUserData(keypair_dir);

        users.push_back(*this);
        std::cout << "Account created successfully!" << std::endl;
        std::cout << "Wallet Address: " << walletAddress << std::endl;

    } catch(const std::exception& e) {
        std::cerr << "Error creating account: " << e.what() << std::endl;
    }
}

void UserAccount::login(std::vector<UserAccount>& users) {
	std::string inputEmail, inputPassword;

	try {
		if(currentUser) {
			throw LoginException("A user is already logged in\n");
		}
		std::cout << "Enter email: ";
		std::cin >> inputEmail;
		std::cout << "Enter password: ";
		std::cin >> inputPassword;

		if (inputEmail.empty() || inputPassword.empty()) {
			throw LoginException("The field cannot be empty");
		}

		for(auto& user : users) {
			if(user.email == inputEmail) {
				// Set the currentUser pointer when login is successful
				currentUser = &user;
				std::cout << "Login successful! Welcome, " << user.name << std::endl;
				return;
			}
		}
		throw LoginException("Invalid email");
	} catch (LoginException& e) {
		std::cerr << "Login Error: " << e.what() << std::endl;
	}
}

/*
 * added the code below to trace if the blockchain Solana is working correctly:
 */
bool UserAccount::checkSolanaInstallation() {
    // Check for solana CLI only
    std::string solana_check = "which solana";
    if (system(solana_check.c_str()) != 0) {
        return false;
    }

    // Verify it's working by checking version
    std::string version_check = "solana --version";
    if (system(version_check.c_str()) != 0) {
        return false;
    }

    return true;
}

void UserAccount::installSolanaInstructions() {
    std::cout << "\nSolana CLI tools are not completely installed. Please follow these steps:" << std::endl;
    std::cout << "\n1. Install Rust and Cargo if not already installed:" << std::endl;
    std::cout << "   curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh" << std::endl;
    std::cout << "   source $HOME/.cargo/env" << std::endl;
    
    std::cout << "\n2. Install Solana CLI tools:" << std::endl;
    std::cout << "   cargo install solana-cli" << std::endl;
    
    std::cout << "\n3. Verify installation:" << std::endl;
    std::cout << "   solana --version" << std::endl;
    
    std::cout << "\nAfter installation, please restart your terminal and this program." << std::endl;
}


void UserAccount::viewProfile() {
	if (!currentUser) {
		std::cout <<"Login first\n"<<std::endl;
		return;
	}

 	std::cout<<"Wallet Address: " <<currentUser->walletAddress<<std::endl;
	std::cout<<"Name: " <<currentUser->name<<std::endl;
	std::cout<<"Email: " <<currentUser->email<<std::endl;
	std::cout<<"Wallet Balance: "<<currentUser->walletBalance<<std::endl;
}


void UserAccount::checkWalletBalance() {
	try {
		if (!currentUser) {
		std::cout <<"Login first\n"<<std::endl;
		return;
	}
	std::cout<<"Address: "<<currentUser->walletAddress<<std::endl;
	std::cout<<"Balance: "<<currentUser->walletBalance<<std::endl;

	} catch(const std::exception& e) {
		std::cerr<<"Error: "<<e.what()<<std::endl;
	}
}

void UserAccount::logout() {
	try {
		if (!currentUser) {
			throw LoginException("Login first");
		}

		std::string userName = currentUser->name;
		currentUser = nullptr;
		std::cout<<userName<<" Logged out successfully"<<std::endl;
	} catch (const LoginException& e) {
		std::cerr <<"Logout error" <<e.what()<<std::endl;
	}
}

void UserAccount::viewTransactionHistory() {

	if (!currentUser) {
		std::cout <<"Login first\n"<<std::endl;
		return;
	}

	if (currentUser->transactionHistory.empty()) {
		std::cout<<"No transaction found"<<std::endl;
		return;
	}

	for (const auto& txId: currentUser->transactionHistory) {
		std::cout<<"transaction ID: "<<txId<<std::endl;
	}

}

void UserAccount::createNFTCollection(std::vector<Collection>& collections) {

	if (!currentUser) {
		std::cout <<"Login first\n"<<std::endl;
		return;
	}

	try {
		std::string collectionName;
		
		std::cout<<"Enter collection name: "<<std::endl;
		std::cin.ignore();
		std::getline(std::cin, collectionName);

		Collection newCollection(collectionName, currentUser->name);

		currentUser->collections.push_back(newCollection);
		collections.push_back(newCollection);

		std::cout<<"NFT collection created successfully!"<< std::endl;
		std::cout <<"Collection Name: "<< collectionName<< std::endl;
        	std::cout <<"Creator "<< currentUser->name<< std::endl;

    	} catch (const std::exception& e) {
        	std::cerr<<"Error creating collection: "<< e.what()<< std::endl;
    }
}



void UserAccount::addNFTToCollection(std::vector<NFT>& nfts, std::vector<Collection>& collections) {
	
	if (!currentUser) {
		std::cout <<"Login first\n"<<std::endl;
		return;
	}

	if (currentUser->collections.empty()) {
		std::cout<<"Create a collection first\n"<<std::endl;
		return;
	}

// Add Solana balance check
    	if (SolanaIntegration::getBalance(currentUser->getWalletAddress()) < 0.05) {
        std::cout << "Insufficient SOL for minting. Need at least 0.05 SOL" << std::endl;
        std::cout << "Would you like to request test SOL? (y/n): ";
        char choice;
        std::cin >> choice;
        if (choice == 'y') {
            SolanaIntegration::airdropDevnet(currentUser->getWalletAddress());
            std::cout << "SOL added to your wallet" << std::endl;
        } else {
            return;
        }
    }

	try {
		for (const auto& collection : currentUser->collections) {
			std::cout<<collection.getName()<<std::endl;
		}

		std::string collectionName;
		std::cout<<"\nEnter collection name to add NFT: ";
		std::cin.ignore();
		std::getline(std::cin, collectionName);
		
		Collection* targetCollection = nullptr;
        	for (auto& collection : currentUser->collections) {
            		if (collection.getName() == collectionName) {
                		targetCollection = &collection;
                		break;
            		}
       		}

        
        	if (!targetCollection) {
            		throw std::runtime_error("Collection not found");
        	}	

		std::string nftName;
        	double price;
        
        	std::cout<<"Enter NFT name: ";
        	std::getline(std::cin, nftName);
        
        	std::cout<<"Enter NFT price: ";
        	std::cin>>price;

        	if (price < 0) {
            		throw std::runtime_error("Price cannot be negative");
        	}
		
		NFT newNFT(nftName, currentUser->walletAddress, price);

 // Add Solana minting
        	if (newNFT.mintOnSolana()) {
            		std::cout << "NFT minted on Solana devnet" << std::endl;
			targetCollection->addNFT(newNFT);
			nfts.push_back(newNFT);
			currentUser->ownedNFTs.push_back(newNFT);
		} else {
           		 throw std::runtime_error("Failed to mint NFT on Solana");
        	}			
		

 		std::cout<<"\nNFT added successfully!" << std::endl;
        	std::cout<<"Token ID: " << newNFT.getTokenId() << std::endl;

    	} catch (const std::exception& e) {
        	std::cerr << "Error adding NFT: " << e.what() << std::endl;
    	}

}


void UserAccount::viewNFTsCollection() {
    if (!currentUser) {
        std::cout << "Please login first\n" << std::endl;
        return;
    }

    if (currentUser->collections.empty()) {
        std::cout << "\nNo collections found. Create a collection first!\n" << std::endl;
        return;
    }

    try {
        // First for loop was not properly closed
        for (const auto& collection : currentUser->collections) {
            std::cout << collection.getName() << std::endl;
        }  // Added missing closing brace here

        std::string collectionName;
        std::cout << "\nEnter collection name to view: ";
        std::cin.ignore();
        std::getline(std::cin, collectionName);

        bool found = false;
        for (const auto& collection : currentUser->collections) {
            if (collection.getName() == collectionName) {
                collection.displayCollection();
                found = true;
                break;
            }
        }

        if (!found) {
            throw std::runtime_error("Collection not found");
        }

    } catch (const std::exception& e) {
        std::cerr << "Error viewing collection: " << e.what() << std::endl;
    }
}

void UserAccount::connectPhantomWallet() {
    if (!currentUser) {
        std::cout << "Login first" << std::endl;
        return;
    }

    try {

 	if (!checkSolanaInstallation()) {
            installSolanaInstructions();
            throw std::runtime_error("Solana CLI tools not installed");
        }
	
	 std::string safe_email = currentUser->email;
        std::replace(safe_email.begin(), safe_email.end(), '@', '_');
        std::replace(safe_email.begin(), safe_email.end(), '.', '_');
        std::string keypair_dir = "keypairs/" + currentUser->name + "_" + safe_email;
        currentUser->keypairPath = keypair_dir + "/id.json";

        // Set Solana configuration with existing keypair
        std::string config_cmd = "solana config set --url https://api.testnet.solana.com --keypair " + currentUser->keypairPath;
        if (system(config_cmd.c_str()) != 0) {
            throw std::runtime_error("Failed to set configuration");
        }

        // Now connect the wallet
        if (currentUser->wallet.connectPhantom()) {
            currentUser->walletAddress = currentUser->wallet.getPublicKey();
            currentUser->walletBalance = std::to_string(currentUser->wallet.getBalance());
            std::cout << "Phantom Wallet connected successfully!" << std::endl;
            std::cout << "Wallet address: " << currentUser->walletAddress << std::endl;
        } else {
            throw std::runtime_error("Failed to connect wallet");
        }

    } catch(const std::exception& e) {
        std::cerr << "Error connecting wallet: " << e.what() << std::endl;
    }
}


void UserAccount::checkSolBalance() {
    if (!currentUser) {
        std::cout << "Login first" << std::endl;
        return;
    }

    try {
        if (currentUser->wallet.updateBalance()) {
            currentUser->walletBalance = std::to_string(currentUser->wallet.getBalance());
            std::cout << "SOL Balance: " << currentUser->walletBalance << std::endl;
        } else {
            std::cout << "Failed to fetch balance" << std::endl;
        }
    } catch(const std::exception& e) {
        std::cerr << "Error checking balance: " << e.what() << std::endl;
    }
} 



void UserAccount::requestTestSol() {
    if (!currentUser) {
        std::cout << "Login first" << std::endl;
        return;
    }

    try {
        if (currentUser->wallet.requestAirdrop()) {
            currentUser->walletBalance = std::to_string(currentUser->wallet.getBalance());
        }
    } catch(const std::exception& e) {
        std::cerr << "Error requesting SOL: " << e.what() << std::endl;
    }
}
