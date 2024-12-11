
#include <gtest/gtest.h>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>

/* remove_document()
 *
 * This function removes a document's references from the inverted index and deletes
 * its metadata entry in the document metadata database. It ensures that all references
 * to the document are completely removed from the system.
 *
 * Parameters:
 *   doc_id (const std::string&):
 *       The unique identifier for the document to be removed.
 *
 * Functionality:
 *   - The function interacts with the MongoDB database to perform the following:
 *       1. Locate and delete all entries in the inverted index associated with
 *          the given `doc_id`.
 *       2. Delete the corresponding document metadata entry from the document
 *          metadata database.
 *   - Ensure that all database operations are atomic. If any operation fails
 *     (e.g., deletion of an index entry or metadata entry), the function must
 *     log the error, abort further operations, and return a failure status.
 *   - The function must return a success status only if all database operations
 *     complete successfully.
 *
 * Error Handling:
 *   - If any database operation fails (e.g., due to a database connection error
 *     or a query issue), the function must:
 *       - Log the error message.
 *       - Roll back any partial changes if possible.
 *       - Return false.
 *
 * Return:
 *   - Returns a boolean value:
 *       - `true` if all database operations (inverted index deletion and
 *         document metadata deletion) complete successfully.
 *       - `false` if any error occurs during the operation or if the `doc_id`
 *         does not exist in the database.
 *
 * Examples of Usage:
 *   - Removing a document from the index:
 *       remove_document("doc123");
 */
bool remove_document(const std::string& doc_id);


/* Test Database Configuration */
// Initialize a MongoDB instance.
// This is required to set up the MongoDB driver in the current process.
// Without initializing `mongocxx::instance`, no MongoDB operations can be performed.
mongocxx::instance instance{}; 
// Create a MongoDB client connected to the local MongoDB instance.
// This client will be used to interact with the test database during unit tests.
// The URI specifies the MongoDB instance to connect to, in this case, "localhost" on the default port (27017).
mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
// Access the test database named "test_remove_document".
// Using a dedicated test database ensures that the tests do not interfere with the production database
// and provides a controlled environment for testing isolated components.
mongocxx::database test_db = client["test_remove_document"];

/* Helper function to clear test database collections */
// Clears all documents from the "inverted_index" and "document_metadata" collections in the test database.
// This function is necessary to ensure that each test starts with a clean and consistent database state.
void ClearTestDatabase() {
    // Clear all entries in the inverted index collection.
    test_db["inverted_index"].delete_many({});
    // Clear all entries in the document metadata collection.
    test_db["document_metadata"].delete_many({});
}


/* Helper function to check if a document exists in the inverted index */
// Checks if a specific document ID is associated with a given term in the inverted index.
// This function is essential for validating the state of the database during unit tests.
// It helps confirm whether the inverted index was updated correctly after executing `remove_document`.
//
// Parameters:
// - `term`: The term to search for in the inverted index.
// - `doc_id`: The document ID to check for within the postings list of the term.
//
// Returns:
// - `true` if the specified term and document ID exist in the inverted index, `false` otherwise.
//
// Example Usage:
//   bool exists = DocumentExistsInIndex("term1", "doc123");
//   ASSERT_TRUE(exists) << "Document ID 'doc123' should exist under term 'term1'.";
bool DocumentExistsInIndex(const std::string& term, const std::string& doc_id) {
    // Access the "inverted_index" collection in the test database.
    auto inverted_index = test_db["inverted_index"];
    // Build a query to search for the term and associated document ID in the postings list.
    auto result = inverted_index.find_one(
        bsoncxx::builder::stream::document{}
            << "term" << term                        // Match the term.
            << "postings.doc_id" << doc_id          // Match the document ID
            << bsoncxx::builder::stream::finalize);
    // Return whether the query found a matching document.
    return result.has_value();
}


/* Helper function to check if a document metadata entry exists */
// Checks if a metadata entry for a specific document ID exists in the document metadata collection.
// This function is crucial for validating whether document metadata is correctly removed
// during the execution of `remove_document`.
//
// Parameters:
// - `doc_id`: The unique document ID to search for in the document metadata collection.
//
// Returns:
// - `true` if the document metadata entry exists, `false` otherwise.
//
// Example Usage:
//   bool exists = DocumentMetadataExists("doc123");
//   ASSERT_TRUE(exists) << "Metadata for document ID 'doc123' should exist in the collection.";
bool DocumentMetadataExists(const std::string& doc_id) {
    // Access the "document_metadata" collection in the test database.
    auto metadata_collection = test_db["document_metadata"];
    // Build a query to search for the document ID in the metadata collection.
    auto result = metadata_collection.find_one(
        bsoncxx::builder::stream::document{}
            << "doc_id" << doc_id                    // Match the document ID.
            << bsoncxx::builder::stream::finalize);
    // Return whether the query found a matching document metadata entry.
    return result.has_value();
}


/* Unit Test Cases */

// Test Suite for remove_document
class RemoveDocumentTest : public ::testing::Test {
protected:
    // The `SetUp` function is called before each test case in the suite is run.
    // It ensures that the database is in a consistent state with pre-defined test data.
    void SetUp() override {
        // Clear the database before each test.
        ClearTestDatabase();

        // start the database with a sample document for testing.
        // This simulates the presence of pre-existing data in the database,

        // Access the "inverted_index" and "document_metadata" collections in the test database.
        auto inverted_index = test_db["inverted_index"];
        auto document_metadata = test_db["document_metadata"];

        // Insert a sample entry into the inverted index collection.
        // This entry associates the term "term1" with a single document ID "doc1".
        // The postings array represents the list of documents containing the term.
        inverted_index.insert_one(bsoncxx::builder::stream::document{}
            << "term" << "term1"
            << "postings" << bsoncxx::builder::stream::open_array
            << bsoncxx::builder::stream::document{}
            << "doc_id" << "doc1" // Document ID associated with the term.
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::close_array
            << bsoncxx::builder::stream::finalize);

        // Insert a sample entry into the document metadata collection.
        // This entry contains metadata for the document with ID "doc1".
        // Metadata could include additional information about the document,
        // such as its title, author, or any custom data required by the application.
        document_metadata.insert_one(bsoncxx::builder::stream::document{}
            << "doc_id" << "doc1"            // Unique document ID.
            << "metadata" << "sample metadata" // Example metadata associated with the document.
            << bsoncxx::builder::stream::finalize);
    }

    // The `TearDown` function is called after each test case in the suite is run.
    // It cleans up the database to ensure no leftover data remains.
    void TearDown() override {
        ClearTestDatabase();
    }
};

// Test Case 1: Successfully removing a document
TEST_F(RemoveDocumentTest, RemoveExistingDocument) {
    /*
      Test Description:
      - This test verifies that an existing document's references are successfully
        removed from both the inverted index and the document metadata database.
    */
    std::string doc_id = "doc1";

    bool result = remove_document(doc_id);

    ASSERT_TRUE(result) << "Failed to remove the existing document.";
    ASSERT_FALSE(DocumentMetadataExists(doc_id)) 
        << "Document metadata entry was not deleted for doc_id: " << doc_id;
    ASSERT_FALSE(DocumentExistsInIndex("term1", doc_id))
        << "Inverted index entry was not deleted for term 'term1' and doc_id: " << doc_id;
}

// Test Case 2: Removing a document with multiple terms in the inverted index
TEST_F(RemoveDocumentTest, RemoveDocumentWithMultipleTerms) {
    /*
      Test Description:
      - This test verifies that a document with multiple terms in the inverted
        index is fully removed, ensuring that all associated terms are deleted
        from the inverted index.
    */
    std::string doc_id = "doc1";

    // Add multiple terms associated with the same document to the inverted index
    auto inverted_index = test_db["inverted_index"];
    inverted_index.insert_one(bsoncxx::builder::stream::document{}
        << "term" << "term2"
        << "postings" << bsoncxx::builder::stream::open_array
        << bsoncxx::builder::stream::document{}
        << "doc_id" << doc_id
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::close_array
        << bsoncxx::builder::stream::finalize);

    inverted_index.insert_one(bsoncxx::builder::stream::document{}
        << "term" << "term3"
        << "postings" << bsoncxx::builder::stream::open_array
        << bsoncxx::builder::stream::document{}
        << "doc_id" << doc_id
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::close_array
        << bsoncxx::builder::stream::finalize);

    bool result = remove_document(doc_id);

    ASSERT_TRUE(result) << "Failed to remove the document with multiple terms.";
    ASSERT_FALSE(DocumentMetadataExists(doc_id)) 
        << "Document metadata entry was not deleted for doc_id: " << doc_id;
    ASSERT_FALSE(DocumentExistsInIndex("term1", doc_id)) 
        << "Inverted index entry was not deleted for term 'term1' and doc_id: " << doc_id;
    ASSERT_FALSE(DocumentExistsInIndex("term2", doc_id)) 
        << "Inverted index entry was not deleted for term 'term2' and doc_id: " << doc_id;
    ASSERT_FALSE(DocumentExistsInIndex("term3", doc_id)) 
        << "Inverted index entry was not deleted for term 'term3' and doc_id: " << doc_id;
}

// Test Case 3: Removing a document with a large metadata entry
TEST_F(RemoveDocumentTest, RemoveDocumentWithLargeMetadata) {
    /*
      Test Description:
      - This test verifies that a document with a large metadata entry is successfully
        removed from both the inverted index and the metadata database.
    */
    std::string doc_id = "doc1";

    // Insert a large metadata entry for the document
    auto document_metadata = test_db["document_metadata"];
    std::string large_metadata(10000, 'x'); // Simulate a large metadata string
    document_metadata.insert_one(bsoncxx::builder::stream::document{}
        << "doc_id" << doc_id
        << "metadata" << large_metadata
        << bsoncxx::builder::stream::finalize);

    bool result = remove_document(doc_id);

    ASSERT_TRUE(result) << "Failed to remove the document with a large metadata entry.";
    ASSERT_FALSE(DocumentMetadataExists(doc_id)) 
        << "Document metadata entry was not deleted for doc_id: " << doc_id;
    ASSERT_FALSE(DocumentExistsInIndex("term1", doc_id)) 
        << "Inverted index entry was not deleted for term 'term1' and doc_id: " << doc_id;
}

// Test Case 4: Database error simulation
TEST_F(RemoveDocumentTest, DatabaseErrorSimulation) {
    /*
      Test Description:
      - This test simulates a database error (e.g., by using an invalid database)
        to ensure that the function handles errors gracefully.
    */
    // Use an invalid database to simulate a connection failure
    mongocxx::database invalid_db = client["invalid_database"];

    // Simulate the database error by pointing to an invalid database (this part assumes 
    // remove_document allows passing a database instance, modify as necessary).
    bool result = false;
    try {
        result = remove_document("doc1");
    } catch (const std::exception& e) {
        // Catch any exception caused by database errors
        result = false;
    }

    ASSERT_FALSE(result) << "Function should return false if a database error occurs.";
}

// Test Case 5: Partial removal failure (manual simulation)
TEST_F(RemoveDocumentTest, PartialRemovalFailure) {
    /*
      Test Description:
      - This test simulates a failure in the inverted index deletion step to ensure
        that the function handles partial failures properly.
    */
    // Simulate a failure by removing the inverted index entry manually
    test_db["inverted_index"].delete_many({}); // Simulate that index removal already failed

    std::string doc_id = "doc1";

    bool result = remove_document(doc_id);

    ASSERT_FALSE(result) << "Function should return false if a partial removal failure occurs.";
    ASSERT_TRUE(DocumentMetadataExists(doc_id)) 
        << "Document metadata entry should still exist due to partial failure.";
}


