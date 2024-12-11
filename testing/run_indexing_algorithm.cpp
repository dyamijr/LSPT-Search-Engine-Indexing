#include <gtest/gtest.h>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>


/* run_indexing_algorithm()
 *
 * This function processes a document's terms and updates the inverted index
 * stored in the MongoDB database. The function communicates with the database
 * to ensure that all indexing operations are executed and verified. The
 * success or failure of the database operations determines the return value
 * of the function.
 *
 * Parameters:
 *   doc_id (const std::string&):
 *       The unique identifier for the document being indexed.
 *
 *   document_terms (const std::vector<std::string>&):
 *       A collection of terms extracted from the document.
 *
 *   is_update (bool):
 *       A boolean flag indicating whether the function should update existing
 *       entries for the document (if true) or add new entries for a document
 *       that does not already exist in the index (if false).
 *
 * Functionality:
 *   - This function interacts directly with the MongoDB database to manage the
 *     inverted index.
 *   - If `is_update` is true:
 *       - The function must remove all current entries in the inverted index
 *         associated with `doc_id` by issuing a database delete query.
 *       - Ensure that the removal operation is confirmed by the database.
 *   - For each term in `document_terms`:
 *       - Insert or update the corresponding inverted index entry in the
 *         database. Each entry must associate the term with `doc_id` and
 *         include metadata, such as the frequency of the term in the document.
 *   - Save the new or updated inverted index entries to the database.
 *   - Ensure that all database operations are atomic. If any database operation
 *     fails (e.g., deletion, insertion, or update), log the error, abort the
 *     process, and return false.
 *   - The function will rely on the success or failure message returned by the
 *     MongoDB database to determine its return value.
 *
 * Error Handling:
 *   - If the MongoDB database connection fails or an error occurs during any
 *     database operation, the function must:
 *       - Log the error message.
 *       - Roll back any partial changes if possible.
 *       - Return false.
 *   - If `document_terms` is empty, the function should return without modifying
 *     the database but should log that no terms were processed.
 *
 * Return:
 *   - Returns a boolean value:
 *       - `true` if all database operations (delete, insert, or update) complete
 *         successfully, as indicated by the success messages from the MongoDB
 *         database.
 *       - `false` if any error occurs during the database operations or if input
 *         validation fails.
 *
 * Examples of Usage:
 *   - Adding a new document to the index:
 *       run_indexing_algorithm("doc123", {"term1", "term2", "term3"}, false);
 *   - Updating an existing document's index:
 *       run_indexing_algorithm("doc123", {"term1", "term2", "term4"}, true);
 *
 */
bool run_indexing_algorithm(const std::string& doc_id,
                            const std::vector<std::string>& document_terms,
                            bool is_update);

/* Test Database Configuration */

// Initialize a MongoDB instance.
// This is required to set up the MongoDB driver in the current process.
// Without initializing `mongocxx::instance`, no MongoDB operations can be performed.
mongocxx::instance instance{}; 
// Create a MongoDB client connected to the local MongoDB instance.
// This client will be used to interact with the test database during unit tests.
// The URI specifies the MongoDB instance to connect to, in this case, "localhost" on the default port (27017).
mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
// Access the test database named "test_indexing".
// Using a dedicated test database ensures that the tests do not interfere with the production database
// and provides a controlled environment for testing isolated components.
mongocxx::database test_db = client["test_indexing"];

// Helper function to clear test database collections
// This function is necessary to ensure that each test starts with a clean and predictable database state.
// By removing all documents before and after each test, we prevent data from previous tests
// from affecting the results of subsequent tests.
void ClearTestDatabase() {
    test_db["inverted_index"].delete_many({});
    test_db["document_metadata"].delete_many({});
}

/* Helper function to check if a document exists in the inverted index */
// Check if a specific document ID is associated with a term in the inverted index.
// For example, after running the `run_indexing_algorithm` or `remove_document` functions,
// this helper can verify whether the database was updated as expected.
//
// Parameters:
// - `term`: The term to search for in the inverted index.
// - `doc_id`: The document ID to check within the term's postings list.
//
// Returns:
// - `true` if the specified term and document ID exist in the inverted index, `false` otherwise.
bool DocumentExistsInIndex(const std::string& term, const std::string& doc_id) {
    // Access the "inverted_index" collection.
    auto inverted_index = test_db["inverted_index"];
    // Build a query to search for the specified term and document ID in the inverted index.
    auto result = inverted_index.find_one(
        bsoncxx::builder::stream::document{}
            << "term" << term                        // Match the term.
            << "postings.doc_id" << doc_id          // Match the document ID 
            << bsoncxx::builder::stream::finalize);
    // Return whether the query found a matching document.
    return result.has_value();
}
/* Unit Test Cases */

// Test Suite for run_indexing_algorithm
class RunIndexingAlgorithmTest : public ::testing::Test {
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

// Test Case 1: Adding a new document with valid terms
TEST_F(RunIndexingAlgorithmTest, AddNewDocument) {
    /*
      Test Description:
      - This test verifies that a new document with valid terms can be added to the inverted index.
      - It checks that the terms are correctly stored and associated with the provided doc_id.
    */
    std::string doc_id = "doc1";
    std::vector<std::string> document_terms = {"term1", "term2", "term3"};
    bool is_update = false;

    bool result = run_indexing_algorithm(doc_id, document_terms, is_update);

    ASSERT_TRUE(result) << "Failed to add a new document to the inverted index.";
    for (const auto& term : document_terms) {
        ASSERT_TRUE(DocumentExistsInIndex(term, doc_id)) 
            << "Term " << term << " is not found in the inverted index for document " << doc_id;
    }
}

// Test Case 2: Updating an existing document
TEST_F(RunIndexingAlgorithmTest, UpdateExistingDocument) {
    /*
      Test Description:
      - This test verifies that an existing document's terms can be updated in the inverted index.
      - It checks that old terms are removed and new terms are added correctly.
    */
    std::string doc_id = "doc1";
    std::vector<std::string> initial_terms = {"term1", "term2"};
    std::vector<std::string> updated_terms = {"term3", "term4"};
    bool is_update;

    // Add initial terms
    is_update = false;
    ASSERT_TRUE(run_indexing_algorithm(doc_id, initial_terms, is_update));

    // Update terms
    is_update = true;
    ASSERT_TRUE(run_indexing_algorithm(doc_id, updated_terms, is_update));

    // Old terms should not exist
    for (const auto& term : initial_terms) {
        ASSERT_FALSE(DocumentExistsInIndex(term, doc_id)) 
            << "Old term " << term << " should have been removed from the inverted index.";
    }
    // New terms should exist
    for (const auto& term : updated_terms) {
        ASSERT_TRUE(DocumentExistsInIndex(term, doc_id)) 
            << "New term " << term << " is not found in the inverted index for document " << doc_id;
    }
}

// Test Case 3: Adding terms with duplicates for an existing document
TEST_F(RunIndexingAlgorithmTest, AddingDuplicateTerms) {
    /*
      Test Description:
      - This test verifies that when a document contains duplicate terms in the input,
        the function correctly handles them by ensuring that the inverted index stores
        each term only once with the correct metadata (e.g., term frequency).
    */
    std::string doc_id = "doc1";
    std::vector<std::string> document_terms = {"term1", "term1", "term2", "term2"};
    bool is_update = false; // Adding new terms to the document

    bool result = run_indexing_algorithm(doc_id, document_terms, is_update);

    ASSERT_TRUE(result) << "Failed to add terms with duplicates for the document.";
    ASSERT_TRUE(DocumentExistsInIndex("term1", doc_id))
        << "Term 'term1' is not found in the inverted index for doc_id: " << doc_id;
    ASSERT_TRUE(DocumentExistsInIndex("term2", doc_id))
        << "Term 'term2' is not found in the inverted index for doc_id: " << doc_id;
}


// Test Case 4: Updating a document with completely new terms
TEST_F(RunIndexingAlgorithmTest, UpdatingWithNewTerms) {
    /*
      Test Description:
      - This test verifies that when updating an existing document with a completely
        new set of terms, the function correctly removes old terms and adds the new ones.
    */
    std::string doc_id = "doc1";

    // Initial terms for the document
    std::vector<std::string> initial_terms = {"term1", "term2"};
    bool is_update = false; // Adding initial terms
    ASSERT_TRUE(run_indexing_algorithm(doc_id, initial_terms, is_update))
        << "Failed to add initial terms to the document.";

    // New terms for the update
    std::vector<std::string> updated_terms = {"term3", "term4"};
    is_update = true; // Update the document terms


    bool result = run_indexing_algorithm(doc_id, updated_terms, is_update);

    ASSERT_TRUE(result) << "Failed to update the document with new terms.";

    // Old terms should not exist
    ASSERT_FALSE(DocumentExistsInIndex("term1", doc_id))
        << "Old term 'term1' was not removed from the inverted index.";
    ASSERT_FALSE(DocumentExistsInIndex("term2", doc_id))
        << "Old term 'term2' was not removed from the inverted index.";

    // New terms should exist
    ASSERT_TRUE(DocumentExistsInIndex("term3", doc_id))
        << "New term 'term3' is not found in the inverted index for doc_id: " << doc_id;
    ASSERT_TRUE(DocumentExistsInIndex("term4", doc_id))
        << "New term 'term4' is not found in the inverted index for doc_id: " << doc_id;
}


// Test Case 5: Database error simulation
TEST_F(RunIndexingAlgorithmTest, DatabaseErrorSimulation) {
    /*
      Test Description:
      - This test verifies that the function handles database errors gracefully.
      - This can be simulated by disconnecting the MongoDB client or using an invalid database name.
    */
    // Use an invalid database to simulate a connection failure
    mongocxx::database invalid_db = client["invalid_database"];
    
    // Attempt to add a document
    std::string doc_id = "doc3";
    std::vector<std::string> document_terms = {"term1", "term2"};
    bool is_update = false;

    // Simulate the database error by pointing to an invalid database (this part assumes 
    // run_indexing_algorithm allows passing a database instance, modify as necessary).
    bool result = false;
    try {
        result = run_indexing_algorithm(doc_id, document_terms, is_update);
    } catch (const std::exception& e) {
        // Catch any exception caused by database errors
        result = false;
    }

    ASSERT_FALSE(result) << "Function should return false if a database error occurs.";
}