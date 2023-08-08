#pragma once

#include "search_server.h"

#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) : search_server_(search_server) {}

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        auto results = search_server_.FindTopDocuments(raw_query, document_predicate);
        AddQueryResult(raw_query, results);
        return results;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query) {
        return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
    }

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        std::string query;
        std::vector<Document> results;
    };
    
    void AddQueryResult(const std::string& query, const std::vector<Document>& results);

    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& search_server_;
};