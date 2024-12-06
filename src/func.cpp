#include <iostream>
#include <string>
#include <vector>
using namespace std;
#include "nlohmann/json.hpp"

// map<string, set<string>> invertedIndex;


// main func
std::string addToIndex (std::string doc_ID){

    auto doc_dbClient = connectToDatabase("329382098");
    auto index_dbClient = connectToDatabase("232323");

    // json object token
    auto tokens = find (doc_dbClient, doc_id);


    // Validate input
    if (doc_ID == ""){
        return "invalid input";
    }

    /*query database for id*/
    /*determine if update entry or new entry*/


    using json = nlohmann::json;
    std::ifstream file("test.json");
    json j;
    file >> j;

    std::cout << j["name"] << std::endl;
    std::cout << j["age"] << std::endl;
    std::cout << j["tokens"][0]["frequency"] << std::endl;


    string index;
    int frequency = j["tokens"][0]["frequency"];
    std::vector <int> positions = 


    addIndex(index_dbClient, string index, string docId, int frequency, vector<int> positions);
    //INDEXING ALGO

    // update
        // remove old index entry


    // create inverted index array  [cat] --> ID1, ID2, ID3
    

    // save entry to database

}


// main func
int getAverageDocLength(){
    //return average as integer
    int avg_len = calc_avg_length ();
    avg_len = 0; 
    return avg_len;
}

std::vector <int> getDocLengths (void){
    // query database for all doc lengths


}

int calc_avg_length (){

    std::vector <int> total_lengths = getDocLengths ();
    
    int sum_lengths = 0;
    for (int i = 0; i < total_lengths.size(); i++){
        sum_lengths += total_lengths[i];
    }

    int num_docs = total_lengths.size();
    int average_len = sum_lengths / num_docs;


    return 
}