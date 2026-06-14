#ifndef WEB_SEARCH_H
#define WEB_SEARCH_H

#include <string>
#include <vector>

namespace khushi {

class WebSearch {
public:
    WebSearch();
    ~WebSearch();
    
    // Search the web for information
    std::string search(const std::string& query);
    
    // Get search results as structured data
    struct SearchResult {
        std::string title;
        std::string url;
        std::string snippet;
    };
    std::vector<SearchResult> search_results(const std::string& query);
    
private:
    // Make HTTP request to search API
    std::string make_search_request(const std::string& query);
    
    // Parse search results from JSON response
    std::vector<SearchResult> parse_search_results(const std::string& json_response);
};

} // namespace khushi

#endif // WEB_SEARCH_H
