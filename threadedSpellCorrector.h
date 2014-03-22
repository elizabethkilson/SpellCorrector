#ifndef _SPELLCORRECTOR_H
#define _SPELLCORRECTOR_H
#include "corrector.h"
#include "../libdistributed/ThreadPool.hpp"
#include <string>
#include <sqlite3.h>
#include <functional>
#include <mutex>
#include <condition_variable>

#define AVERAGE_WORD_LEN 6

#define cmd_begin "0BEGIN.0"
#define cmd_wait "__wait__"
#define cmd_close  "__close__"
#define cmd_eoph "__eoph__"

using namespace Distributed;

namespace SpellCorrector
{

std::string bestBigram(std::string first, std::string second, corrector * corr, 
    sqlite3 * db);

class ViterbiWSA //Viterbi Word Splitting Algorithm
{
private:
    std::vector<std::vector<std::tuple<double, std::string, int>>> best;
    void unwrap_string(std::list<std::string> * output, 
        std::vector<int> * indices);
    std::string create_sequence(std::list<std::string> * words);
    
public:
    ViterbiWSA() {};
    /*std::string Viterbi( std::string text, corrector * corr, sqlite3 * db, 
        double made_up_word_penalty, int acceptable_freq, int storage_num, 
        std::function<void (*)(corrector *, std::string, double, double, 
        std::function<void (SpellCorrector::ViterbiWSA::*)
        (int, double, std::string, int)>, void * context)> 
        correcting_search);*/
    std::string Viterbi( std::string text, corrector * corr, sqlite3 * db, 
        double made_up_word_penalty, int acceptable_freq, int storage_num, 
        std::function<void (corrector *, std::string, int, double, double, 
        std::mutex *, int *, std::mutex *, std::condition_variable *, 
        ViterbiWSA*)> correcting_search, ThreadPool * tpool);
    double update_vectors(int i, double p, std::string word, int w);
};

std::string correct(std::string input, double confidence, corrector * corr,
    std::string first, sqlite3 * db);
}

#endif //_SPELLCORRECTOR_H
