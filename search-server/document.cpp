#include "document.h"

using  std::string_literals::operator ""s;

Document::Document() = default;

Document::Document(int id, double relevance, int rating)
	: id(id)
	, relevance(relevance)
	, rating(rating)
{}
std::ostream& operator<<(std::ostream& out, const Document& document) {
	out << "{ "s
		<< "document_id = "s << document.id << ", "s
		<< "relevance = "s << document.relevance << ", "s
		<< "rating = "s << document.rating << " }"s;
	return out;
}