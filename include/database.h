#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <memory>
#include <libpq-fe.h>

namespace khushi {

class Database {
public:
    Database();
    ~Database();
    
    // Connect to database
    bool connect(const std::string& conn_string);
    
    // Disconnect from database
    void disconnect();
    
    // User operations
    int create_user(const std::string& username);
    int get_user_id(const std::string& username);
    
    // Conversation operations
    int create_conversation(int user_id, const std::string& title);
    void save_message(int conversation_id, bool is_user, const std::string& content);
    
    // Load conversation history
    std::vector<std::pair<bool, std::string>> load_conversation(int conversation_id);
    
    // Check if connected
    bool is_connected() const { return connected_; }

private:
    PGconn* connection_; // PostgreSQL connection
    bool connected_;
};

} // namespace khushi

#endif // DATABASE_H
