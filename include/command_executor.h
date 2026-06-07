#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include <string>
#include <vector>
#include <functional>

namespace khushi {

struct CommandResult {
    bool success;
    std::string output;
    std::string error;
};

class CommandExecutor {
public:
    CommandExecutor();
    ~CommandExecutor();
    
    // Execute a system command
    CommandResult execute_command(const std::string& command);
    
    // Parse natural language and execute corresponding command
    CommandResult execute_natural_command(const std::string& input);
    
    // File operations
    CommandResult open_file(const std::string& filepath);
    CommandResult create_file(const std::string& filepath, const std::string& content);
    CommandResult delete_file(const std::string& filepath);
    CommandResult list_directory(const std::string& path);
    
    // Application operations
    CommandResult launch_application(const std::string& app_name);
    CommandResult close_application(const std::string& app_name);
    
    // System information
    CommandResult get_system_info();
    CommandResult get_battery_info();
    CommandResult get_disk_info();
    
    // Web operations
    CommandResult open_url(const std::string& url);
    CommandResult search_web(const std::string& query);
    
    // Security
    void set_permission_callback(std::function<bool(const std::string&)> callback);
    bool is_command_safe(const std::string& command);

private:
    std::function<bool(const std::string&)> permission_callback_;
    
    // Command parsing helpers
    std::string extract_command(const std::string& input);
    std::string extract_argument(const std::string& input, const std::string& keyword);
    bool is_file_command(const std::string& input);
    bool is_app_command(const std::string& input);
    bool is_system_command(const std::string& input);
    bool is_web_command(const std::string& input);
};

} // namespace khushi

#endif // COMMAND_EXECUTOR_H
