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
        std::string keygen_cmd = "solana-keygen new --no-bip39-passphrase --force -o " + keypair_path;
        if (system(keygen_cmd.c_str()) != 0) {
            throw std::runtime_error("Failed to generate keypair");
        }

        // Set Solana configuration
        std::string config_cmd = "solana config set --url https://api.testnet.solana.com --keypair " + keypair_path;
        system(config_cmd.c_str());

        // Get the actual wallet address from the keypair
        std::string address_cmd = "solana address -k " + keypair_path;
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

void UserAccount::loadExistingUsers(std::vector<UserAccount>& users) {
    try {
        std::cout << "Loading existing users from keypairs directory..." << std::endl;
        
        // Check if keypairs directory exists
        std::string cmd = "ls keypairs/ 2>/dev/null";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            std::cout << "No keypairs directory found." << std::endl;
            return;
        }
        
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string dir_name(buffer);
            if (!dir_name.empty() && dir_name[dir_name.length()-1] == '\n') {
                dir_name.erase(dir_name.length()-1);
            }
            
            // Try to load user data from this directory
            std::string info_path = "keypairs/" + dir_name + "/info.json";
            std::ifstream info_file(info_path);
            if (info_file.is_open()) {
                // Parse basic info (simplified - in real app you'd use JSON parser)
                std::string line;
                std::string name, email, walletAddress, balance;
                
                while (std::getline(info_file, line)) {
                    // Clean the line
                    line.erase(0, line.find_first_not_of(" \t"));
                    if (line.empty()) continue;
                    
                    if (line.find("\"name\":") != std::string::npos) {
                        size_t colonPos = line.find(":");
                        if (colonPos != std::string::npos) {
                            size_t start = line.find("\"", colonPos);
                            size_t end = line.find("\"", start + 1);
                            if (start != std::string::npos && end != std::string::npos) {
                                name = line.substr(start + 1, end - start - 1);
                            }
                        }
                    } else if (line.find("\"email\":") != std::string::npos) {
                        size_t colonPos = line.find(":");
                        if (colonPos != std::string::npos) {
                            size_t start = line.find("\"", colonPos);
                            size_t end = line.find("\"", start + 1);
                            if (start != std::string::npos && end != std::string::npos) {
                                email = line.substr(start + 1, end - start - 1);
                            }
                        }
                    } else if (line.find("\"walletAddress\":") != std::string::npos) {
                        size_t colonPos = line.find(":");
                        if (colonPos != std::string::npos) {
                            size_t start = line.find("\"", colonPos);
                            size_t end = line.find("\"", start + 1);
                            if (start != std::string::npos && end != std::string::npos) {
                                walletAddress = line.substr(start + 1, end - start - 1);
                            }
                        }
                    } else if (line.find("\"balance\":") != std::string::npos) {
                        size_t colonPos = line.find(":");
                        if (colonPos != std::string::npos) {
                            size_t start = line.find("\"", colonPos);
                            size_t end = line.find("\"", start + 1);
                            if (start != std::string::npos && end != std::string::npos) {
                                balance = line.substr(start + 1, end - start - 1);
                            }
                        }
                    }
                }
                
                if (!name.empty() && !email.empty()) {
                    UserAccount user(walletAddress, name, email, "", balance);
                    
                    // Load collections for this user
                    std::string user_dir = "keypairs/" + dir_name;
                    user.loadCollections(user_dir);
                    
                    users.push_back(user);
                    std::cout << "Loaded existing user: " << name << " (" << email << ")" << std::endl;
                    std::cout << "  Collections loaded: " << user.getCollections().size() << std::endl;
                } else {
                    std::cout << "Failed to parse user data from " << info_path << std::endl;
                    std::cout << "Parsed values - Name: '" << name << "', Email: '" << email << "'" << std::endl;
                }
                
                info_file.close();
            }
        }
        pclose(pipe);
    } catch (const std::exception& e) {
        std::cerr << "Error loading existing users: " << e.what() << std::endl;
    }
}

void UserAccount::login(std::vector<UserAccount>& users) {
	try {
		std::string inputEmail, inputPassword;
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
	
	// Get current devnet balance
	std::string balance_cmd = "solana balance " + currentUser->getWalletAddress() + " --url https://api.devnet.solana.com";
	FILE* balance_pipe = popen(balance_cmd.c_str(), "r");
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
			double currentBalance = std::stod(balance_str);
			std::cout<<"Current Devnet Balance: "<<currentBalance<<" SOL"<<std::endl;
			// Update stored balance
			currentUser->walletBalance = std::to_string(currentBalance);
		}
		pclose(balance_pipe);
	} else {
		std::cout<<"Wallet Balance: "<<currentUser->walletBalance<<" SOL"<<std::endl;
	}
}


void UserAccount::checkWalletBalance() {
	try {
		if (!currentUser) {
		std::cout <<"Login first\n"<<std::endl;
		return;
	}
	std::cout<<"Address: "<<currentUser->walletAddress<<std::endl;
	
	// Get current devnet balance
	std::string balance_cmd = "solana balance " + currentUser->getWalletAddress() + " --url https://api.devnet.solana.com";
	FILE* balance_pipe = popen(balance_cmd.c_str(), "r");
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
			double currentBalance = std::stod(balance_str);
			std::cout<<"Current Devnet Balance: "<<currentBalance<<" SOL"<<std::endl;
			// Update stored balance
			currentUser->walletBalance = std::to_string(currentBalance);
		}
		pclose(balance_pipe);
	} else {
		std::cout<<"Balance: "<<currentUser->walletBalance<<" SOL"<<std::endl;
	}

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
		
		std::cout<<"Enter collection name: ";
		std::getline(std::cin, collectionName);

		Collection newCollection(collectionName, currentUser->name);

		currentUser->collections.push_back(newCollection);
		collections.push_back(newCollection);

		// Save collections to disk
		std::string safe_email = currentUser->email;
		std::replace(safe_email.begin(), safe_email.end(), '@', '_');
		std::replace(safe_email.begin(), safe_email.end(), '.', '_');
		std::string keypair_dir = "keypairs/" + currentUser->name + "_" + safe_email;
		currentUser->saveCollections(keypair_dir);

		std::cout<<"NFT collection created successfully!"<< std::endl;
		std::cout <<"Collection Name: "<< collectionName<< std::endl;
        	std::cout <<"Creator "<< currentUser->name<< std::endl;

    	} catch (const std::exception& e) {
        	std::cerr<<"Error creating collection: "<< e.what()<< std::endl;
    }
}



void UserAccount::addNFTToCollection(std::vector<NFT>& nfts) {
	
	if (!currentUser) {
		std::cout <<"Login first\n"<<std::endl;
		return;
	}

	if (currentUser->collections.empty()) {
		std::cout<<"Create a collection first\n"<<std::endl;
		return;
	}

// Add Solana balance check (devnet)
        std::string balance_cmd = "solana balance " + currentUser->getWalletAddress() + " --url https://api.devnet.solana.com";
        FILE* balance_pipe = popen(balance_cmd.c_str(), "r");
        double currentBalance = 0.0;
        if (balance_pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), balance_pipe) != nullptr) {
                currentBalance = std::stod(buffer);
            }
            pclose(balance_pipe);
        }
        
        if (currentBalance < 0.05) {
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
		std::cout << "DEBUG: Available collections for user '" << currentUser->name << "':" << std::endl;
		if (currentUser->collections.empty()) {
			std::cout << "  No collections found in currentUser->collections" << std::endl;
		} else {
			for (const auto& collection : currentUser->collections) {
				std::cout << "  - '" << collection.getName() << "'" << std::endl;
			}
		}

		std::string collectionName;
		std::cout<<"\nEnter collection name to add NFT: ";
		std::getline(std::cin, collectionName);
		std::cout << "DEBUG: Looking for collection: '" << collectionName << "'" << std::endl;
		
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
		
		// Save updated collections to disk
		std::string safe_email = currentUser->email;
		std::replace(safe_email.begin(), safe_email.end(), '@', '_');
		std::replace(safe_email.begin(), safe_email.end(), '.', '_');
		std::string keypair_dir = "keypairs/" + currentUser->name + "_" + safe_email;
		currentUser->saveCollections(keypair_dir);

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
        std::string config_cmd = "solana config set --url https://api.devnet.solana.com --keypair " + currentUser->keypairPath;
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
        std::cout << "DEBUG: Current user info:" << std::endl;
        std::cout << "  Name: " << currentUser->name << std::endl;
        std::cout << "  Email: " << currentUser->email << std::endl;
        std::cout << "  Wallet Address: " << currentUser->walletAddress << std::endl;
        std::cout << "Checking SOL balance for wallet: " << currentUser->walletAddress << std::endl;
        
        // Check devnet balance (primary)
        std::string devnet_cmd = "solana balance " + currentUser->walletAddress + " --url https://api.devnet.solana.com";
        FILE* pipe = popen(devnet_cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
                          if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                 std::cout << "Raw buffer: '" << buffer << "'" << std::endl; // DEBUG
                 std::string balance_str(buffer);
                 // Remove " SOL" suffix and newline
                 size_t sol_pos = balance_str.find(" SOL");
                 if (sol_pos != std::string::npos) {
                     balance_str = balance_str.substr(0, sol_pos);
                 }
                 if (!balance_str.empty() && balance_str.back() == '\n') {
                     balance_str.pop_back();
                 }
                 std::cout << "Parsed balance string: '" << balance_str << "'" << std::endl; // DEBUG
                 
                 try {
                     double devnetBalance = std::stod(balance_str);
                     std::cout << "Devnet Balance: " << devnetBalance << " SOL" << std::endl;
                     currentUser->walletBalance = std::to_string(devnetBalance);
                     std::cout << "Updated walletBalance to: " << currentUser->walletBalance << std::endl; // DEBUG
                 } catch (const std::exception& e) {
                     std::cout << "Failed to parse balance: " << e.what() << std::endl;
                 }
             }
            pclose(pipe);
        } else {
            std::cout << "Failed to fetch devnet balance" << std::endl;
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
        std::cout << "Requesting test SOL airdrop..." << std::endl;
        std::cout << "Wallet address: " << currentUser->getWalletAddress() << std::endl;
        
        // Use SolanaIntegration::airdropDevnet directly instead of wallet.requestAirdrop
        SolanaIntegration::airdropDevnet(currentUser->getWalletAddress());
        
        // Update the user's balance
        std::string balance_cmd = "solana balance " + currentUser->getWalletAddress() + " --url https://api.devnet.solana.com";
        FILE* balance_pipe = popen(balance_cmd.c_str(), "r");
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
                    double newBalance = std::stod(balance_str);
                    currentUser->walletBalance = std::to_string(newBalance);
                    std::cout << "Updated balance: " << currentUser->walletBalance << " SOL" << std::endl;
                } catch (const std::exception& e) {
                    std::cout << "Failed to parse balance: " << e.what() << std::endl;
                }
            }
            pclose(balance_pipe);
        }
        
    } catch(const std::exception& e) {
        std::cerr << "Error requesting SOL: " << e.what() << std::endl;
    }
}




    void UserAccount::saveCollections(const std::string& dir) {
        std::string collections_path = dir + "/collections.json";
        std::ofstream collections_file(collections_path);
        if (collections_file.is_open()) {
            collections_file << "{\n";
            collections_file << "  \"collections\": [\n";
            
            for (size_t i = 0; i < collections.size(); i++) {
                const auto& collection = collections[i];
                collections_file << "    {\n";
                collections_file << "      \"name\": \"" << collection.getName() << "\",\n";
                collections_file << "      \"creator\": \"" << collection.getCreator() << "\",\n";
                collections_file << "      \"nfts\": [\n";
                
                const auto& nfts = collection.getNFTs();
                for (size_t j = 0; j < nfts.size(); j++) {
                    const auto& nft = nfts[j];
                    collections_file << "        {\n";
                    collections_file << "          \"name\": \"" << nft.getName() << "\",\n";
                    collections_file << "          \"tokenId\": \"" << nft.getTokenId() << "\",\n";
                    collections_file << "          \"owner\": \"" << nft.getOwner() << "\",\n";
                    collections_file << "          \"price\": " << nft.getPrice() << ",\n";
                    collections_file << "          \"isListed\": " << (nft.getIsListed() ? "true" : "false") << ",\n";
                    collections_file << "          \"mintAddress\": \"" << nft.getMintAddress() << "\",\n";
                    collections_file << "          \"metadataUri\": \"" << nft.getMetadataUri() << "\"\n";
                    collections_file << "        }";
                    if (j < nfts.size() - 1) collections_file << ",";
                    collections_file << "\n";
                }
                
                collections_file << "      ]\n";
                collections_file << "    }";
                if (i < collections.size() - 1) collections_file << ",";
                collections_file << "\n";
            }
            
            collections_file << "  ]\n";
            collections_file << "}";
            collections_file.close();
        }
    }

    void UserAccount::loadCollections(const std::string& dir) {
        std::string collections_path = dir + "/collections.json";
        std::ifstream collections_file(collections_path);
        if (!collections_file.is_open()) {
            return; // No collections file exists yet
        }

        try {
            std::string line;
            std::string currentCollection;
            std::string currentNFT;
            bool inCollections = false;
            bool inCollection = false;
            bool inNFTs = false;
            bool inNFT = false;
            
            std::string collectionName, collectionCreator;
            std::string nftName, nftTokenId, nftOwner, nftMintAddress, nftMetadataUri;
            double nftPrice = 0.0;
            bool nftIsListed = false;
            V<NFT> currentNFTs;

            while (std::getline(collections_file, line)) {
                // Remove whitespace
                line.erase(0, line.find_first_not_of(" \t"));
                if (line.empty()) continue;

                if (line.find("\"collections\":") != std::string::npos) {
                    inCollections = true;
                } else if (line.find("\"name\":") != std::string::npos && inCollections) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        if (inCollection) {
                            collectionName = line.substr(start + 1, end - start - 1);
                        } else if (inNFT) {
                            nftName = line.substr(start + 1, end - start - 1);
                        }
                    }
                } else if (line.find("\"creator\":") != std::string::npos && inCollection) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        collectionCreator = line.substr(start + 1, end - start - 1);
                    }
                } else if (line.find("\"tokenId\":") != std::string::npos && inNFT) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        nftTokenId = line.substr(start + 1, end - start - 1);
                    }
                } else if (line.find("\"owner\":") != std::string::npos && inNFT) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        nftOwner = line.substr(start + 1, end - start - 1);
                    }
                } else if (line.find("\"price\":") != std::string::npos && inNFT) {
                    size_t colonPos = line.find(":");
                    if (colonPos != std::string::npos) {
                        std::string priceStr = line.substr(colonPos + 1);
                        // Remove trailing comma if present
                        if (!priceStr.empty() && priceStr.back() == ',') {
                            priceStr.pop_back();
                        }
                        nftPrice = std::stod(priceStr);
                    }
                } else if (line.find("\"isListed\":") != std::string::npos && inNFT) {
                    if (line.find("true") != std::string::npos) {
                        nftIsListed = true;
                    }
                } else if (line.find("\"mintAddress\":") != std::string::npos && inNFT) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        nftMintAddress = line.substr(start + 1, end - start - 1);
                    }
                } else if (line.find("\"metadataUri\":") != std::string::npos && inNFT) {
                    size_t start = line.find("\"", line.find(":"));
                    size_t end = line.find("\"", start + 1);
                    if (start != std::string::npos && end != std::string::npos) {
                        nftMetadataUri = line.substr(start + 1, end - start - 1);
                    }
                } else if (line.find("\"nfts\":") != std::string::npos && inCollection) {
                    inNFTs = true;
                } else if (line.find("{") != std::string::npos) {
                    if (inCollections && !inCollection) {
                        inCollection = true;
                        collectionName = "";
                        collectionCreator = "";
                        currentNFTs.clear();
                    } else if (inNFTs && !inNFT) {
                        inNFT = true;
                        nftName = "";
                        nftTokenId = "";
                        nftOwner = "";
                        nftPrice = 0.0;
                        nftIsListed = false;
                        nftMintAddress = "";
                        nftMetadataUri = "";
                    }
                } else if (line.find("}") != std::string::npos) {
                    if (inNFT) {
                        // Complete NFT
                        NFT nft(nftName, nftOwner, nftPrice, nftIsListed, nftMetadataUri);
                        // Set additional properties
                        // Note: We can't set tokenId directly as it's generated in constructor
                        nft.setIsListed(nftIsListed);
                        currentNFTs.push_back(nft);
                        inNFT = false;
                    } else if (inCollection) {
                        // Complete collection
                        Collection collection(collectionName, collectionCreator);
                        for (const auto& nft : currentNFTs) {
                            collection.addNFT(nft);
                        }
                        collections.push_back(collection);
                        inCollection = false;
                        inNFTs = false;
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading collections: " << e.what() << std::endl;
        }
        
        collections_file.close();
    }


