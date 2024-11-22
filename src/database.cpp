#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <iostream>
using namespace std;

int main()
{
    mongocxx::instance instance{}; // Initialize MongoDB driver
    mongocxx::client client{mongocxx::uri{"mongodb+srv://dyamiwatsonjr:LSPTX@lspt.xq5ap.mongodb.net/?retryWrites=true&w=majority&appName=LSPT"}};

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
}