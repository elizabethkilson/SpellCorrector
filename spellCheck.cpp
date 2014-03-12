#include <stdio.h>
#include <sqlite3.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include <locale>
#include <math.h> //log()
#include <sstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include "corrector.h"

#define AVERAGE_WORD_LEN 6

#define MAX_BUF 4096
#define cmd_begin "0BEGIN.0"
#define cmd_wait "__wait__"
#define cmd_close  "__close__"
#define cmd_eoph "__eoph__"

int fillBigramList( std::list<dictEntry> ** bigramList, std::string first,
    sqlite3 *db)
{
    std::string statement;
    int rc;
    sqlite3_stmt * ppStmt;
    
    statement = "SELECT * FROM Bigrams\nWHERE First='" + first + "'";
    rc = sqlite3_prepare_v2( db, statement.c_str(), statement.length() + 1, 
        &ppStmt, NULL);
    
    if(sqlite3_step(ppStmt) != SQLITE_ROW)
    {
        (*bigramList) = NULL;
        return 0;
    }
    
    (*bigramList) = new std::list<dictEntry>();
    
    int count = 0;
    
    do
    {
        int score = sqlite3_column_int(ppStmt, 2);
        count += score;
        const unsigned char * sec = sqlite3_column_text(ppStmt, 1);
        std::string second = std::string((char *)sec);
        dictEntry de = dictEntry(second, score);
        (*bigramList)->push_back(de);
    } while (sqlite3_step(ppStmt) == SQLITE_ROW);
    
    (*bigramList)->sort();
    (*bigramList)->reverse();
    return count;
}

std::string bestBigram(std::string first, std::string second, corrector * corr, 
    sqlite3 * db)
{
    std::list<dictEntry> * bigramList;
    int count = fillBigramList( &bigramList, first, db );
    if ((bigramList == NULL) || (count == 0))
    {
        return second;
    }
    
    std::list<entry> wordList = std::list<entry>();
    
    corr->fillPossibleWordListLin(&wordList, bigramList, log(count), 
        second, -20);
    
    if (wordList.empty())
        return second;
    entry e = wordList.front();
    
    delete bigramList;
    return e.str;
}

int validWordCount( std::string word, corrector * corr )
{
    std::string stripped = strip_punc(word);
    std::stringstream sLine (stripped);
    int count = 0;
    std::locale loc;
    while (sLine>>stripped)
    {
        if (corr->getWordFreq(word) || 
            corr->getWordFreq(std::tolower(stripped, loc)))
            count++;
    }
    return count;
}

/*double scoreBigrams( std::string phrase, std::string first, corrector * corr,
std::unordered_map<std::string, void *> * bigramSets, sqlite3 * db )
{
    std::string word;
    std::unordered_map<std::string, int> * bigramSet;
    std::stringstream sLine(phrase);
    double bgPenalty = -20; //TODO: find appropriate
    double penalty = 0;
    while (sLine>>word)
    {
        if (!bigramSets->count(first))
        {
            fillBigramSet( &bigramSet, first, db);
            bigramSets->insert({first, bigramSet});
        }
        bigramSet = (std::unordered_map<std::string, int> *)
        bigramSets->at(first);
        if (bigramSet != NULL)
        {
            if (bigramSet->count(word))
            {
                penalty += log(bigramSet->at(word) + 1);
            }
        }
        penalty += bgPenalty;
        first = word;
    }
    return penalty;
}*/

std::string Viterbi( std::string text, corrector * corr, 
    double made_up_word_penalty, int acceptable_freq)
{
    int n = text.length();
    std::vector<std::string> words = std::vector<std::string>(n + 1, "");
    words[0] = text;
    std::vector<double> best = std::vector<double>(n + 1, -3000.0);
    std::vector<int> lens = std::vector<int>(n + 1, n);
    best[0] = 0.0;
    for (int i = 0; i < n + 1; i++)
    {
        for (int j = 0; j < i; j++)
        {
            std::string word = text.substr(j, i-j);
            int w = word.length();
            double p = -1*made_up_word_penalty;
            if (w > AVERAGE_WORD_LEN)
            {
                p = p*w/AVERAGE_WORD_LEN;
            }
            int count;
            if (count = corr->getWordFreq(word))
            {
                p = log(count) - log(corr->getNWords());
            }
            if (count < acceptable_freq)
            {
                std::list<entry> * wordList = new std::list<entry>();
                corr->fillPossibleWordListLin(wordList, word, -20, 1000, 
                    acceptable_freq, 1);
                if ((wordList != NULL)&&(!wordList->empty()))
                {
                    entry e = wordList->front();
                    word = e.str;
                    p = e.d;
                }
                delete wordList;
            }
            if (p + best[i - w] >= best[i])
            {
                //std::cout<<"i "<<i<<" j "<<j<<std::endl;
                //std::cout<<word<<std::endl;
                //std::cout<<"score "<<p + best[i - w]<<" prev "<<best[i - w]<<" best "<<best[i]<<" w "<<w<<std::endl;
                best[i] = p + best[i - w];
                words[i] = word;
                lens[i] = w;
            }
        }
    }
    
    std::string sequence = words[n];
    //std::cout<<"i "<<n<<" word "<<words[n]<<std::endl;
    int i = n - lens[n];
    //std::cout<<lens[i]<<std::endl;
    while (i > 0)
    {
        //std::cout<<"i "<<i<<" word "<<words[i]<<std::endl;
        sequence = words[i] + " " + sequence;
        //std::cout<<sequence<<std::endl;
        //std::cout<<lens.size()<<std::endl;
        //std::cout<<lens[i]<<std::endl;
        i = i - lens[i];
    }
    return sequence;
}

std::string correct(std::string input, double confidence, corrector * corr,
    std::string first, sqlite3 * db)
{
    std::cout<<"a "<<confidence<<std::endl;
    //int correctCase;
    std::locale loc;
    std::cout<<"b "<<corr->getWordFreq(input)<<std::endl;
    if (corr->getWordFreq(input))
    {
        std::cout<<"word found 1"<<std::endl;
        if (confidence < 90) //TODO: find good threshold
        {
            if (first == cmd_begin)
            {
                std::list<entry> wordList = std::list<entry>();
                corr->fillPossibleWordListLin(&wordList, input, -20);
                if (!wordList.empty())
                {
                    entry e = wordList.front();
                    input = e.str;
                }
            }
            else
            {
                input = bestBigram(first, input, corr, db);
            }
        }
        std::cout<<"word found 2"<<std::endl;
        return input;
    }
    std::cout<<"c"<<std::endl;
    std::list<entry> wordList = std::list<entry>();
    corr->fillPossibleWordListLin(&wordList, input, -20);
    std::cout<<"d"<<std::endl;
    if (!wordList.empty())
    {
        entry e = wordList.front();
        if (e.d > -50) //TODO: find good threshold
        {
            std::cout<<"score "<<e.d<<std::endl;
            return e.str;
        }
    }
    std::cout<<"e"<<std::endl;
    double made_up_word_penalty;
    if (isupper(input[0]))
    {
        made_up_word_penalty = (100 - confidence)*1.5;
    }
    else
    {
        made_up_word_penalty = (100 - confidence)*3;
    }
    std::cout<<"made up word penalty "<<made_up_word_penalty<<std::endl;
    int acceptable_freq = 300;
    std::cout<<"f"<<std::endl;
    input = Viterbi(input, corr, made_up_word_penalty, acceptable_freq);
    std::cout<<"g"<<std::endl;
    return input;
}

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
    
    while (std::cin>>input)
    {
        std::cin>>confidence;
        //wordList = new std::list<entry>();
        
        //corr->fillPossibleWordListLin(wordList, input, -20);
        
        //entry e = wordList->front();
        
        output = correct(input, confidence, corr, first, db);
        //score = e.d;
        std::cout<<"returned"<<std::endl;
        
        std::cout<<output<<std::endl;
        
        //delete wordList;
    }
    
    
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
        
        
    }
}


/*

if (word in dictionary)||(lowerword in dictionary)
{
    if (confidence high)
    {
        pass through;
    }
    else
    {
        return bestBigram;
    }
}
else
{
    if (short enough)
    {
        try single word fix
        if (score > threshold)
            return result
    }
    MADE_UP_WORD_PENALTY = f(confidence, capitalization)
    ACCEPTABLE_FREQ = f(pipe size)
    return Viterbi
}

*/



