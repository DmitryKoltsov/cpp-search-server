#include "search_server.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

/* Подставьте вашу реализацию класса SearchServer сюда */

/*
   Подставьте сюда вашу реализацию макросов
   ASSERT, ASSERT_EQUAL, ASSERT_EQUAL_HINT, ASSERT_HINT и RUN_TEST
*/

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop wait a minute"s);
    }
}

void TestAddDoc()
{
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto res = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(res.size(), 1);
        ASSERT_EQUAL(res[0].id, doc_id);
    }
}

void TestMatchWord()
{
    const int doc_id = 42;
    const string content1 = "cat in the city"s;
    const string content2 = "cat in city"s;
    const vector<int> ratings = { 1, 2, 3 };

    {
        SearchServer server;
        server.SetStopWords("the"s);
        server.AddDocument(doc_id, content1, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id + 1, content2, DocumentStatus::ACTUAL, ratings);
        const auto [matched, status] = server.MatchDocument("in the city"s, doc_id);
        vector<string> words{ "city", "in" };
        ASSERT_EQUAL(matched, words);
    }
}

void TestMinusWord()
{
    const int doc_id = 42;
    const string content1 = "cat in the city"s;
    const string content2 = "cat in city"s;
    const vector<int> ratings = { 1, 2, 3 };

    {
        SearchServer server;
        server.AddDocument(doc_id, content1, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id + 1, content2, DocumentStatus::ACTUAL, ratings);
        const auto res = server.FindTopDocuments("in -the city"s);
        const Document& doc0 = res[0];
        ASSERT_EQUAL(res.size(), 1);
        ASSERT_EQUAL(doc0.id, doc_id + 1);
    }
}

void TestRel()
{
    const int doc_id_1 = 42;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = { 1, 2, 3 };

    const int doc_id_2 = 25;
    const string content_2 = "big yellow wolf "s;
    const vector<int> ratings_2 = { 4, 5, 6 };

    const int doc_id_3 = 26;
    const string content_3 = "rabbit on small house "s;
    const vector<int> ratings_3 = { 7, 8, 9 };

    {
        SearchServer server;
        server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        const auto res = server.FindTopDocuments("cat wolf on "s);
        vector<int> order{ 25,26,42 };
        ASSERT_EQUAL(res.size(), 3);
        for (size_t i = 0; i < res.size(); ++i)
        {
            ASSERT_EQUAL(order[i], res[i].id);
        }
    }
}

void TestRating()
{
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto res = server.FindTopDocuments("city cat"s);
        ASSERT_EQUAL(res[0].rating, round((1 + 2 + 3) / 3));
    }
}

void TestReleCalc()
{
    SearchServer server;
    const double EPSILON2 = 1e-6;
    server.AddDocument(0, "cat cat city dog"s, DocumentStatus::ACTUAL, { 1 });
    server.AddDocument(1, "city dog"s, DocumentStatus::ACTUAL, { 1 });
    server.AddDocument(2, "cat city potato"s, DocumentStatus::ACTUAL, { 1 });


    {
        const auto& res = server.FindTopDocuments("cat"s);

        double expected_relevance_doc_0 = std::log(static_cast<double>(server.GetDocumentCount()) / 2.0) * (2.0 / 4.0);
        double expected_relevance_doc_2 = std::log(static_cast<double>(server.GetDocumentCount()) / 2.0) * (1.0 / 3.0);

        ASSERT_EQUAL(res.size(), 2);

        ASSERT(abs(res[0].relevance - expected_relevance_doc_0) < EPSILON2);
        ASSERT(abs(res[1].relevance - expected_relevance_doc_2) < EPSILON2);
    }

    {
        const auto& res = server.FindTopDocuments("city"s);

        ASSERT_EQUAL(res.size(), 3);

        ASSERT(std::abs(res[0].relevance) < EPSILON2);
        ASSERT(std::abs(res[1].relevance) < EPSILON2);
        ASSERT(std::abs(res[2].relevance) < EPSILON2);
    }
}

void TestStatus()
{
    const int doc_id_1 = 42;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = { 1, 2, 3 };

    const int doc_id_2 = 25;
    const string content_2 = "big yellow wolf "s;
    const vector<int> ratings_2 = { 4, 5, 6 };

    const int doc_id_3 = 26;
    const string content_3 = "rabbit on small house "s;
    const vector<int> ratings_3 = { 7, 8, 9 };

    SearchServer server;
    server.AddDocument(doc_id_1, content_1, DocumentStatus::BANNED, ratings_1);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::REMOVED, ratings_2);
    server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
    const auto res = server.FindTopDocuments("cat wolf rabbit", DocumentStatus::ACTUAL);
    ASSERT_EQUAL(res.size(), 1);
    ASSERT_EQUAL(res[0].id, doc_id_3);
}

void TestPred()
{
    const int doc_id_1 = 42;
    const string content_1 = "cat in the city"s;
    const vector<int> ratings_1 = { 1, 2, 3 };

    const int doc_id_2 = 25;
    const string content_2 = "big yellow wolf "s;
    const vector<int> ratings_2 = { 4, 5, 6 };

    const int doc_id_3 = 26;
    const string content_3 = "rabbit on small house "s;
    const vector<int> ratings_3 = { 7, 8, 9 };

    SearchServer server;
    server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
    server.AddDocument(doc_id_2, content_2, DocumentStatus::REMOVED, ratings_2);
    server.AddDocument(doc_id_3, content_3, DocumentStatus::BANNED, ratings_3);
    const auto res = server.FindTopDocuments("cat"s, [](int document_id, DocumentStatus, int) {return document_id % 2 == 0; });
    ASSERT_EQUAL(res.size(), 1);
    ASSERT_EQUAL(res[0].id, doc_id_1);
}

/*
Разместите код остальных тестов здесь
*/

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDoc);
    RUN_TEST(TestMatchWord);
    RUN_TEST(TestMinusWord);
    RUN_TEST(TestRel);
    RUN_TEST(TestRating);
    RUN_TEST(TestReleCalc);
    RUN_TEST(TestStatus);
    RUN_TEST(TestPred);
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}