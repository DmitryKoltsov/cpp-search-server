#pragma once
#include <vector>
#include <stack>
#include <string>
#include "search_server.h"
#include "document.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        auto res = search_server_.FindTopDocuments(raw_query, document_predicate);
		if (res.empty()) {
			++no_res_req_;
		}

		requests_.push_back({ res, res.empty() });
		UpdateSize();
		return res;
    }
    
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    
    int GetNoResultRequests() const;
    
private:
    struct QueryResult {
        std::vector<Document> documents_;
        bool empty;
    };
    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    const static int min_in_day_ = 1440;
    int no_res_req_ = 0;
        
    void UpdateSize();
};