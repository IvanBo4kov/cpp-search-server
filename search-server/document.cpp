#include "document.h"

std::ostream& operator<<(std::ostream& os, const Document& document) {
    os << "{ ";
    os << "document_id = " << document.id;
    os << ", relevance = " << document.relevance;
    os << ", rating = " << document.rating << " }";
    return os;
}