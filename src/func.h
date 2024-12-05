#ifndef FUNC_H
#define FUNC_H

#include <string>
#include <unordered_map>
#include <crow/json.h>

// Function declarations - to be changed later to match func.cpp
void setup_example_data();
crow::json::wvalue pingIndex(const std::string& doc_ID, const std::string& operation, const std::string& timestamp);
crow::json::wvalue getDocsFromIndex(const std::string& index_ID);
crow::json::wvalue getDocumentMetadata(const std::string& doc_ID);
int get_average_doc_length();

#endif
