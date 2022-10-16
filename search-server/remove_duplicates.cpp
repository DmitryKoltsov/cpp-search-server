#include "remove_duplicates.h"

using namespace std;

void RemoveDuplicates(SearchServer & search_server) 
{
    set<set<string>> document;
    vector<int> deleted_document;
    
    for (auto document_id : search_server) 
    {
        set<string> word_in_document;
        for (auto & [word, freq] : search_server.GetWordFrequencies(document_id)) 
        {
            word_in_document.insert(word);
        }
        
    if(document.find(word_in_document) == document.end())
       {
        document.insert(word_in_document);
       }
       else
       {
        deleted_document.push_back(document_id);
       }
    }
   for (int i : deleted_document) 
   {
   search_server.RemoveDocument(i);
   cout << "Found duplicate document id " << i << endl;
   }
}