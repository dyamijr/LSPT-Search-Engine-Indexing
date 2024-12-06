#include "database.h"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <iostream>
using namespace std;

auto findIndexAndDoc(auto indexTable, string index, string docId){
    // Query the document by index and docId field
    auto query = bsoncxx::builder::stream::document{}
                 << "index" << index
                 << "Documents.DocId" << docId
                 << bsoncxx::builder::stream::finalize;

    // Find the document
    auto result = indexTable.find_one(query.view());
    
    return result; 
}

bool addIndex(auto dbClient, string index, string docId, int frequency, vector<int> positions){
    // Step 1: Query to find the document with the given index and DocId
    auto db = dbClient["IndexingDB"];
    auto indexTable = db["indextable"];
    //auto query = findIndexAndDoc(indexTable, index, docId);
    
    //New logic
    auto query = bsoncxx::builder::stream::document{}
                 << "index" << index
                 << "Documents.DocId" << docId
                 << bsoncxx::builder::stream::finalize;
    
    auto result = indexTable.find_one(query.view());
    int pos = 0;
    //CHECK IF DOC AND INDEX EXIST
    if (result){
        bsoncxx::builder::stream::document update_builder;
        update_builder << "$push" << bsoncxx::builder::stream::open_document
                        << "Documents.$.positions" << to_string(pos)  // Append new position
                        << bsoncxx::builder::stream::close_document;
        auto newResult = indexTable.update_one(query.view(), update_builder.view());
        if(newResult && newResult->matched_count() > 0){
            return true;
        } else {
            return false;
        }
    }
    //CHECK IF INDEX EXIST
    query = bsoncxx::builder::stream::document{}
                 << "index" << index
                 << bsoncxx::builder::stream::finalize;
    result = indexTable.find_one(query.view());
    if (result){
        bsoncxx::builder::stream::document new_doc_builder;
        new_doc_builder << "DocId" << docId  // New document ID
                        << "frequency" << frequency
                        << "positions" << bsoncxx::builder::stream::open_array
                        << to_string(pos) // New positions
                        << bsoncxx::builder::stream::close_array;
        bsoncxx::builder::stream::document update_builder;
        update_builder << "$push" << bsoncxx::builder::stream::open_document
                    << "Documents" << new_doc_builder.view()  // Append new document
                    << bsoncxx::builder::stream::close_document;
        auto newResult = indexTable.update_one(query.view(), update_builder.view());
        if(newResult && newResult->matched_count() > 0){
            return true;
        } else {
            return false;
        }
    }
    bsoncxx::builder::stream::document new_entry_builder;
    new_entry_builder << "index" << index  // New 'index' value
                      << "Documents" << bsoncxx::builder::stream::open_array
                      << bsoncxx::builder::stream::open_document
                      << "DocId" << docId
                      << "frequency" << frequency
                      << "positions" << bsoncxx::builder::stream::open_array
                      << to_string(pos)
                      << bsoncxx::builder::stream::close_array
                      << bsoncxx::builder::stream::close_document
                      << bsoncxx::builder::stream::close_array;
    auto newResult = indexTable.insert_one(new_entry_builder.view());
    if (newResult) {
        return true;
    } 
    return false;
}

void removeDoc(auto dbClient, string docId){
    auto db = dbClient["IndexingDB"];
    auto indexTable = db["indextable"];
    // Define the update operation using $pull
    auto update = bsoncxx::builder::stream::document{}
                  << "$pull" << bsoncxx::builder::stream::open_document
                  << "Documents" << bsoncxx::builder::stream::open_document
                  << "DocId" << docId
                  << bsoncxx::builder::stream::close_document
                  << bsoncxx::builder::stream::close_document
                  << bsoncxx::builder::stream::finalize;

    // Perform the update operation
    auto result = indexTable.update_many({}, update.view());
}

mongocxx::client connectToDatabase(string dbConnectionString){
    mongocxx::instance instance{}; // Initialize MongoDB driver
    mongocxx::client client{mongocxx::uri{dbConnectionString}};
    return client;
}

/*
int main()
{
    mongocxx::instance instance{}; // Initialize MongoDB driver
    mongocxx::client client{mongocxx::uri{"mongodb+srv://<Username>:<password>@lspt.xq5ap.mongodb.net/?retryWrites=true&w=majority&appName=LSPT"}};

    auto db = client["IndexingDB"];
    auto index = db["indextable"];
    auto docs = db["documentmetada"];

    // Insert a document
    bsoncxx::builder::stream::document document{};
    document << "name" << "John Doe" << "age" << 30;
    collection.insert_one(document.view());

    // Retrieve documents
    auto cursor = collection.find({});
    for (auto&& doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << std::endl;
    }

    return 0;
}*/