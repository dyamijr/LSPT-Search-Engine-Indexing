#include <gtest/gtest.h>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>

/* retrieve_document_by_indexID()
 *
 * This function queries the inverted index to retrieve all document IDs
 * associated with a given index ID. It ensures that the results are accurate
 * and relevant to the specified index ID.
 *
 * Parameters:
 *   index_ID (const std::string&):
 *       The unique identifier for the index being queried. 
 *
 *   database_connection (const mongocxx::database&):
 *       A valid connection to the MongoDB database. The function will use this connection
 *       to query the inverted index collection.
 *
 * Functionality:
 *   - This function interacts with the MongoDB database to retrieve the document IDs
 *     associated with the specified `index_ID`.
 *   - Steps performed:
 *       1. Query the inverted index collection in the database for the provided `index_ID`.
 *       2. Extract and return the list of document IDs that are associated with the given index ID.
 *       3. If the `index_ID` does not exist in the database, the function should return an empty list.
 *   - The function should ensure that the query results are complete and no duplicates are returned.
 *
 * Error Handling:
 *   - If the MongoDB query fails (e.g., due to connection issues), the function must:
 *       - Log the error message.
 *       - Return an empty result.
 *   - If the `index_ID` does not exist in the database, the function must return an empty result
 *     but should log a warning for debugging purposes.
 *
 * Return:
 *   - Returns a `std::vector<std::string>` containing the document IDs associated with the
 *     specified `index_ID`.
 *       - If the query succeeds, the vector will contain all document IDs (unique).
 *       - If the query fails or the `index_ID` is invalid/non-existent, the vector will be empty.
 *
 * Examples of Usage:
 *   - Retrieve documents associated with an index:
 *       std::vector<std::string> doc_ids = retrieve_document_by_indexID("index123", db_connection);
 */
std::vector<std::string> retrieve_document_by_indexID(const std::string& index_ID,
                                                      const mongocxx::database& database_connection);


/* Test Database Configuration */
// Initialize a MongoDB instance.
// This is required to set up the MongoDB driver in the current process.
// Without initializing `mongocxx::instance`, no MongoDB operations can be performed.
mongocxx::instance instance{}; 
// Create a MongoDB client connected to the local MongoDB instance.
// This client will be used to interact with the test database during unit tests.
// The URI specifies the MongoDB instance to connect to, in this case, "localhost" on the default port (27017).
mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
// Access the test database named "test_retrieve_documents".
// Using a dedicated test database ensures that the tests do not interfere with the production database
// and provides a controlled environment for testing isolated components.
mongocxx::database test_db = client["test_retrieve_documents"];

/* Helper function to clear test database collections */
// Clear all documents in the "inverted_index" collection of the test database.
// This function is necessary to ensure that each test starts with a clean and predictable state.
// By removing all documents before and after each test, we prevent any unintended effects
// caused by leftover data from previous tests, ensuring test isolation.
void ClearTestDatabase() {
    // Remove all entries in the "inverted_index" collection.
    test_db["inverted_index"].delete_many({});
}

/* Helper function to insert documents into the inverted index for testing */
// Inserts a list of document IDs into the inverted index under a specific index ID.
// This function is essential for setting up the initial state of the database for unit tests.
// By adding predefined entries to the database, we can control the environment and test the
// behavior of the function `retrieve_document_by_indexID` under known conditions.
//
// Parameters:
// - `index_ID`: The unique identifier for the index entry (e.g., a term or key).
// - `doc_ids`: A vector of document IDs associated with the given index ID.
//
// Example Usage:
//   InsertIntoInvertedIndex("index123", {"doc1", "doc2", "doc3"});
void InsertIntoInvertedIndex(const std::string& index_ID, const std::vector<std::string>& doc_ids) {
    // Access the "inverted_index" collection.
    auto inverted_index = test_db["inverted_index"];
    // Build the MongoDB document to insert.
    bsoncxx::builder::stream::document document_builder;
    auto postings = document_builder << "index_ID" << index_ID << "documents" << bsoncxx::builder::stream::open_array;
    // Add each document ID to the "documents" array field.
    for (const auto& doc_id : doc_ids) {
        postings = postings << doc_id;
    }
    postings << bsoncxx::builder::stream::close_array;
    // Insert the document into the "inverted_index" collection.
    inverted_index.insert_one(document_builder.view());
}

/* Helper function to check if a document ID exists in the retrieved list */
// Checks if a specific document ID exists within a given list of retrieved document IDs.
// This function is used to validate the output of `retrieve_document_by_indexID` during unit tests.
// By comparing the retrieved results with expected values, this function ensures that
// the correct document IDs are being returned by the function.
//
// Parameters:
// - `doc_list`: The list of document IDs returned by the `retrieve_document_by_indexID` function.
// - `doc_id`: The document ID to search for within the list.
//
// Returns:
// - `true` if the document ID exists in the list, `false` otherwise.
//
// Example Usage:
//   std::vector<std::string> doc_list = {"doc1", "doc2"};
//   bool exists = DocumentIDExistsInList(doc_list, "doc1"); // Returns true.
bool DocumentIDExistsInList(const std::vector<std::string>& doc_list, const std::string& doc_id) {
    // Use the standard library's `find` algorithm to search for the document ID in the list.
    return std::find(doc_list.begin(), doc_list.end(), doc_id) != doc_list.end();
}

/* Unit Test Cases */

// Test Suite for retrieve_document_by_indexID
class RetrieveDocumentByIndexIDTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clear the database before each test
        ClearTestDatabase();
    }

    void TearDown() override {
        // Clear the database after each test
        ClearTestDatabase();
    }
};

// Test Case 1: Retrieving documents for a valid index_ID
TEST_F(RetrieveDocumentByIndexIDTest, RetrieveValidIndexID) {
    /*
      Test Description:
      - This test verifies that the function correctly retrieves document IDs
        associated with a valid index_ID from the database.
    */
    std::string index_ID = "index123";
    std::vector<std::string> expected_doc_ids = {"doc1", "doc2", "doc3"};
    InsertIntoInvertedIndex(index_ID, expected_doc_ids);

    std::vector<std::string> retrieved_doc_ids = retrieve_document_by_indexID(index_ID, test_db);

    ASSERT_EQ(retrieved_doc_ids.size(), expected_doc_ids.size()) << "Mismatch in the number of retrieved document IDs.";
    for (const auto& doc_id : expected_doc_ids) {
        ASSERT_TRUE(DocumentIDExistsInList(retrieved_doc_ids, doc_id))
            << "Document ID " << doc_id << " was not retrieved for index_ID: " << index_ID;
    }
}

// Test Case 2: Retrieving documents for a non-existent index_ID
TEST_F(RetrieveDocumentByIndexIDTest, RetrieveNonExistentIndexID) {
    /*
      Test Description:
      - This test verifies that the function returns an empty list when the provided
        index_ID does not exist in the database.
    */
    std::string index_ID = "non_existent_index";

    std::vector<std::string> retrieved_doc_ids = retrieve_document_by_indexID(index_ID, test_db);

    ASSERT_TRUE(retrieved_doc_ids.empty()) << "Function should return an empty list for a non-existent index_ID.";
}

// Test Case 3: Retrieving documents for an index_ID with no associated documents
TEST_F(RetrieveDocumentByIndexIDTest, RetrieveIndexIDWithNoDocuments) {
    /*
      Test Description:
      - This test verifies that the function returns an empty list when the provided
        index_ID exists but has no associated document IDs.
    */
    std::string index_ID = "index_no_docs";
    InsertIntoInvertedIndex(index_ID, {}); // Insert an empty list of documents

    std::vector<std::string> retrieved_doc_ids = retrieve_document_by_indexID(index_ID, test_db);

    ASSERT_TRUE(retrieved_doc_ids.empty()) << "Function should return an empty list for an index_ID with no documents.";
}

// Test Case 4: Retrieving documents with duplicate document IDs 
TEST_F(RetrieveDocumentByIndexIDTest, RetrieveIndexIDWithDuplicateDocuments) {
    /*
      Test Description:
      - This test verifies that the function handles cases where duplicate document
        IDs are present in the database. It should return unique document IDs.
    */
    std::string index_ID = "index_with_duplicates";
    std::vector<std::string> duplicate_doc_ids = {"doc1", "doc1", "doc2", "doc2"};
    InsertIntoInvertedIndex(index_ID, duplicate_doc_ids);

    std::vector<std::string> retrieved_doc_ids = retrieve_document_by_indexID(index_ID, test_db);

    ASSERT_EQ(retrieved_doc_ids.size(), 2) << "Function should return unique document IDs.";
    ASSERT_TRUE(DocumentIDExistsInList(retrieved_doc_ids, "doc1"))
        << "Document ID 'doc1' is missing in the retrieved list.";
    ASSERT_TRUE(DocumentIDExistsInList(retrieved_doc_ids, "doc2"))
        << "Document ID 'doc2' is missing in the retrieved list.";
}

// Test Case 5: Retrieving documents for an invalid index_ID (empty string)
TEST_F(RetrieveDocumentByIndexIDTest, RetrieveInvalidIndexID) {
    /*
      Test Description:
      - This test verifies that the function handles invalid input, such as an
        empty string for the index_ID, by returning an empty list.
    */
    std::string index_ID = ""; // Invalid index_ID

    std::vector<std::string> retrieved_doc_ids = retrieve_document_by_indexID(index_ID, test_db);

    ASSERT_TRUE(retrieved_doc_ids.empty()) << "Function should return an empty list for an invalid index_ID.";
}

