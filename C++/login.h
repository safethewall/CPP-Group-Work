#pragma once
#include "user.h"
#include "include.h"
#include "legalinput.h"

using namespace std;

class Login {
private:
    bool loginsuccess = false;
    bool administratorToken = false;

public:
    User* log, * user;
    Login() {
        leading();
    };
    ~Login() {}
    void success() {
        loginsuccess = true;
    }
    void tokencheck() {//再加入一个用户名SHA加分别加上id和type的SHA来保密，防止权限泄露
        administratorToken = true;
    }
    void userload(string username, string password, string id, string type) {
        if (stoi(type) == 1) {
            user = new Customer(username, password, type, id);

        }
        else if (stoi(type) == 2) {
            user = new Team(username, password, type, id);
        }
        else if (stoi(type) == 3) {
            user = new Manager(username, password, type, id);
        }
        else {
            cout << "unknown error ,exit with code 100" << endl;
            exit(100);
        }
    }
    User* returnuser() {
        log = user;
        return log;
    };
    bool returnToken() {
        return administratorToken;
    };
    bool returnloginstate() {
        return loginsuccess;
    }
    void managermode() {
        std::unordered_map<std::string, std::tuple<std::string, std::string, std::string>> users;
        loadUsers(users);
        registerUser(users);
    }


    string hashPassword(const string& password) {
        CryptoPP::SHA256 hash;
        CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
        hash.CalculateDigest(digest, (const CryptoPP::byte*)password.data(), password.size());

        CryptoPP::HexEncoder encoder;
        string output;
        encoder.Attach(new CryptoPP::StringSink(output));
        encoder.Put(digest, sizeof(digest));
        encoder.MessageEnd();

        return output;
    }

    // Saves the user's information in a file
    void saveUser(const std::string& username, const std::string& hashedPassword, const std::string& id, const std::string& type) {
        std::ofstream file("users.txt", std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Failed to open users.txt for appending.\n";
            return;
        }
        if (file.tellp() > 0) {
            file << std::endl;
        }
        file << username << endl << hashedPassword << endl << id << endl << type << endl;
    }

    // Loads users from a file into a data structure
    bool loadUsers(std::unordered_map<std::string, std::tuple<std::string, std::string, std::string>>& users) {
        std::fstream file;
        file.open("users.txt", std::fstream::in | std::fstream::out);
        if (!file.is_open()) {
            std::cout << "File does not exist, creating it.\n";
            file.open("users.txt", std::fstream::out);
        }
        else {
            std::cout << "File already exists, opening and moving the read pointer to the first position.\n";
        }

        if (file.is_open()) {
            std::string username, password, id, type;
            while (std::getline(file, username)) {
                std::getline(file, password);
                std::getline(file, id);
                std::getline(file, type);
                users[username] = make_tuple(password, id, type);
            }
            file.close();
        }
        else {
            std::cout << "The file cannot be opened or created.\n";
            exit(104);
        }

        return true;
    }

    // Checks if the password is strong
    bool isPasswordStrong(const string& password) {
        if (password.length() < 8) {
            return false; // The password needs to be at least 8 characters long.
        }
        bool containsDigit = false;
        bool containsAlpha = false;
        for (char ch : password) {
            if (isdigit(ch)) {
                containsDigit = true;
            }
            else if (isalpha(ch)) {
                containsAlpha = true;
            }
            if (containsDigit && containsAlpha) {
                return true; // Found both a digit and an alphabetic character.
            }
        }
        return false; // Did not find both a digit and an alphabetic character.
    }

    // Registers a new user after validating password strength
    void registerUser(unordered_map<string, std::tuple<std::string, std::string, std::string>>& users) {
        string username, password, id, type;
        cout << "Enter username: ";
        cin >> username;

        if (users.find(hashPassword(username)) != users.end()) {
            cout << "Username already exists.\n";
            return;
        }

        do {
            cout << "Enter password: ";
            cin >> password;

            if (!isPasswordStrong(password)) {
                cout << "Password must be at least 8 characters long and contain at least one digit.\n";
            }
            else {
                break; // Valid password, exit loop.
            }
        } while (true);
        bool idcheck = true;
        do {
            while (idcheck == true) {
                cout << "Enter your id: ";
                cin >> id;
                for (const auto value : users) {
                    if (id == get<1>(value.second)) {
                        cout << "the is is existing,enter again" << endl;
                        idcheck = false;
                    }
                }
                if (idcheck == true) {
                    break;
                }
                else {
                    idcheck = true;
                }
            }
            cout << "the id no existing,accept" << endl;
            break;
        } while (true);
        int numbertype = 0;
        if (administratorToken == false) {
            do {
                cout << "enter number to choose usertype \n 1.customer 2.team  ";
                numbertype = getValidInt();
                if (numbertype == 1 || numbertype == 2 ) {
                    break;
                }
            } while (true);
        }
        else {
            do {
                cout << "enter number to choose usertype \n 1.customer 2.team 3.administer: ";
                numbertype = getValidInt();
                if (numbertype == 1 || numbertype == 2 || numbertype == 3) {
                    break;
                }
            } while (true);
        }
        
        type = to_string(numbertype);
        string hashedUsername = hashPassword(username);
        string hashedPassword = hashPassword(password);
        users[hashedUsername] = make_tuple(hashedPassword, id, type);
        saveUser(hashedUsername, hashedPassword, id, type);
        cout << "User registered successfully!\n";
    }

    // Handles the login process
    void loginUser(const unordered_map<string, tuple<string, string, string>>& users) {
        string username, password;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;
        auto it = users.find(hashPassword(username));
        if (it == users.end()) {
            cout << "Invalid login credentials.\n";
            return;
        }

        if (std::get<0>(it->second) != hashPassword(password)) {
            int time = 3;
            while (time--) {
                cout << "Wrong password, please enter again.\n";
                cout << "You have " << time + 1 << " chance(s).\n";
                cin >> password;
                if (std::get<0>(it->second) == hashPassword(password)) {
                    break;
                }
            }
            if (time < 0) { // All attempts used
                cout << "No more attempts allowed.\n";
                return;
            }
        }
        cout << "Logged in successfully!\n";
        success();
        userload(username, password, get<1>(it->second), get<2>(it->second));
        cout << "Your privilege is " << std::get<2>(it->second) << ".\n";
        if (stoi(std::get<2>(it->second)) == 1) {
            cout << "Your grade is customer.\n";
        }
        else if (stoi(std::get<2>(it->second)) == 2) {
            cout << "Your grade is team.\n";
        }
        else {
            cout << "Your grade is administrater.\n";
            tokencheck();
        }
        cout << endl;
    }

    void leading() {//加一个只有经理登录后，注册才开放23的权限
        std::unordered_map<std::string, std::tuple<std::string, std::string, std::string>> users;
        if (!loadUsers(users)) {
            cout << "Failed to load user data, exiting.\n";
            exit(1);
        }
        else {
            cout << "Loaded user data successfully.\n";
        }

        bool exitProgram = false;
        while (!exitProgram) {
            int option;
            cout << "1: Register\n2: Login\n0: Exit\nEnter option: ";
            option=getValidInt();
            switch (option) {
            case 1:
                registerUser(users);
                break;
            case 2:
                loginUser(users);
                if (returnloginstate() == true) {
                    exitProgram = true;

                }
                break;
            case 0:
                exitProgram = true;
                cout << "Exiting program.\n";
                break;
            default:
                cout << "Invalid option.\n";
            }
        }

    }
};