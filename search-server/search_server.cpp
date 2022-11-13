#include "search_server.h"

using std::string_literals::operator""s;
using std::string_view_literals::operator""sv;

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))  
{
}

SearchServer::SearchServer(const std::string_view stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))  
{
}

void SearchServer::AddDocument(int document_id, const std::string_view document, DocumentStatus status, const std::vector<int>& ratings) {
    if ((document_id < 0) || (SearchServer::documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    const auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (const std::string_view word : words) {
        word_to_document_freqs_[std::basic_string(word)][document_id] += inv_word_count;
        document_to_word_freqs_[document_id][word] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

const std::set<int>::const_iterator SearchServer::begin() const
{
    return document_ids_.cbegin();
}

const std::set<int>::const_iterator SearchServer::end() const
{
    return document_ids_.cend();
}

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    
    return document_to_word_freqs_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) 
{
    if(find(document_ids_.begin(), document_ids_.end(), document_id) == document_ids_.end())
    {
        return;
    }
    for (auto [word, freq] : document_to_word_freqs_[document_id])
    {
        word_to_document_freqs_[std::basic_string(word)].erase(document_id);
    }
    document_to_word_freqs_.erase(document_id);
    documents_.erase(document_id);
    document_ids_.erase(find(document_ids_.begin(), document_ids_.end(), document_id));
}


std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view raw_query, int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, const std::string_view raw_query, int document_id) const {
    auto query = ParseQuery(raw_query,true);

    std::vector<std::string_view> matched_words;

    for (const std::string_view word : query.plus_words) {
        if (document_to_word_freqs_.at(document_id).count(word) == 0) {
            continue;
        }
        if (SearchServer::document_to_word_freqs_.at(document_id).count(word)) {
            matched_words.push_back(word);
        }
    }

    for (const std::string_view word : query.minus_words) {

        if (SearchServer::document_to_word_freqs_.at(document_id).count(word)) {
            return { {},documents_.at(document_id).status };
        }
    }

    return { matched_words, documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, const std::string_view raw_query, int document_id) const {

    SearchServer::Query query = ParseQuery(raw_query,false);
    bool minus = std::none_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(),
        [this, document_id](std::string_view word) {
            return document_to_word_freqs_.at(document_id).count(word) != 0;
        });
    if (!minus) {
        return { {},documents_.at(document_id).status };
    }
    std::vector<std::string_view>matched_words(query.plus_words.size());

    auto it1 = std::copy_if(std::execution::par, std::make_move_iterator(query.plus_words.begin()), std::make_move_iterator(query.plus_words.end()),
        matched_words.begin(),
        [this, document_id](std::string_view word) {
            return (document_to_word_freqs_.at(document_id).count(word));
        });
    std::sort(std::execution::par, matched_words.begin(), it1);
    auto it = std::unique(std::execution::par, matched_words.begin(), it1);
    return { {matched_words.begin(),it }, documents_.at(document_id).status };
}

bool SearchServer::IsStopWord(const std::string_view word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string_view word) {
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view text) const {
    std::vector<std::string_view> words;
    for (const std::string_view word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word "s + std::basic_string(word) + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    return std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view word) const {
    if (word.empty()) {
        throw std::invalid_argument("Query word is empty"s);
    }
    bool is_minus = false;
    if (word[0] == '-') 
    {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw std::invalid_argument("Query word "s + std::basic_string(word) + " is invalid"s);
    }
    return { word, is_minus, IsStopWord(word) };
}
SearchServer::Query SearchServer::ParseQuery(const std::string_view text, bool isUnique) const {
    Query result;
    std::vector<std::string_view>Split = SplitIntoWords(text);
    for (const std::string_view word : Split) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.emplace_back(query_word.data);
            }
            else 
            {
                result.plus_words.emplace_back(query_word.data);
            }
        }
    }
    if(isUnique) {
        sort(result.minus_words.begin(), result.minus_words.end());
        auto itM = unique(result.minus_words.begin(), result.minus_words.end());
        result.minus_words.erase(itM, result.minus_words.end());
 
        sort(result.plus_words.begin(), result.plus_words.end());
        auto itP = unique(result.plus_words.begin(), result.plus_words.end());
        result.plus_words.erase(itP, result.plus_words.end());
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(std::basic_string(word)).size());
}