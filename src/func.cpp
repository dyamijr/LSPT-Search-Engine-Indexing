#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "database.h"
#include <mongocxx/client.hpp>
using namespace std;
#include "nlohmann/json.hpp"

// map<string, set<string>> invertedIndex;

using json = nlohmann::json;
// main func
std::string addToIndexsd (std::string doc_ID){
    string connect = "mongodb+srv://dyamiwatsonjr:LSPTTeamx@lspt.xq5ap.mongodb.net/?retryWrites=true&w=majority&appName=LSPT";
    // json object token
    //auto tokens = find (doc_dbClient, doc_id);


    // Validate input
    if (doc_ID == ""){
        return "invalid input";
    }

    /*query database for id*/
    /*determine if update entry or new entry*/
    
    //ToDO: read in data from dds database
    //TODO: handle metadata

    
    std::ifstream file("../src/test.json");
    json tokenList;
    file >> tokenList;

    for (auto token : tokenList["tokens"]){
        string index = token["token"];
        int frequency = token["frequency"];
        int position = token["position"];
        std::cout << index << std::endl;
        std::cout << frequency << std::endl;
        std::cout << position <<"\n"  << std::endl;
        addIndex(connect, index, doc_ID, frequency, position);
    }
    return "";
}

/*
// main func
int getAverageDocLength(){
    //return average as integer
    //int avg_len = calc_avg_length ();
    int avg_len = 0; 
    return avg_len;
}

/*std::vector <int> getDocLengths (void){
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


    return 0;
}*/