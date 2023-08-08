#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end) {}
    
    Iterator begin() const {
        return begin_;
    }
    
    Iterator end() const {
        return end_;
    }
    
    size_t size() const {
        return distance(begin_, end_);
    }
private:
    Iterator begin_;
    Iterator end_;
};

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        const size_t container_size = distance(begin, end);
        const size_t num_pages = container_size / page_size + (container_size % page_size != 0);

        auto page_begin = begin;
        for (size_t i = 0; i < num_pages; ++i) {
            const size_t current_page_size = std::min(page_size, container_size - i * page_size);
            const auto page_end = next(page_begin, current_page_size);
            pages_.emplace_back(page_begin, page_end);
            page_begin = page_end;
        }
    }
    
    auto begin() const {
        return pages_.begin();
    }
    
    auto end() const {
        return pages_.end();
    }
    
    size_t size() const {
        return pages_.size();
    }
private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& os, const IteratorRange<Iterator>& range) {
    for (auto it = range.begin(); it != range.end(); ++it) {
        os << *it;
    }
    return os;
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}