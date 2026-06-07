#include "command_executor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

namespace khushi {

CommandExecutor::CommandExecutor() {}

CommandExecutor::~CommandExecutor() {}

void CommandExecutor::set_permission_callback(std::function<bool(const std::string&)> callback) {
    permission_callback_ = callback;
}

bool CommandExecutor::is_command_safe(const std::string& command) {
    // Block dangerous commands
    std::vector<std::string> dangerous = {"rm -rf /", "mkfs", "dd if=", "format", ":(){:|:&};:"};
    for (const auto& danger : dangerous) {
        if (command.find(danger) != std::string::npos) {
            return false;
        }
    }
    return true;
}

CommandResult CommandExecutor::execute_command(const std::string& command) {
    CommandResult result;
    
    if (!is_command_safe(command)) {
        result.success = false;
        result.error = "Command blocked for security reasons";
        return result;
    }
    
    if (permission_callback_ && !permission_callback_(command)) {
        result.success = false;
        result.error = "Permission denied";
        return result;
    }
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        result.success = false;
        result.error = "Failed to execute command";
        return result;
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result.output += buffer;
    }
    
    int exit_code = pclose(pipe);
    result.success = (exit_code == 0);
    
    if (!result.success) {
        result.error = "Command failed with exit code " + std::to_string(exit_code);
    }
    
    return result;
}

std::string CommandExecutor::extract_command(const std::string& input) {
    std::string lower = input;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

std::string CommandExecutor::extract_argument(const std::string& input, const std::string& keyword) {
    size_t pos = input.find(keyword);
    if (pos != std::string::npos) {
        size_t start = pos + keyword.length();
        size_t end = input.find_first_of(" \t\n\r", start);
        if (end == std::string::npos) {
            return input.substr(start);
        }
        return input.substr(start, end - start);
    }
    return "";
}

bool CommandExecutor::is_file_command(const std::string& input) {
    std::string lower = extract_command(input);
    return lower.find("open") != std::string::npos ||
           lower.find("create") != std::string::npos ||
           lower.find("delete") != std::string::npos ||
           lower.find("list") != std::string::npos ||
           lower.find("file") != std::string::npos;
}

bool CommandExecutor::is_app_command(const std::string& input) {
    std::string lower = extract_command(input);
    return lower.find("launch") != std::string::npos ||
           lower.find("open") != std::string::npos ||
           lower.find("start") != std::string::npos ||
           lower.find("close") != std::string::npos ||
           lower.find("app") != std::string::npos;
}

bool CommandExecutor::is_system_command(const std::string& input) {
    std::string lower = extract_command(input);
    return lower.find("system") != std::string::npos ||
           lower.find("battery") != std::string::npos ||
           lower.find("disk") != std::string::npos ||
           lower.find("info") != std::string::npos ||
           lower.find("status") != std::string::npos;
}

bool CommandExecutor::is_web_command(const std::string& input) {
    std::string lower = extract_command(input);
    return lower.find("open") != std::string::npos && lower.find("http") != std::string::npos ||
           lower.find("search") != std::string::npos ||
           lower.find("browser") != std::string::npos ||
           lower.find("website") != std::string::npos;
}

CommandResult CommandExecutor::execute_natural_command(const std::string& input) {
    std::string lower = extract_command(input);
    CommandResult result;
    
    // File commands
    if (is_file_command(input)) {
        if (lower.find("open file") != std::string::npos || lower.find("open the file") != std::string::npos) {
            std::string filepath = extract_argument(input, "file");
            if (filepath.empty()) {
                // Try to extract from context
                size_t pos = input.find("open");
                if (pos != std::string::npos) {
                    filepath = input.substr(pos + 4);
                    filepath = filepath.substr(filepath.find_first_not_of(" \t\n\r"));
                }
            }
            return open_file(filepath);
        }
        else if (lower.find("create file") != std::string::npos) {
            std::string filepath = extract_argument(input, "file");
            return create_file(filepath, "");
        }
        else if (lower.find("delete file") != std::string::npos) {
            std::string filepath = extract_argument(input, "file");
            return delete_file(filepath);
        }
        else if (lower.find("list") != std::string::npos || lower.find("show files") != std::string::npos) {
            std::string path = extract_argument(input, "in");
            if (path.empty()) path = ".";
            return list_directory(path);
        }
    }
    
    // Application commands
    if (is_app_command(input)) {
        if (lower.find("launch") != std::string::npos || lower.find("start") != std::string::npos) {
            std::string app = extract_argument(input, "launch");
            if (app.empty()) app = extract_argument(input, "start");
            return launch_application(app);
        }
        else if (lower.find("close") != std::string::npos) {
            std::string app = extract_argument(input, "close");
            return close_application(app);
        }
    }
    
    // System commands
    if (is_system_command(input)) {
        if (lower.find("battery") != std::string::npos) {
            return get_battery_info();
        }
        else if (lower.find("disk") != std::string::npos) {
            return get_disk_info();
        }
        else if (lower.find("system") != std::string::npos || lower.find("info") != std::string::npos) {
            return get_system_info();
        }
    }
    
    // Web commands
    if (is_web_command(input)) {
        if (lower.find("http") != std::string::npos) {
            size_t url_start = input.find("http");
            std::string url = input.substr(url_start);
            size_t url_end = url.find_first_of(" \t\n\r");
            if (url_end != std::string::npos) {
                url = url.substr(0, url_end);
            }
            return open_url(url);
        }
        else if (lower.find("search") != std::string::npos) {
            std::string query = extract_argument(input, "for");
            if (query.empty()) {
                size_t pos = input.find("search");
                query = input.substr(pos + 6);
                query = query.substr(query.find_first_not_of(" \t\n\r"));
            }
            return search_web(query);
        }
    }
    
    result.success = false;
    result.error = "Unknown command";
    return result;
}

CommandResult CommandExecutor::open_file(const std::string& filepath) {
    std::string cmd = "xdg-open \"" + filepath + "\" 2>&1";
    CommandResult result = execute_command(cmd);
    if (result.success) {
        result.output = "Opening file: " + filepath;
    }
    return result;
}

CommandResult CommandExecutor::create_file(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        CommandResult result;
        result.success = false;
        result.error = "Failed to create file";
        return result;
    }
    file << content;
    file.close();
    
    CommandResult result;
    result.success = true;
    result.output = "File created: " + filepath;
    return result;
}

CommandResult CommandExecutor::delete_file(const std::string& filepath) {
    std::string cmd = "rm \"" + filepath + "\" 2>&1";
    CommandResult result = execute_command(cmd);
    if (result.success) {
        result.output = "File deleted: " + filepath;
    }
    return result;
}

CommandResult CommandExecutor::list_directory(const std::string& path) {
    std::string cmd = "ls -la \"" + path + "\" 2>&1";
    CommandResult result = execute_command(cmd);
    if (result.success) {
        result.output = "Directory listing for " + path + ":\n" + result.output;
    }
    return result;
}

CommandResult CommandExecutor::launch_application(const std::string& app_name) {
    std::string cmd = app_name + " & 2>&1";
    CommandResult result = execute_command(cmd);
    if (result.success) {
        result.output = "Launching application: " + app_name;
    }
    return result;
}

CommandResult CommandExecutor::close_application(const std::string& app_name) {
    std::string cmd = "pkill -9 " + app_name + " 2>&1";
    CommandResult result = execute_command(cmd);
    if (result.success) {
        result.output = "Closing application: " + app_name;
    }
    return result;
}

CommandResult CommandExecutor::get_system_info() {
    std::string cmd = "uname -a 2>&1";
    CommandResult result = execute_command(cmd);
    if (result.success) {
        result.output = "System Info:\n" + result.output;
    }
    return result;
}

CommandResult CommandExecutor::get_battery_info() {
    std::string cmd = "upower -i /org/freedesktop/UPower/devices/battery 2>&1 | grep -E 'state|percentage' 2>&1";
    CommandResult result = execute_command(cmd);
    if (result.success) {
        result.output = "Battery Info:\n" + result.output;
    } else {
        // Fallback for systems without upower
        cmd = "cat /proc/acpi/battery/BAT0/info 2>&1";
        result = execute_command(cmd);
        if (result.success) {
            result.output = "Battery Info:\n" + result.output;
        } else {
            result.output = "Battery information not available";
            result.success = true;
        }
    }
    return result;
}

CommandResult CommandExecutor::get_disk_info() {
    std::string cmd = "df -h 2>&1";
    CommandResult result = execute_command(cmd);
    if (result.success) {
        result.output = "Disk Usage:\n" + result.output;
    }
    return result;
}

CommandResult CommandExecutor::open_url(const std::string& url) {
    std::string cmd = "xdg-open \"" + url + "\" 2>&1";
    CommandResult result = execute_command(cmd);
    if (result.success) {
        result.output = "Opening URL: " + url;
    }
    return result;
}

CommandResult CommandExecutor::search_web(const std::string& query) {
    std::string encoded_query = query;
    // Simple URL encoding (replace spaces with +)
    for (size_t i = 0; i < encoded_query.length(); ++i) {
        if (encoded_query[i] == ' ') {
            encoded_query[i] = '+';
        }
    }
    std::string url = "https://www.google.com/search?q=" + encoded_query;
    return open_url(url);
}

} // namespace khushi
