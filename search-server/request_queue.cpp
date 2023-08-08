#include "request_queue.h"

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return AddFindRequest(raw_query, [status](int document_id, DocumentStatus doc_status, int rating) {
        return doc_status == status;
    });
}

int RequestQueue::GetNoResultRequests() const {
    int count = 0;
    for (const auto& query_result : requests_) {
        if (query_result.number_of_documents == 0) {
            ++count;
        }
    }
    return count;
}

void RequestQueue::AddQueryResult(const std::string& query, const size_t& results) {
    if (requests_.size() == min_in_day_) {
        requests_.pop_front();
    }
    requests_.emplace_back(QueryResult{query, results});
}