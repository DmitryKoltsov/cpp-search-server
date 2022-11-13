#include"string_processing.h"

using namespace std;

vector<string_view> SplitIntoWords(string_view text)
{

    vector<string_view> words;
    string word;
    auto pos = text.find(' ');
    size_t first = 0;
    while (pos != text.npos)
    {
        words.push_back(text.substr(first, pos - first));
        first = pos + 1;
        pos = text.find(' ', first);
    }
    words.push_back(text.substr(first, pos - first));
    return words;
}