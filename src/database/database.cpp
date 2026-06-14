#include "database.h"
#include <iostream>
#include <cstring>
#include <libpq-fe.h>

namespace khushi {

Database::Database() : connection_(nullptr), connected_(false) {
}

Database::~Database() {
    disconnect();
}

bool Database::connect(const std::string& conn_string) {
    connection_ = PQconnectdb(conn_string.c_str());
    
    if (PQstatus(connection_) != CONNECTION_OK) {
        std::cerr << "Database connection failed: " << PQerrorMessage(connection_) << std::endl;
        PQfinish(connection_);
        connection_ = nullptr;
        connected_ = false;
        return false;
    }
    
    connected_ = true;
    std::cout << "Connected to PostgreSQL database successfully" << std::endl;
    return true;
}

void Database::disconnect() {
    if (connection_) {
        PQfinish(connection_);
        connection_ = nullptr;
        connected_ = false;
        std::cout << "Disconnected from PostgreSQL database" << std::endl;
    }
}

int Database::create_user(const std::string& username) {
    if (!connected_) return -1;
    
    std::string query = "INSERT INTO users (username) VALUES ('" + username + "') RETURNING id;";
    PGresult* result = PQexec(connection_, query.c_str());
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "Failed to create user: " << PQerrorMessage(connection_) << std::endl;
        PQclear(result);
        return -1;
    }
    
    int user_id = atoi(PQgetvalue(result, 0, 0));
    PQclear(result);
    
    std::cout << "Created user with ID: " << user_id << std::endl;
    return user_id;
}

int Database::get_user_id(const std::string& username) {
    if (!connected_) return -1;
    
    std::string query = "SELECT id FROM users WHERE username = '" + username + "';";
    PGresult* result = PQexec(connection_, query.c_str());
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "Failed to get user ID: " << PQerrorMessage(connection_) << std::endl;
        PQclear(result);
        return -1;
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return -1; // User not found
    }
    
    int user_id = atoi(PQgetvalue(result, 0, 0));
    PQclear(result);
    
    return user_id;
}

int Database::create_conversation(int user_id, const std::string& title) {
    if (!connected_) return -1;
    
    std::string query = "INSERT INTO conversations (user_id, title) VALUES (" + 
                       std::to_string(user_id) + ", '" + title + "') RETURNING id;";
    PGresult* result = PQexec(connection_, query.c_str());
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "Failed to create conversation: " << PQerrorMessage(connection_) << std::endl;
        PQclear(result);
        return -1;
    }
    
    int conversation_id = atoi(PQgetvalue(result, 0, 0));
    PQclear(result);
    
    std::cout << "Created conversation with ID: " << conversation_id << std::endl;
    return conversation_id;
}

void Database::save_message(int conversation_id, bool is_user, const std::string& content) {
    if (!connected_) return;
    
    std::string query = "INSERT INTO messages (conversation_id, is_user, content) VALUES (" + 
                       std::to_string(conversation_id) + ", " + 
                       (is_user ? "true" : "false") + ", '" + content + "');";
    
    PGresult* result = PQexec(connection_, query.c_str());
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        std::cerr << "Failed to save message: " << PQerrorMessage(connection_) << std::endl;
    }
    
    PQclear(result);
}

std::vector<std::pair<bool, std::string>> Database::load_conversation(int conversation_id) {
    std::vector<std::pair<bool, std::string>> messages;
    
    if (!connected_) return messages;
    
    std::string query = "SELECT is_user, content FROM messages WHERE conversation_id = " + 
                       std::to_string(conversation_id) + " ORDER BY created_at;";
    
    PGresult* result = PQexec(connection_, query.c_str());
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "Failed to load conversation: " << PQerrorMessage(connection_) << std::endl;
        PQclear(result);
        return messages;
    }
    
    int rows = PQntuples(result);
    for (int i = 0; i < rows; i++) {
        bool is_user = (strcmp(PQgetvalue(result, i, 0), "t") == 0);
        std::string content = PQgetvalue(result, i, 1);
        messages.push_back({is_user, content});
    }
    
    PQclear(result);
    
    std::cout << "Loaded " << messages.size() << " messages from conversation" << std::endl;
    return messages;
}

} // namespace khushi
