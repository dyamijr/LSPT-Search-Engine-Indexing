// database.h

#ifndef DATABASE_H // Include guard
#define DATABASE_H

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <iostream>
using namespace std;

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
bool addIndex(string dbConnectionString, string index, string docId, int frequency, int positions);

/* 
*   Function Name:
*       removeDoc
*   Description: 
*       Attempts to remove all occurence of a document id in the database
*   Parameters:
*       dbClient - auto - a database client object corresponding to the connected database
*       docId - the document we want to remove
*   Returns:
*       None
*   Side Effects:
*       the indexTable in the database will be updated no longer containing the document information with DocId
*   Example Usages:
*       addIndex(IndexingClient, "cat", "1234", 5, {1,6,12,25,80})
*/
void removeDoc(auto dbClient, string docId);

#endif // DATABASE_H

