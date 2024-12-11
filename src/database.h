// database.h
#ifndef DATABASE_H // Include guard
#define DATABASE_H

#include <string>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <iostream>
using namespace std;

/* 
*   Function Name:
*       pingIndex
*   Description: 
*       Calls the associated function
*   Parameters:
*       docId - string - the document we need to operate on 
*       operation - string - the operation we will complete, must be "add", "remove", or "update"
*   Returns:
*       true on successful operation else false
*   Side Effects:
*       the database will be modified based on the operation
*   Example Usages:
*       pingIndex("1234", "add");
*/
bool pingIndex(string doc_ID, string operation);
/* 
*   Function Name:
*       addIndex
*   Description: 
*       Attempts to create a link between a document and an index. If a link between the 
*       two already exist, update it with the new docId and positions information. If the 
*       index exist but not associated with the docId yet create a new document object 
*       associated with the index. If this is our first time seeing the index just create 
*       a new insert for the table.
*   Parameters:
*       dbClient - auto - a database client object corresponding to the connected database
*       index - string - the index we want to be adding
*       docId - the document the index appears in 
*       frequency - the number of times the index appears in the document
*       positions - where in the document the index appears
*   Returns:
*       None
*   Side Effects:
*       the indexTable in the database will be updated with the index, docId, frequency and positions
*   Example Usages:
*       addIndex(IndexingClient, "cat", "1234", 5, {1,6,12,25,80})
*/
bool addIndex(string dbConnectionString, bsoncxx::types::b_string index, string docId, string frequency, string positions);

/* 
*   Function Name:
*       addToIndex
*   Description: 
*       Attempts to add all information from the given docid to the index.
*   Parameters:
*       doc_ID - stores the document id we want to add
*   Returns:
*       true on a successful add otherwise false
*   Side Effects:
*       the indexTable in the database will be updated with the information realting to the doc_ID
*   Example Usages:
*       addToIndex("1234") = true
*/
bool addToIndex(string doc_ID);
/* 
*   Function Name:
*       removeFromIndex
*   Description: 
*       Attempts to remove all information from the given docid from the index.
*   Parameters:
*       doc_ID - stores the document id we want to remove
*   Returns:
*       true as long as no errors occured otherwise false
*   Side Effects:
*       the indexTable in the database will be updated with all information realting to the doc_ID removed
*   Example Usages:
*       removeFromIndex("1234") = true
*/
bool removeFromIndex(string doc_ID);
/* 
*   Function Name:
*       updateIndex
*   Description: 
*       Attempts to update information from the given docid to the index.
*   Parameters:
*       doc_ID - stores the document id we want to update
*   Returns:
*       true as long as no errors occured and information was added
*   Side Effects:
*       the indexTable in the database will be updated with the information realting to the doc_ID
*   Example Usages:
*       updateIndex("1234") = true
*/
bool updateIndex(string doc_ID);
#endif // DATABASE_H

