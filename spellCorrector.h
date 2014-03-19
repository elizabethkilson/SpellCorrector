#ifndef _SPELLCORRECTOR_H
#define _SPELLCORRECTOR_H
#include "corrector.h"
#include <string>
#include <sqlite3.h>

#define AVERAGE_WORD_LEN 6

#define cmd_begin "0BEGIN.0"
#define cmd_wait "__wait__"
#define cmd_close  "__close__"
#define cmd_eoph "__eoph__"


namespace SpellCorrector{
std::string bestBigram(std::string first, std::string second, corrector * corr, 
    sqlite3 * db);

std::string Viterbi2( std::string text, corrector * corr, sqlite3 * db, 
    double made_up_word_penalty, int acceptable_freq, int storage_num);

std::string correct(std::string input, double confidence, corrector * corr,
    std::string first, sqlite3 * db);
}

#endif //_SPELLCORRECTOR_H
