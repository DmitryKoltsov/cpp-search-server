#pragma once
#include <vector>
#include <ostream>
#include <cassert>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange() = default;

    IteratorRange(Iterator begin, Iterator end)
        : first_(begin), last_(end), size_(distance(first_, last_))
    {
    }

    Iterator begin() const {
        return first_;
    }

    Iterator end() const {
        return last_;
    }

    int size() const {
        return size_;
    }

private:
    Iterator first_;
    Iterator last_;
    int size_;
};

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, int size) 
    {
        assert(end >= begin && size > 0);
        for (int left = distance(begin, end); left > 0;) {
            const size_t current_page_size = std::min(size, left);
            const Iterator current_page_end = next(begin, current_page_size);
            pages_.push_back({ begin, current_page_end });
            left -= current_page_size;
            begin = current_page_end;
        }
    }

    auto begin() const {
        return pages_.begin();
    }

    auto end() const {
        return pages_.end();
    }

    int size() const {
        return pages_.size();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& range) {
    for (Iterator it = range.begin(); it != range.end(); ++it) {
        out << *it;
    }
    return out;
}

template <typename Container>
auto Paginate(const Container& c, int page_size) {
    return Paginator(begin(c), end(c), page_size);
}