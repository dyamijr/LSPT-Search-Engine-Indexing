#include "database.h"
#include <string>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <iostream>

using namespace std;

bool pingIndex(string doc_ID, string operation){
    if(operation == "add"){
        return addToIndex(doc_ID);
    } else if (operation == "remove"){
        return removeFromIndex(doc_ID);
    } else if (operation == "update"){
        return updateIndex(doc_ID);
    } else {
        return false;
    }
}

bool addIndexToDatabase(string dbConnectionString, string index, string docId, string frequency, string position){
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
                        << "Documents.$.positions" << position  // Append new position
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
                        << position // New positions
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
                      << position
                      << bsoncxx::builder::stream::close_array
                      << bsoncxx::builder::stream::close_document
                      << bsoncxx::builder::stream::close_array;
    auto newResult = indexTable.insert_one(new_entry_builder.view());
    if (newResult) {
        return true;
    } 
    return false;
}

bool addToIndex(string doc_ID) {
    if (doc_ID.empty()) {
        cerr << "Invalid input: doc_ID is empty." << endl;
        return false;
    }
    // Connect to the metadata table of the database
    string connect = "mongodb+srv://dyamiwatsonjr:LSPTTeamx@lspt.xq5ap.mongodb.net/?retryWrites=true&w=majority&appName=LSPT";
    mongocxx::client dbClient = mongocxx::client{mongocxx::uri{connect}};
    auto db = dbClient["IndexingDB"];
    auto metadata = db["metadata"];

    // Connect to Document Data Store(DDS) Database
    mongocxx::client ddsDBClient = mongocxx::client{mongocxx::uri{"mongodb://128.113.126.79:27017"}};
    auto ddsDB = ddsDBClient["test"]; // dbClient["testdb"]; // Uncomment for testing
    auto transformed = ddsDB["TRANSFORMED"];

    //query command for DDS database
    auto query = bsoncxx::builder::stream::document{} << "doc_id" << doc_ID << bsoncxx::builder::stream::finalize;

    try {
        //Searches for the document and if found add associated tokens, bigrams, and trigrams

        auto result = transformed.find_one(query.view());
        if (!result) {
            cerr << "Document not found for ID: " << doc_ID << endl;
            return false;
        }

        auto transformedData = result->view();

        bsoncxx::array::view tokenList = transformedData["tokens"].get_array().value;
        for (auto&& token : tokenList) {
            auto tokenView = token.get_document().view();
            string index = tokenView["token"].get_utf8().value.to_string();
            int frequency = tokenView["frequency"].get_int32().value;
            int position = tokenView["position"].get_int32().value;
            addIndexToDatabase(connect, index, doc_ID, to_string(frequency), to_string(position));
        }
        bsoncxx::array::view bigrams = transformedData["bigrams"].get_array().value;
        for (auto&& bigram : bigrams) {
            auto bigramView = bigram.get_document().view();
            bsoncxx::array::view bigramArray = bigramView["bigram"].get_array().value;
            string indexTerm = bigramArray[0].get_utf8().value.to_string() + bigramArray[1].get_utf8().value.to_string();
            int frequency = bigramView["frequency"].get_int32().value;
            addIndexToDatabase(connect, indexTerm, doc_ID, to_string(frequency), "");
        }
        bsoncxx::array::view trigrams = transformedData["trigrams"].get_array().value;
        for (auto&& trigram : trigrams) {
            auto trigramView = trigram.get_document().view();
            bsoncxx::array::view trigramArray = trigramView["trigram"].get_array().value;
            string indexTerm = trigramArray[0].get_utf8().value.to_string() + trigramArray[1].get_utf8().value.to_string()
                                + trigramArray[2].get_utf8().value.to_string();
            int frequency = trigramView["frequency"].get_int32().value;
            addIndexToDatabase(connect, indexTerm, doc_ID, to_string(frequency), "");
        }
        bsoncxx::builder::stream::document metadata_builder;
        metadata_builder << "DocId" << doc_ID
                         << "total_length" << transformedData["total_length"].get_int32().value;
        metadata.insert_one(metadata_builder.view());

        return true;

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return false;
    }
}

bool removeFromIndex(string doc_ID) {
    // Step 1: Connect to the database and collection
    string connect = "mongodb+srv://dyamiwatsonjr:LSPTTeamx@lspt.xq5ap.mongodb.net/?retryWrites=true&w=majority&appName=LSPT";
    mongocxx::client dbClient = mongocxx::client{mongocxx::uri{connect}};
    auto db = dbClient["IndexingDB"];
    auto indexTable = db["indextable"];
    auto metadata = db["metadata"];

    // Step 2: Define the query to find documents containing the specified docId
    try{
        auto indexQuery = bsoncxx::builder::stream::document{}
                 << "Documents.DocId" << doc_ID
                 << bsoncxx::builder::stream::finalize;
        auto metadataQuery = bsoncxx::builder::stream::document{}
                    << "DocId" << doc_ID
                    << bsoncxx::builder::stream::finalize;

        // Step 3: Perform the delete operation to remove matching documents
        auto indexResult = indexTable.delete_many(indexQuery.view());
        auto metadataResult = metadata.delete_many(metadataQuery.view());

        // Step 4: Handle cases where no matching document was found
        if (indexResult && indexResult->deleted_count() > 0 && metadataResult && metadataResult->deleted_count() > 0) {
            cout << "Successfully removed " << indexResult->deleted_count()
                    << " document(s) containing DocId: " << doc_ID << "." << endl;
        } else {
            cout << "No document found with DocId: " << doc_ID << " to remove." << endl;
        }
        return true;
    }  catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return false;
    }
    
}

bool updateIndex(string doc_ID) {
    try{
        // Attempts to remove data and if no errors occur re add the document
        bool remove = removeFromIndex(doc_ID);
        if (!remove){
            return false;
        }
        return addToIndex(doc_ID);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return false;
    }
}
