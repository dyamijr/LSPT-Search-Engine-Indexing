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

void addIndex(auto dbClient, string index, string docId, int frequency, vector<int> positions){
    // Step 1: Query to find the document with the given index and DocId
    auto db = dbClient["IndexingDB"];
    auto indexTable = db["indextable"];
    auto query = findIndexAndDoc(indexTable, index, docId);
    
    // Step 2: Define the update operation for updating the frequency and positions
    auto update = bsoncxx::builder::stream::document{}
                  << "$set" << bsoncxx::builder::stream::open_document
                  << "Documents.$.frequency" << frequency
                  << bsoncxx::builder::stream::close_document;

    update << "$push" << bsoncxx::builder::stream::open_document
           << "Documents.$.positions" << bsoncxx::builder::stream::open_array;
    for (int pos : positions) {
        update << pos;
    }
    update << bsoncxx::builder::stream::close_array
           << bsoncxx::builder::stream::close_document
           << bsoncxx::builder::stream::finalize;

    // Step 3: If no matching document found for update, add a new document
    auto addNewDoc = bsoncxx::builder::stream::document{}
                       << "$push" << bsoncxx::builder::stream::open_document
                       << "Documents" << bsoncxx::builder::stream::open_document
                       << "DocId" << docId
                       << "frequency" << frequency
                       << "positions" << bsoncxx::builder::stream::open_array;
    for (int pos : positions) {
        addNewDoc << pos;
    }
    addNewDoc << bsoncxx::builder::stream::close_array
              << bsoncxx::builder::stream::close_document
              << bsoncxx::builder::stream::close_document
              << bsoncxx::builder::stream::finalize;

    // Step 4: Check if the document with index and DocId exists, and update or insert
    auto result = indexTable.update_one(query.view(), update.view());

    if ( !(result && result->modified_count() > 0)) {
        // If no matching document was updated, check if the index exists
        auto indexQuery = bsoncxx::builder::stream::document{}
                           << "index" << index
                           << bsoncxx::builder::stream::finalize;

        // Try inserting the new document if the index exists but DocId does not
        result = indexTable.update_one(indexQuery.view(), addNewDoc.view());
        if ( !(result && result->modified_count() > 0) ) {
            // If index does not exist, insert the full document
            auto fullInsert = bsoncxx::builder::stream::document{}
                               << "index" << index
                               << "Documents" << bsoncxx::builder::stream::open_array
                               << bsoncxx::builder::stream::open_document
                               << "DocId" << docId
                               << "frequency" << frequency
                               << "positions" << bsoncxx::builder::stream::open_array;
            for (int pos : positions) {
                fullInsert << pos;
            }
            fullInsert << bsoncxx::builder::stream::close_array
                       << bsoncxx::builder::stream::close_document
                       << bsoncxx::builder::stream::close_array
                       << bsoncxx::builder::stream::finalize;

            // Insert the new document
            indexTable.insert_one(fullInsert.view());
        }
    }
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