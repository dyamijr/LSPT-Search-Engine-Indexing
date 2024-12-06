#include "database.h"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <iostream>
#include <mutex>

using namespace std;

bool addIndex(string dbConnectionString, string index, string docId, int frequency, int position){
    // Step 1: Query to find the document with the given index and DocId
    mongocxx::client dbClient = mongocxx::client{mongocxx::uri{dbConnectionString}};
    auto db = dbClient["IndexingDB"];
    auto indexTable = db["indextable"];
    //auto query = findIndexAndDoc(indexTable, index, docId);
    
    //New logic
    auto query = bsoncxx::builder::stream::document{}
                 << "index" << index
                 << "Documents.DocId" << docId
                 << bsoncxx::builder::stream::finalize;
    
    auto result = indexTable.find_one(query.view());
    //CHECK IF DOC AND INDEX EXIST
    if (result){
        bsoncxx::builder::stream::document update_builder;
        update_builder << "$push" << bsoncxx::builder::stream::open_document
                        << "Documents.$.positions" << to_string(position)  // Append new position
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
                        << to_string(position) // New positions
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
                      << to_string(position)
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
