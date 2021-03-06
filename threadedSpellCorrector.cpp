#include "threadedSpellCorrector.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <locale>
#include <math.h> //log()
#include <sstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <time.h>
#include <thread>

using namespace Distributed;

namespace SpellCorrector
{

int fillBigramList( std::list<dictEntry> ** bigramList, std::string first,
    sqlite3 *db)
{
    clock_t t;
    t = clock();
    
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
    } while ((rc = (sqlite3_step(ppStmt)) == SQLITE_ROW));
    
    if ((rc != SQLITE_DONE)&&(rc != SQLITE_OK))
    {
        std::cerr<<"Error reading bigram list "<<rc<<std::endl;
    }
    
    (*bigramList)->sort();
    (*bigramList)->reverse();
    
    t = clock() - t;
    
    //std::cout<<"time to load bigram list for "<<first<<": "<<
        //((float)t)/CLOCKS_PER_SEC<<std::endl;
    
    return count;
}

void fillBigramSet( std::unordered_map<std::string, int> ** bigramSet,
    std::string first, sqlite3 * db)
{
    clock_t t;
    t = clock();
    std::string statement;
    int rc;
    sqlite3_stmt * ppStmt;
    
    statement = "SELECT * FROM Bigrams\nWHERE First='" + first + "'";
    rc = sqlite3_prepare_v2( db, statement.c_str(), statement.length() + 1, 
        &ppStmt, NULL);
    
    if(sqlite3_step(ppStmt) != SQLITE_ROW)
    {
        (*bigramSet) = NULL;
        return;
    }
    
    (*bigramSet) = new std::unordered_map<std::string, int>();
    
    do
    {
        int score = sqlite3_column_int(ppStmt, 2);
        const unsigned char * sec = sqlite3_column_text(ppStmt, 1);
        std::string second = std::string((char *)sec);
        (*bigramSet)->insert({second, score});
    } while (sqlite3_step(ppStmt) == SQLITE_ROW);
    
    //std::cout << "Bigram set size: " << (*bigramSet)->size() <<std::endl;
    
    t = clock() - t;
    
    //std::cout<<"time to load bigram set for "<<first<<": "<<
        //((float)t)/CLOCKS_PER_SEC<<std::endl;
    
    /*std::locale loc;
    std::string lf = std::tolower(first, loc);
    if (lf == first)
        return;
    statement = "SELECT * FROM Bigrams\nWHERE First='" + lf + "'";
    rc = sqlite3_prepare_v2( db, statement.c_str(), statement.length() + 1, 
        &ppStmt, NULL);
    while (sqlite3_step(ppStmt) == SQLITE_ROW)
    {
        int score = sqlite3_column_int(ppStmt, 2);
        const unsigned char * sec = sqlite3_column_text(ppStmt, 1);
        std::string second = std::string((char *)sec);
        (*bigramSet)->insert({second, score});
    }*/
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
    
    double maxErr = -20;
    int len;
    if ((len = second.length()) > (2*AVERAGE_WORD_LEN))
    {
        maxErr = maxErr * len / (2*AVERAGE_WORD_LEN);
    }
    
    corr->fillPossibleWordListLin(&wordList, bigramList, log(count), 
        second, maxErr);
    
    if (wordList.empty())
        return second;
    entry e = wordList.front();
    
    delete bigramList;
    return e.str;
}

double scoreBigram( std::string first, std::string second, corrector * corr,
    std::unordered_map<std::string, std::unordered_map<std::string, int> *> 
    * bigramSets, sqlite3 * db )
{
    std::string word;
    std::unordered_map<std::string, int> * bigramSet;
    double score = 0;
    if (!bigramSets->count(first))
    {
        fillBigramSet( &bigramSet, first, db);
        bigramSets->insert({first, bigramSet});
    }
    if (!bigramSets->count(first))
    {
        return 0;
    }
    bigramSet = bigramSets->at(first);
    if (bigramSet != NULL)
    {
        if (bigramSet->count(second))
        {
            score += bigramSet->at(second);
        }
    }
    return score;
}

double ViterbiWSA::update_vectors(int i, double p, std::string word, int w)
{
    //std::cout<<"update "<<word<<" "<<p<<std::endl;
    if (p < std::get<0>(best[i][best[i].size() - 1]))
        return std::get<0>(best[i][best[i].size() - 1]);
    int k;
    for (k = (best[i]).size() - 1; k > 0; k--)
    {
        if (p < std::get<0>(best[i][k - 1]))
        {
            break;
        }
        best[i][k] = std::move(best[i][k - 1]);
    }
    
    best[i][k] = std::tuple<double, std::string, int>(p, word, w);
    return std::get<0>(best[i][best[i].size() - 1]);
}

void ViterbiWSA::unwrap_string(std::list<std::string> * output, 
    std::vector<int> * indices)
{
    int i = indices->size() - 1;
    int j = best.size() - 1;
    for (; (j > 0)&&(i >= 0); i--)
    {
        output->push_front(std::get<1>(best[j][indices->at(i)]));
        j -= std::get<2>(best[j][indices->at(i)]);
    }
    while (j > 0)
    {
        output->push_front(std::get<1>(best[j][0]));
        j -= std::get<2>(best[j][0]);
    }
}

std::string ViterbiWSA::create_sequence(std::list<std::string> * words)
{
    std::list<std::string>::iterator it = words->begin();
    std::string sequence = (*it);
    ++it;
    for (; it != words->end(); ++it)
    {
        sequence += " " + (*it);
    }
    return sequence;
}

/*std::string ViterbiWSA::Viterbi( std::string text, corrector * corr, 
    sqlite3 * db, double made_up_word_penalty, int acceptable_freq, 
    int storage_num,
    std::function<void (*)(corrector *, std::string, double, double, 
    std::function<void (SpellCorrector::ViterbiWSA::*)
    (int, double, std::string, int)>, void * context)> 
    correcting_search)*/
std::string ViterbiWSA::Viterbi( std::string text, corrector * corr, 
    sqlite3 * db, double made_up_word_penalty, int acceptable_freq, 
    int storage_num,
    std::function<void (corrector *, std::string, int, double, double, 
    std::mutex *, int *, std::mutex *, std::condition_variable *, 
    ViterbiWSA*)> correcting_search, ThreadPool * tpool)
{
    clock_t t;
    t = clock();
    int n = text.length();
    best = std::vector<std::vector<std::tuple<double, std::string, int>>>
        (n + 1, std::vector<std::tuple<double, std::string, int>> (storage_num,
        std::tuple<double, std::string, int>(-10000, "", n)));
    best[0][0] = std::tuple<double, std::string, int>(0.0, text, n);
    //std::cout<<"a "<<std::endl;
    //boost::threadpool::pool tp(4);
    for (int i = 0; i < n + 1; i++)
    {
        std::vector<std::thread> searches;
        std::mutex data_write_lock;
        int threadsRunning = 0;
        std::mutex count_lock;
        std::condition_variable cv;
        for (int j = std::max(0, i - 10); j < i; j++)
        {
            //std::cout<<"1 i "<<i<<" j "<<j<<std::endl;
            bool inserted = false;
            std::string word = text.substr(j, i-j);
            int w = word.length();
            double p = -1*made_up_word_penalty;
            if (isupper(word[0]))
            {
                p = p/2;
            }
            if (w > AVERAGE_WORD_LEN)
            {
                p = p*w/AVERAGE_WORD_LEN;
            }
            //std::cout<<"b "<<std::endl;
            int count;
            //std::cout<<"c "<<std::endl;
            if (count = corr->getWordFreq(word))
            {
                p = log(count) - log(corr->getNWords());
                p += std::get<0>(best[i - w][0]);
                if (p > std::get<0>(best[i][storage_num - 1]))
                {
                    data_write_lock.lock();
                    update_vectors( i, p, word, w);
                    data_write_lock.unlock();
                }
                inserted = true;
            }
            //std::cout<<"d "<<std::endl;
            
            count_lock.lock();
            threadsRunning++;
            count_lock.unlock();
            //std::cout<<"e"<<std::endl;
            tpool->execute([&, w, word, i, corr] () 
                {return correcting_search(corr, word, i, 
                std::get<0>(best[i - w][0]), std::get<0>(best[i][0]),
                &data_write_lock, &threadsRunning, &count_lock, &cv, this);});
            
            /*std::thread th (correcting_search, corr, word, i,
                std::get<0>(best[i - w][0]), std::get<0>(best[i][0]),
                &data_write_lock, this);
            searches.push_back(std::move(th));*/
            
            //std::cout<<"f "<<std::endl;
            if (!inserted && (p > std::get<0>(best[i][storage_num - 1])))
            {
                p += std::get<0>(best[i - w][0]);
                data_write_lock.lock();
                update_vectors( i, p, word, w);
                data_write_lock.unlock();
            }
        }
        
        /*for (std::thread &t: searches) 
        {
            if (t.joinable()) {
                t.join();
            }
        }*/
        //std::cout<<"g"<<std::endl;
        std::unique_lock<std::mutex> ul (count_lock);
        cv.wait(ul, [&]{ return threadsRunning == 0;});
        //std::cout<<"h"<<std::endl;
    }
    //std::cout<<"Finished Viterbi. Starting bigram check."<<std::endl;
    std::list<std::string> results = std::list<std::string>();
    
    std::vector<int> indices = std::vector<int>(n, 0);
    
    unwrap_string( &results, &indices);
    
    /*
    std::unordered_map<std::string, std::unordered_map<std::string, int> *>
        bigramSets;
    
    std::list<std::vector<int>> indices_list = std::list<std::vector<int>>();
    //std::unordered_set<int> checked = std::unordered_set<int>();
    
    int safetyCount = 0;

    indices_list.push_front(std::vector<int>(results.size(), 0));
    //std::cout<<"b "<<std::endl;
    while (!indices_list.empty())
    {
        ++safetyCount;
        std::cout<<"Safety Count: "<<safetyCount<<std::endl;
        if (safetyCount > 10) break;
        //std::cout<<"c "<<std::endl;
        indices = indices_list.front();
        indices_list.pop_front();
        int key = 0;
        /*for (int i = 0; i < indices.size(); i++)
        {
            key = key*storage_num + indices[i];
        }
        if (checked.count(key))
            continue;
        
        results = std::list<std::string>();
        unwrap_string( &results, &indices );
        
        std::cout<<"Results size: "<<results.size()<<std::endl;
        
        if (results.empty())
            continue;
        //std::cout<<"d "<<std::endl;
        std::list<std::string>::iterator it = --(--results.end());
        std::list<std::string>::iterator it2 = --results.end();
        bool bigrams_found = (results.size() > 1);
        std::vector<int> new_indices (indices.size() + 2, 0);
        std::cout<<"Results size: "<<results.size()<<" NI size: "<<new_indices.size()<<std::endl;
        int offset = indices.size() - new_indices.size();
        for (int i = indices.size() - 1; (it2 != results.begin()) && (i >= 0);
            --it, --it2, --i)
        {
            std::cout<<"1 i "<<i<<" "<<new_indices.size()<<std::endl;
            std::cout<<(*it)<<" "<<(*it2)<<std::endl;
            double score = scoreBigram( (*it), (*it2), corr, &bigramSets, db );
            //std::cout << "Bigram score for "<< (*it) << " " << (*it2) <<": "<<score<<std::endl;
            std::cout<<"2 "<<results.size()<<" "<<new_indices.size()<<std::endl;
            bigrams_found = bigrams_found && (score > 0);
            std::cout<<"3 "<<i<<" "<<offset<<" "<<new_indices.size()<<std::endl;
            new_indices[i - offset] = indices[i];
            std::cout<<"4 "<<score<<std::endl;
            if (!score)
            {
                std::cout<< "a" <<std::endl;
                if (new_indices[i - offset] < storage_num - 1)
                {
                    //std::cout<<"new indices size: "<<new_indices.size() << std::endl;
                    new_indices[i - offset]++;
                    //int key = 0;
                    std::cout<< "a2" <<std::endl;
                    /*for (int j = 0; j < new_indices.size(); j++)
                    {
                        key = key*storage_num + new_indices[j];
                        std::cout<<j<<" "<<new_indices.size()<<std::endl;
                    }
                    //std::cout<< "a3" <<std::endl;
                    //if (!checked.count(key))
                    {
                        //std::cout<< "a32" <<std::endl;
                        indices_list.push_back(new_indices);
                        //std::cout<< "a33" <<std::endl;
                        //checked.insert(key);
                        //std::cout<< "a34" <<std::endl;
                    }
                    std::cout<< "a3" <<std::endl;
                    new_indices[i - offset]--;
                }
                std::cout<< "b" <<std::endl;
                if (((i - offset - 1) > 0) && 
                    (new_indices[i - offset - 1] < storage_num - 1))
                {
                    new_indices[i - offset - 1]++;
                    //int key = 0;
                    /*for (int j = 0; j < new_indices.size(); j++)
                    {
                        key = key*storage_num + new_indices[j];
                    }
                    //if (!checked.count(key))
                    {
                        indices_list.push_back(new_indices);
                        //checked.insert(key);
                    }
                    new_indices[i - offset - 1]++;
                }
                std::cout<< "c" <<std::endl;
            }
            std::cout<<"end of if (!score)"<<std::endl;
        }
        int ind = -1*offset;
        while (it2 != results.begin())
        {
            double score = scoreBigram( (*it), (*it2), corr, &bigramSets, db );
            bigrams_found = bigrams_found && (score > 0);
            if (!score)
            {
                if (new_indices[ind] < storage_num - 1)
                {
                    new_indices[ind]++;
                    /*int key = 0;
                    for (int j = 0; j < new_indices.size(); j++)
                    {
                        key = key*storage_num + indices[j];
                    }
                    //if (!checked.count(key))
                    {
                        indices_list.push_back(new_indices);
                        //checked.insert(key);
                    }
                    new_indices[ind]--;
                }
                if (((ind - 1) > 0) && 
                    (new_indices[ind - 1] < storage_num - 1))
                {
                    new_indices[ind - 1]++;
                    /*int key = 0;
                    for (int j = 0; j < new_indices.size(); j++)
                    {
                        key = key*storage_num + indices[j];
                    }
                    //if (!checked.count(key))
                    {
                        indices_list.push_back(new_indices);
                        //checked.insert(key);
                    }
                    new_indices[ind - 1]++;
                }
            }
            --it, --it2, --ind;
        }
        if (bigrams_found)
        {
            //std::cout<<"bigrams found"<<std::endl;
            t = clock() - t;
    
            //std::cout<<"time for Viterbi2: "<<
                //((float)t)/CLOCKS_PER_SEC<<std::endl;
                
            return create_sequence(&results);
        }
    }*/
    
    results = std::list<std::string>();
    
    indices = std::vector<int>(n, 0);
    
    unwrap_string( &results, &indices );
    
    t = clock() - t;

    //std::cout<<"time for Viterbi2: "<<((float)t)/CLOCKS_PER_SEC<<std::endl;
    
    return create_sequence(&results);
}

void correcting_search(corrector * corr, std::string word, int i, double prev, 
    double min, std::mutex * data_write_lock, int * threads, 
    std::mutex * count_lock, std::condition_variable * cv, ViterbiWSA * v)
{
    std::list<entry> * wordList = new std::list<entry>();
    corr->fillPossibleWordListLin(wordList, word, -20, 1000, 1, 1);
    double p;
    //std::cout<<"e "<<std::endl;
    if ((wordList != NULL)&&(!wordList->empty()))
    {
        std::list<entry>::iterator it = wordList->begin();
        while (((p = (it->d + prev)) > min)
            && (it != wordList->end()))
        {
            if (word == it->str)
            {
                ++it;
                continue;
            }
            data_write_lock->lock();
            min = v->update_vectors( i, p, it->str, word.length());
            data_write_lock->unlock();
        }
        ++it;
    }
    //std::cout<<"f "<<std::endl;
    delete wordList;
    count_lock->lock();
    (*threads)--;
    count_lock->unlock();
    cv->notify_all();
}

std::string correct(std::string input, corrector * corr,
    std::string first, sqlite3 * db, ThreadPool * tpool)
{
    if (corr->getWordFreq(input))
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
        return input;
    }
    
    std::string lowered = input;
    std::locale loc;
    for (int i = 0; i < input.length(); ++i)
    {
        lowered[i] = std::tolower(input[i], loc);
    }
    if (corr->getWordFreq(lowered))
    {
        if (first == cmd_begin)
        {
            std::list<entry> wordList = std::list<entry>();
            corr->fillPossibleWordListLin(&wordList, lowered, -20);
            if (!wordList.empty())
            {
                entry e = wordList.front();
                lowered = e.str;
            }
        }
        else
        {
            lowered = bestBigram(first, lowered, corr, db);
        }
        return lowered;
    }
    
    std::list<entry> wordList = std::list<entry>();
    corr->fillPossibleWordListLin(&wordList, input, -20);
    double reasonableFound = 0;
    std::string strFound;
    if (!wordList.empty())
    {
        entry e = wordList.front();
        if (e.d > -25) //TODO: find good threshold
        {
            //std::cout<<"score "<<e.d<<std::endl;
            return e.str;
        }
        if (e.d > -30)
        {
            reasonableFound = e.d;
            strFound = e.str;
        }
    }
    
    wordList = std::list<entry>();
    corr->fillPossibleWordListLin(&wordList, lowered, -20);
    if (!wordList.empty())
    {
        entry e = wordList.front();
        if (e.d > -25) //TODO: find good threshold
        {
            //std::cout<<"4 e.d: "<<e.d<<" confidence: "<<confidence<<" "<<e.str<<std::endl;
            return e.str;
        }
        if (e.d > -30)
        {
            if ((!(reasonableFound))||(e.d > reasonableFound))
            {
                //std::cout<<"4 e.d: "<<e.d<<" confidence: "<<confidence<<" "<<e.str<<std::endl;
                return e.str;
            }
        }
        if (reasonableFound)
        {
            //std::cout<<"3 e.d: "<<reasonableFound<<" confidence: "<<confidence<<" "<<strFound<<std::endl;
            return strFound;
        }
    }
    
    double made_up_word_penalty = 60;
    int acceptable_freq = 200;
    ViterbiWSA v;
    input = v.Viterbi(input, corr, db, made_up_word_penalty, acceptable_freq, 
        4, correcting_search, tpool);
    return input;
}


}
