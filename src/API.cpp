#include "crow.h"
#include "database.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>

using namespace std;

// Simulated in-memory data storage
mutex data_mutex; // Ensures thread safety

int main() {
    crow::SimpleApp app;

    // Route: Root directlry
    CROW_ROUTE(app, "/")([](){
        return "Hello! This is Team X's landing site.";
    });

    // Route: /pingIndex (POST)
    CROW_ROUTE(app, "/pingIndex").methods("POST"_method)([](const crow::request& req) {

        auto json_body = crow::json::load(req.body);

        if (!json_body || !json_body.has("doc_ID") || !json_body.has("operation") || !json_body.has("timestamp")) {
            return crow::response(400, "Invalid JSON");
        }

        crow::json::wvalue response_body;

        try {
            std::string doc_ID = json_body["doc_ID"].s();
            std::string operation = json_body["operation"].s();
            std::string timestamp = json_body["timestamp"].s();

            // Call the pingIndex function from database.cpp
            bool success = pingIndex(doc_ID, operation);

            {
                std::lock_guard<std::mutex> lock(data_mutex);

                if (!success) {
                    response_body["status"] = "error";
                    response_body["message"] = "Failed to process the ping operation";
                    return crow::response(404, response_body);
                }

                // Responses
                response_body["status"] = "success";
                response_body["received_doc_ID"] = doc_ID;
                response_body["received_operation"] = operation;
                response_body["received_timestamp"] = timestamp;
                response_body["message"] = "Ping operation processed successfully";
                
                return crow::response(200, response_body);
            }

        }
        
        catch (const std::exception& e) {
            response_body["status"] = "error";
            response_body["message"] = e.what();
            return crow::response(400, response_body);
        }
    });

        // Route: /getDocsFromIndex (GET)
    CROW_ROUTE(app, "/getDocsFromIndex").methods("GET"_method)([](const crow::request& req) {

        std::string index_ID = req.url_params.get("index_ID");
        
        if (index_ID.empty()) {
            return crow::response(400, "Missing index_ID");
        }

        crow::json::wvalue response_body;
        
        try {
            response_body["recieved_index"] = index_ID;
            return crow::response(202, response_body);
            /*auto documents = getDocsFromIndex(index_ID);

            if (documents.empty()) {
                return crow::response(404, "Index not found");
            }

            crow::json::wvalue docs_json;
            for (const auto& [doc_ID, content] : documents) {
                docs_json[doc_ID] = content;
            }

            response_body["documents"] = docs_json;
            return crow::response(200, response_body);*/
        }
        
        catch (const std::exception& e) {
            response_body["message"] = e.what();
            return crow::response(500, response_body);
        }
    });

    // Route: /getDocumentMetaData (GET)
    CROW_ROUTE(app, "/getDocumentMetaData").methods("GET"_method)([](const crow::request& req) {

        std::string doc_ID = req.url_params.get("doc_ID");

        if (doc_ID.empty()) {
            return crow::response(400, "Missing doc_ID");
        }

        crow::json::wvalue response_body;

        try {
            response_body["received_doc_ID"] = doc_ID;
            return crow::response(202, response_body);
            /*auto metadata = getDocumentMetaData(doc_ID); 

            if (metadata.empty()) {
                return crow::response(404, "Document metadata not found");
            }

            response_body["metadata"] = metadata;
            return crow::response(200, response_body);*/
        }
        
        catch (const std::exception& e) {
            response_body["message"] = e.what();
            return crow::response(500, response_body);
        }
    });

    // Route: /getAverageDocLength (GET)
    CROW_ROUTE(app, "/getAverageDocLength").methods("GET"_method)([]() {

        crow::json::wvalue response_body;
        
        try {
            response_body["average_length"] = "0";
            return crow::response(202, response_body);
            /*int average_length = getAverageDocLength(); 

            response_body["average_length"] = average_length;
            return crow::response(200, response_body);*/
        }

        catch (const std::exception& e) {
            response_body["message"] = e.what();
            return crow::response(500, response_body);
        }
    });

    // Start the Crow server
    std::cout << "Server is running on http://localhost:8080\n";
    app.port(8080).multithreaded().run();

    return 0;
}