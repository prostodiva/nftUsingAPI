#include <crow.h>
#include "../include/header.hpp"
#include <string>

void startApiServer() {
    crow::SimpleApp app;
    const std::vector<int> ports = {3001, 3002, 3003};

    for (int port : ports) {
        try {
            CROW_ROUTE(app, "/api/account/create").methods("POST"_method)
            ([](const crow::request& req) {
                try {
                    auto x = crow::json::load(req.body);
                    
                    std::string name = x["name"].s();
                    std::string keypair_path = "keypairs/" + name + "/id.json";
                    
                    // Use solana-keygen with macOS path
                    std::string keygen_cmd = "/Users/" + std::string(getenv("USER")) + "/.local/share/solana/install/active_release/bin/solana-keygen new --no-bip39-passphrase --force -o " + keypair_path;
                    
                    if (system(keygen_cmd.c_str()) != 0) {
                        return crow::response(500, "Failed to generate keypair");
                    }

                    crow::json::wvalue response;
                    response["status"] = "success";
                    response["keypair_path"] = keypair_path;
                    return crow::response(response);
                } catch(const std::exception& e) {
                    return crow::response(400, e.what());
                }
            });

	CROW_ROUTE(app, "/api/account/<string>").methods("GET"_method)
            ([](const std::string& name) -> crow::response {
                try {
                    // Use the same path structure as in POST
                    std::string keypair_path = "keypairs/" + name + "/id.json";
                    
                    // Check if the keypair file exists
                    std::string check_cmd = "test -f " + keypair_path;
                    if (system(check_cmd.c_str()) == 0) {
                        // File exists, return account info
                        crow::json::wvalue response;
                        response["status"] = "success";
                        response["name"] = name;
                        response["keypair_path"] = keypair_path;
                        return crow::response(response);
                    } else {
                        // File doesn't exist
                        return crow::response(404, "Account not found");
                    }
                } catch(const std::exception& e) {
                    return crow::response(400, e.what());
                }
            });

	CROW_ROUTE(app, "/api/accounts").methods("GET"_method)
            ([]() {
                try {
                    std::vector<std::string> accounts;
                    std::string cmd = "ls -1 keypairs/";
                    
                    // Execute ls command and capture output
                    FILE* pipe = popen(cmd.c_str(), "r");
                    if (!pipe) {
                        return crow::response(500, "Failed to list accounts");
                    }
                    
                    char buffer[128];
                    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                        std::string dir(buffer);
                        if (!dir.empty() && dir[dir.length()-1] == '\n') {
                            dir.erase(dir.length()-1);
                        }
                        accounts.push_back(dir);
                    }
                    pclose(pipe);

                    // Create response
                    crow::json::wvalue response;
                    response["status"] = "success";
                    response["accounts"] = accounts;
                    return crow::response(response);
                } catch(const std::exception& e) {
                    return crow::response(400, e.what());
                }
            });	

	// Add DELETE endpoint
            CROW_ROUTE(app, "/api/account/<string>").methods("DELETE"_method)
            ([](const std::string& name) {
                try {
                    std::string keypair_path = "keypairs/" + name + "/id.json";
                    std::string dir_path = "keypairs/" + name;
                    
                    // Check if account exists
                    std::string check_cmd = "test -f " + keypair_path;
                    if (system(check_cmd.c_str()) == 0) {
                        // Delete the keypair file and directory
                        std::string rm_cmd = "rm -rf " + dir_path;
                        if (system(rm_cmd.c_str()) != 0) {
                            return crow::response(500, "Failed to delete account");
                        }

                        crow::json::wvalue response;
                        response["status"] = "success";
                        response["message"] = "Account deleted successfully";
                        return crow::response(response);
                    } else {
                        return crow::response(404, "Account not found");
                    }
                } catch(const std::exception& e) {
                    return crow::response(400, e.what());
                }
            });

	
	// PUT endpoint for updating account
CROW_ROUTE(app, "/api/account/<string>").methods("PUT"_method)
([](const crow::request& req, const std::string& name) {
    try {
        // First check if the account exists
        std::string old_keypair_path = "keypairs/" + name + "/id.json";
        std::string check_cmd = "test -f " + old_keypair_path;
        
        if (system(check_cmd.c_str()) != 0) {
            crow::json::wvalue error_response;
            error_response["status"] = "error";
            error_response["message"] = "Account '" + name + "' not found";
            return crow::response(404, error_response);
        }

        // Parse the update request
        auto x = crow::json::load(req.body);
        if (!x.has("new_name")) {
            crow::json::wvalue error_response;
            error_response["status"] = "error";
            error_response["message"] = "Missing 'new_name' in request body";
            return crow::response(400, error_response);
        }

        std::string new_name = x["new_name"].s();
        std::string new_keypair_path = "keypairs/" + new_name + "/id.json";
        
        // Create new directory
        std::string mkdir_cmd = "mkdir -p keypairs/" + new_name;
        if (system(mkdir_cmd.c_str()) != 0) {
            return crow::response(500, "Failed to create new directory");
        }

        // Move the keypair file
        std::string mv_cmd = "mv " + old_keypair_path + " " + new_keypair_path;
        if (system(mv_cmd.c_str()) != 0) {
            return crow::response(500, "Failed to move keypair");
        }

        // Remove old directory
        std::string rmdir_cmd = "rm -rf keypairs/" + name;
        system(rmdir_cmd.c_str());  // Don't check result as it's not critical

        // Return success response
        crow::json::wvalue response;
        response["status"] = "success";
        response["message"] = "Account updated successfully";
        response["old_name"] = name;
        response["new_name"] = new_name;
        response["new_keypair_path"] = new_keypair_path;
        return crow::response(response);
    } catch(const std::exception& e) {
        crow::json::wvalue error_response;
        error_response["status"] = "error";
        error_response["message"] = e.what();
        return crow::response(400, error_response);
    }
});

            std::cout << "Server starting on port " << port << "..." << std::endl;
            app.port(port).run();
            return;
        } catch (const std::exception& e) {
            std::cerr << "Failed to start on port " << port << ": " << e.what() << std::endl;
        }
    }
}
