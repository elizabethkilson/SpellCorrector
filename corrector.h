#ifndef _CORRECTOR_H
#define _CORRECTOR_H

//#include "hash.h"
#include <unordered_map>
//#include <unordered_set>
#include "entry.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <list>

#define MAX_ERROR_LEN 4

namespace SpellCorrector
{

class corrector{
	private:
		std::vector<std::list<dictEntry> *> * dictionary;
		std::unordered_map<std::string, int> * dictionaryMap;
		std::unordered_map<std::string, error_item> * errors;
		int nErrors;
		int nTotal;
		int nWords;
		void do_fillPossibleWordList(std::list<entry> * wordList, 
		    std::string word, double maxErr, int forcedMinFreq, 
		    int * allowedMinFreq, std::list<dictEntry> * dictList, 
		    double * bestFound, double wordPenalty);
	    double findMinErrorVal(std::string tStr, std::string cStr, 
	        double maxErr);
        double checkDelIns(std::string str, double maxErr, std::string type);
        double checkSub(std::string tStr, std::string cStr, double maxErr);
        void tryAllIns(entry e, std::list<entry> * wordList, 
            std::list<entry> * poss, double forcedErr, double * maxErr, 
            bool add);
        void tryAllDel(entry e, std::list<entry> * wordList, 
            std::list<entry> * poss, double forcedErr, double * maxErr, 
            bool add);
        void tryAllSub(entry e, std::list<entry> * wordList, 
            std::list<entry> * poss, double forcedErr, double * maxErr, 
            bool add);

	public:
	    corrector();
		void setDictionary(std::vector<std::list<dictEntry> *> * dict)
		    {dictionary = dict;};
		void loadDictionary(std::string filename);
		//void writeDictionary(std::string filename);
		void setErrors(std::unordered_map<std::string, error_item> * err)
		    {errors = err;};
		void loadErrors(std::string filename);
		void writeErrors(std::string filename);
		void addWord(std::string word, int score);
		void addError(std::string tString, std::string cString);
		void learn(std::string tFilename, std::string cFilename);
		std::vector<std::list<dictEntry> *> * getDictionary() 
		    {return dictionary;};
	    std::unordered_map<std::string, int> * getDictionaryMap()
	        {return dictionaryMap;};
		std::unordered_map<std::string, error_item> * getErrors()
		    {return errors;};
		int getNErrors() {return nErrors;};
		int getNTotal() {return nTotal;};
		int getNWords() {return nWords;};
		int getWordFreq(std::string word) 
		{
		    if (dictionaryMap->count(word))
		        return dictionaryMap->at(word);
	        return 0;
		};
		void fillPossibleWordListLin(std::list<entry> * wordList, std::string word, 
            double maxErr, int forcedMinFreq = 500, int allowedMinFreq = 1, 
            int maxLenDev = 2);
        void fillPossibleWordListLin( std::list<entry> * wordList, 
            std::list<dictEntry> * choices, double wordPenalty, 
            std::string word, double maxErr, 
            int forcedMinFreq = 500, int allowedMinFreq = 1);
        void fillPossibleWordListExp( std::list<entry> * wordList, 
            std::string word, double maxErr, double forcedErr = -10);
	
	    ~corrector()
	    {
	        delete dictionary;
	        delete errors;
	    };
};

std::string LCS(std::string X, std::string Y);
std::string format_white_space(std::string orig);
std::string strip_punc(std::string orig);
int word_count(std::string orig);
bool eoph (std::string const &fullString, std::string const &endings);

}

#endif //_CORRECTOR_H
