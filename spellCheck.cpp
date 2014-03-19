#include "spellCorrector.h"
#include <string>
#include <sqlite3.h>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>

#define MAX_BUF 4096

using namespace SpellCorrector;

int main()
{
    std::string input;
    std::string output;
    double score;
    
    corrector * corr = new corrector();
    
    std::string cFile = "../Training/PrideAndPrejudice/PrideAndPrejudice.txt";
    std::string tFile = "../Training/PrideAndPrejudice/tess2.txt";
    
    corr->loadDictionary("../Dictionary/unigrams.txt");
    corr->loadErrors("../trained5.txt");
    //corr->learn(tFile, cFile);
    //std::cout<<"learned"<<std::endl;
    //corr->writeErrors("trained5.txt");
    
    sqlite3 *db;
    int rc;
    rc = sqlite3_open("../Dictionary/BigramDatabase.db", &db);
    if (rc)
    {
        std::cout<<"Can't open database"<<std::endl;
        return 1;
    }
    
    std::string first = "0BEGIN.0";
    double confidence;
    
    std::cin>>input;
    
    while (std::cin>>input)
    {
        confidence = 89;
        //wordList = new std::list<entry>();
        
        //corr->fillPossibleWordListLin(wordList, input, -20);
        
        //entry e = wordList->front();
        
        
        output = correct(input, confidence, corr, first, db);
        std::cout<<"returned"<<std::endl;
        std::cout<<output<<std::endl;
        
        /*output = Viterbi3( input, corr, db, 56000000, 50, 300);
        std::cout<<"returned"<<std::endl;
        std::cout<<output<<std::endl;*/
        
        std::stringstream ss (output);
        while (ss>>first);
        
        //delete wordList;
    }
    
    delete corr;
    return 0;
}

int main2()
{    
    std::string first = cmd_begin;
    std::string input;
    double confidence;
    
    corrector * corr = new corrector();
    corr->loadDictionary("Dictionary/unigrams.txt");
    corr->loadErrors("trained5.txt");
    
    sqlite3 *db;
    int rc;
    rc = sqlite3_open("Dictionary/BigramDatabase.db", &db);
    if (rc)
    {
        std::cout<<"Can't open database"<<std::endl;
        return 1;
    }
    
    int outfd;
    std::string fifoName = "/tmp/sc2esFIFO";
    int status = mkfifo(fifoName.c_str(), 420);
    outfd = open(fifoName.c_str(), O_WRONLY);
    
    int infd;
    std::string inFifoName = "/tmp/ts2scFIFO";
    char buf[MAX_BUF];
    
    std::ifstream in( inFifoName.c_str(), std::ios_base::in);
    int written;
    std::locale loc;
    while(in>>input)
    {
        if (input == cmd_eoph)
        {
            do {
                written = write(outfd, "\n", sizeof(char));
            } while (written == -1);
            continue;
        }
        if (input == cmd_wait)
        {
            continue;
        }
        if (input == cmd_close)
        {
            break;
        }
        
        do {
            written = write(outfd, " ", sizeof(char));
        } while (written == -1);
        
        in>>confidence;
        
        input = correct(input, confidence, corr, first, db);
        
        
    }
}

