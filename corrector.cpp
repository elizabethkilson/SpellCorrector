#include "corrector.h"
#include <fstream>
#include <list>
#include <string>
#include <sstream>
#include <iostream>

corrector::corrector()
{
    std::cout<<"corrector()"<<std::endl;
    dictionary = new std::vector<std::list<dictEntry> *> (10, NULL);
    dictionaryMap = new std::unordered_map<std::string, int>();
    errors = new std::unordered_map<std::string, error_item>();
    nErrors = 0;
    nTotal = 0;
    nWords = 0;
}

void corrector::loadErrors(std::string filename)
{
	std::fstream fs (filename.c_str(), std::fstream::in);
	fs>>nErrors;
	fs>>nTotal;
	char trash [10];
	fs.getline(trash, 10);
	delete errors;
	errors = new std::unordered_map<std::string, error_item>(nErrors);
	std::string line;
	std::string tStr;
	std::string cStr;
	int nObserved;
	double percent;
	int pos;
	int count = 0;
	while(getline(fs, line))
	{
		std::stringstream sLine (line);
		sLine>>tStr;
		while ((pos = tStr.find("space"))!= std::string::npos)
		{
		    tStr = tStr.substr(0, pos) + " " + tStr.substr(pos + 5);
		}
		sLine>>nObserved;
		std::unordered_map<std::string, double> * lp = new 
		    std::unordered_map<std::string, double>();
		while (sLine>>cStr)
		{
			sLine>>percent;
			while ((pos = cStr.find("space"))!= std::string::npos)
			{
			    cStr = cStr.substr(0, pos) + " " + cStr.substr(pos + 5);
			}
			//entry e (cStr, percent);
			lp->insert({cStr, percent});
		}
		error_item item (lp, nObserved);
		errors->insert({tStr, item});
		count++;
	}
	std::cout<<"read "<<count<<" lines from "<<filename<<std::endl;
}

void corrector::writeErrors(std::string filename)
{
	std::fstream fs (filename.c_str(), std::fstream::out);
	fs<<nErrors<<std::endl;
	fs<<nTotal<<std::endl;
	std::list<std::string> * keys = new std::list<std::string>;
	std::list<std::string> * keys2 = new std::list<std::string>;
	for (auto it = errors->begin(); it != errors->end(); ++it)
	{
		keys->push_back(it->first);
	}
	keys->sort();
	std::string cStr;
	int pos;
	for (std::list<std::string>::iterator it=keys->begin(); it != keys->end(); ++it)
	{
		error_item item = errors->at(*it);
		cStr = (*it);
		while ((pos = cStr.find(" "))!= std::string::npos)
		{
		    cStr = cStr.substr(0, pos) + "space" + cStr.substr(pos + 1);
		}
		fs<<cStr<<' '<<item.nObserved;
		std::unordered_map<std::string, double> * lp = item.pv;
		
		delete keys2;
		keys2 = new std::list<std::string>;
		
		for (auto it2 = lp->begin(); it2 != lp->end(); ++it2)
	    {
		    keys2->push_back(it2->first);
	    }
	    keys2->sort();
	    for (std::list<std::string>::iterator it3 = keys2->begin(); 
	        it3 != keys2->end(); ++it3)
        {
            cStr = (*it3);
            while ((pos = cStr.find(" "))!= std::string::npos)
			{
			    cStr = cStr.substr(0, pos) + "space" + cStr.substr(pos + 1);
			}
			fs<<' '<<cStr<<' '<<lp->at(*it3);
        }
		/*lp->sort();
		lp->reverse();
		for(std::list<entry>::iterator it2=lp->begin(); it2 != lp->end(); ++it2)
		{
		    cStr = it2->str;
		    while ((pos = cStr.find(" "))!= std::string::npos)
			{
			    cStr = cStr.substr(0, pos) + "space" + cStr.substr(pos + 1);
			}
		    fs<<' '<<cStr<<' '<<it2->d;
		}*/
		fs<<std::endl;
	}
	delete keys;
	delete keys2;
}

void corrector::loadDictionary(std::string filename)
{
	std::fstream fs (filename.c_str(), std::fstream::in);
	
	delete dictionary;
	dictionary = new std::vector<std::list<dictEntry> *> (10, NULL);
	delete dictionaryMap;
	dictionaryMap = new std::unordered_map<std::string, int>();
	
	std::string word;
	std::string line;
	int score;
	
	getline(fs, line);
	
	std::list<dictEntry> * wordList;
	
	while(getline(fs, line))
	{
	    std::stringstream sLine (line);
	    sLine>>word>>score;
	    int wLen = word.length();
	    if (wLen > dictionary->size())
	    {
	        dictionary->resize(wLen, NULL);
	    }
	    wordList = dictionary->at(wLen - 1);
	    if (wordList == NULL)
	    {
	        dictionary->at(wLen - 1) = new std::list<dictEntry>();
	        wordList = dictionary->at(wLen - 1);
	    }
		wordList->push_back(dictEntry(word, score));
		dictionaryMap->insert({word, score});
		nWords+=score;
	}
	
	for (int i = 0; i < dictionary->size(); i++)
	{
	    wordList = dictionary->at(i);
	    if (wordList)
	    {
	        wordList->sort();
	        wordList->reverse();
	    }
	}
}

void corrector::addWord(std::string word, int score)
{
    int wLen = word.length();
    if (wLen > dictionary->size())
    {
        dictionary->resize(wLen, NULL);
    }
    std::list<dictEntry> * wordList = dictionary->at(wLen - 1);
    if (wordList == NULL)
    {
        dictionary->at(wLen - 1) = new std::list<dictEntry>();
        wordList = dictionary->at(wLen - 1);
    }
    
    wordList->push_back(dictEntry(word, score));
	nWords+=score;
	
	wordList->sort();
    wordList->reverse();
}

void corrector::addError(std::string tString, std::string cString)
{
    if (tString.length() == 0)
        return;
    if (cString.length() == 0)
        return;
	if (errors->count(tString))
	{
		error_item item = errors->at(tString);
		std::unordered_map<std::string, double> * lp = item.pv;
		int N = item.nObserved;
		int found = 0;
		if (lp->count(cString))
		{
		    lp->at(cString) += 1;
		}
		else
		{
		    lp->insert({cString, 1.0});
		}
		/*for (std::list<entry>::iterator it=lp->begin(); it != lp->end(); ++it)
		{
			if (it->str == cString)
			{
				found = 1;
				double nOcc = it->d;
				it->d = nOcc + 1;
			}
		}
		if (found==0)
		{
			entry e (cString, 1.0);
			lp->push_back(e);
		}*/
		item.nObserved = N + 1;
		errors->at(tString) = item;
	}
	else
	{
		std::unordered_map<std::string, double> * lp = new 
		    std::unordered_map<std::string, double>();
		lp->insert({cString, 1});
		
		error_item item (lp, 1);
		errors->insert({tString, item});
		nErrors++;
	}
	nTotal++;
}

void corrector::learn(std::string tFilename, std::string cFilename)
{
    std::cout<<"learn()"<<std::endl;
    std::fstream tFs (tFilename.c_str(), std::fstream::in);
    std::fstream cFs (cFilename.c_str(), std::fstream::in);
    
    std::string tLine;
    std::string cLine;
    std::string t;
    std::string c;
    
    char * tError = new char[MAX_ERROR_LEN + 1];
    char * cCorrection = new char[MAX_ERROR_LEN + 1];
    
    while(getline(tFs, tLine) && getline(cFs, cLine))
    {
        while(tLine.length() == 0)
        {
            getline(tFs, tLine);
            tLine = format_white_space(tLine);
        }
        while(cLine.length() == 0)
        {
            getline(cFs, cLine);
            cLine = format_white_space(cLine);
        }
        std::string common = LCS(tLine, cLine);
        
        /*std::string puncFree = strip_punc(cLine);
        std::stringstream sLine (puncFree);
        std::string word;
        while (sLine>>word)
        {
            addWord(word);
        }*/
        
        if (common.length() < tLine.length()/2)
        {
            std::cout<<"ERROR: lines not similar"<<std::endl;
            std::cout<<cLine<<std::endl;
            std::cout<<tLine<<std::endl;
            std::cout<<common<<std::endl;
            if (cLine.length() > 10)
                break;
            continue;
        }
        
        int i = 0;
        int j = 0;
        int k;
        int l;
        bool overflow;
        
        for (k = 0; (k < common.length()) && (i < tLine.length()) &&
            (j < cLine.length()); k++)
        {
            if ((common[k] == tLine[i])&&(common[k]==cLine[j]))
            {
                ++j;
                ++i;
                continue;
            }
            l = 0;
            overflow = false;
            if (common[k] == tLine[i])//deletion
            {
                while ((common[k] != cLine[j])&&(!overflow)&&(j < cLine.length()))
                {
                    if (!(isalnum(cLine[j])||ispunct(cLine[j])||isspace(cLine[j])))
                    {
                        overflow = true;
                        break;
                    }
                    cCorrection[l++] = cLine[j++];
                    overflow = l > MAX_ERROR_LEN;
                }
                if (overflow)
                {
                    while ((common[k] != cLine[j]) && (j < cLine.length()))
                        ++j;
                    ++i;
                    ++j;
                    continue;
                }
                cCorrection[l] = '\0';
                c = std::string(cCorrection);
                
                int pos;
                while ((pos = c.find(" "))!= std::string::npos)
		        {
		            addError("deletion", c.substr(0, pos));
		            addError(" Del", " ");
		            c = c.substr(pos + 1);
		        }
                
                addError("deletion", c);
                
                ++i;
                ++j;
                continue;
            }
            if (common[k] == cLine[j])//insertion
            {
                while ((common[k] != tLine[i])&&(!overflow)&&(i < tLine.length()))
                {
                    if (!(isalnum(tLine[i])||ispunct(tLine[i])||isspace(tLine[i])))
                    {
                        overflow = true;
                        break;
                    }
                    tError[l++] = tLine[i++];
                    overflow = l > MAX_ERROR_LEN;
                }
                if (overflow)
                {
                    while ((common[k] != tLine[i]) && (i < tLine.length()))
                        ++i;
                    ++i;
                    ++j;
                    continue;
                }
                tError[l] = '\0';
                c = std::string(tError);
                
                int pos;
                while ((pos = c.find(" "))!= std::string::npos)
		        {
		            addError("insertion", c.substr(0, pos));
		            addError(" Ins", " ");
		            c = c.substr(pos + 1);
		        }
                
                addError("insertion", c);
                ++i;
                ++j;
                continue;
            }
            
            while ((common[k] != tLine[i])&&(!overflow)&&(i < tLine.length()))
            {
                if (!(isalnum(tLine[i])||ispunct(tLine[i])||isspace(tLine[i])))
                {
                    overflow = true;
                    break;
                }
                tError[l++] = tLine[i++];
                overflow = l > MAX_ERROR_LEN;
            }
            if (overflow)
            {
                while ((common[k] != tLine[i]) && (i < tLine.length()))
                    ++i;
                while ((common[k] != cLine[j]) && (j < cLine.length()))
                    ++j;
                ++i;
                ++j;
                continue;
            }
            tError[l] = '\0';
            l = 0;
            while ((common[k] != cLine[j])&&(!overflow)&&(j < cLine.length()))
            {
                if (!(isalnum(cLine[j])||ispunct(cLine[j])||isspace(cLine[j])))
                {
                    overflow = true;
                    break;
                }
                cCorrection[l++] = cLine[j++];
                overflow = l > MAX_ERROR_LEN;
            }
            if (overflow)
            {
                while ((common[k] != cLine[j]) && (j < cLine.length()))
                    ++j;
                ++i;
                ++j;
                continue;
            }
            cCorrection[l] = '\0';
            t = std::string(tError);
            c = std::string(cCorrection);
            
            ++i;
            ++j;
            
            if (((c.find(" ")) != std::string::npos) && 
                ((t.find(" ")) != std::string::npos))
            {//both contain spaces. No unique description of error.
                continue;
            }
            
            if (c == " ")
                c = "";
            
            if (t == " ")
                t = "";
            
            if ((c.find(" ")) != std::string::npos)
            {
                while (c.find(" ") == 0)
	            {
	                addError(" Del", " ");
	                c = c.substr(1);
	            }
	            while (c.find(" ") == c.length() - 1)
	            {
	                addError(" Del", " ");
	                c = c.substr(0, c.length() - 1);
	            }
	        }
	        if ((t.find(" ")) != std::string::npos)
	        {
	            while (t.find(" ") == 0)
	            {
	                addError(" Ins", " ");
	                t = t.substr(1);
	            }
	            while (t.find(" ") == t.length() - 1)
	            {
	                addError(" Ins", " ");
	                t = t.substr(0, t.length() - 1);
	            }
	        }
	        
	        if (((c.find(" ")) != std::string::npos) || 
                ((t.find(" ")) != std::string::npos))
            {//contains spaces in middle. No unique description of error.
                continue;
            }
            
            if (t.length() < 1)
            {
                addError("deletion", c);
                continue;
            }
            if (c.length() < 1)
            {
                addError("insertion", t);
                continue;
            }
            
            addError(t, c);
        }
    }
    
    delete[] tError;
    delete[] cCorrection;
}

double corrector::checkDelIns(std::string str, double maxErr, std::string type)
{
    //std::cout<<"checkDelIns"<<std::endl;
    if (str.length() == 0)
    {
        return 0;
    }
    error_item item = errors->at(type);
    std::unordered_map<std::string, double> * lp = item.pv;
    for (int i = 0; i < str.length(); i++)
    {
        double tmpPenalty = 0;
        if (!(lp->count(str.substr(0, str.length() - i))))
            continue;
        tmpPenalty = log(lp->at(str.substr(0, str.length() - i))) 
            - log(nTotal) + 
            checkDelIns(str.substr(str.length() - i), maxErr - 1, type);
        maxErr = std::max(tmpPenalty, maxErr);
    }
    return maxErr;
}

double corrector::checkSub(std::string tStr, std::string cStr, double maxErr)
{
    //std::cout<<"checksub"<<std::endl;
    if (tStr.length() == 0)
    {
        return checkDelIns(cStr, maxErr - 1, "deletion");
    }
    if (cStr.length() == 0)
    {
        return checkDelIns(tStr, maxErr - 1, "insertion");
    }
    for (int i = 0; i <= tStr.length(); i++)
    {
        if (!errors->count(tStr.substr(0, tStr.length() - i)))
            continue;
        error_item item = errors->at(tStr.substr(0, tStr.length() - i));
        std::unordered_map<std::string, double> * lp = item.pv;
        double tmpPenalty = maxErr;
        for (int j = 0; j <= cStr.length(); j++)
        {
            if (!(lp->count(cStr.substr(0, cStr.length() - j))))
                continue;
            tmpPenalty = log(lp->at(cStr.substr(0, cStr.length() - j)))
                - log(nTotal) + 
                checkSub(tStr.substr(tStr.length() - i), 
                    cStr.substr(cStr.length() - j), maxErr - 1);
            maxErr = std::max(tmpPenalty, maxErr);
        }
    }
    return maxErr;
}

double corrector::findMinErrorVal(std::string tStr, std::string cStr, double maxErr)
{
    //std::cout<<"findMinErrorVal"<<std::endl;
    if (tStr.length() == 0) //deletion
    {
        return checkDelIns(cStr, maxErr - 1, "deletion");
    }
    
    if (cStr.length() == 0) //insertion
    {
        return checkDelIns(tStr, maxErr - 1, "insertion");
    }
    
    //substitution
    return checkSub( tStr, cStr, maxErr - 1);
}

void corrector::do_fillPossibleWordList( std::list<entry> * wordList, 
    std::string word, double maxErr, int forcedMinFreq, int * allowedMinFreq, 
    std::list<dictEntry> * dictList, double * bestFound, double wordPenalty)
{
    std::cout<<"do_start"<<std::endl;
    if (dictList == NULL)
        return;
    //std::cout<<"dictList not null"<<std::endl;
    if (dictList->empty())
        return;
    //std::cout<<"dictList not empty"<<std::endl;
    std::list<dictEntry>::iterator it;
    int i;
    int j;
    int k;
    //std::cout<<"a"<<std::endl;
    for ( it = dictList->begin(); it != dictList->end(); ++it)
    {
        int score = it->i;
        if ((score < (*allowedMinFreq)) && (score < forcedMinFreq))
            break;
        std::string dictWord = it->str;
        std::string common = LCS(word, dictWord);
        if ((dictWord.length() > 1) || (word.length() > 1))
        {
            if (common.length() < 
                std::max(dictWord.length(), word.length())/2)
                continue;
        }
        double penalty = 0;
        i = 0;
        j = 0;
        //std::cout<<"c"<<std::endl;
        for (k = 0; (k < common.length()) && (i < word.length()) &&
            (j < dictWord.length()); k++)
        {
            if (penalty < maxErr)
                break;
            //std::cout<<"i "<<i<<" j "<<j<<" k "<<k<<std::endl;
            //std::cout<<word[i]<<" "<<dictWord[j]<<" "<<common[k]<<std::endl;
            if ((common[k] == word[i])&&(common[k] == dictWord[j]))
            {
                ++j;
                ++i;
                continue;
            }
            //std::cout<<"d"<<std::endl;
            
            if (common[k] == word[i])//deletion
            {
                std::string deleted = "";
                while ((common[k] != dictWord[j])&&(j < dictWord.length()))
                {
                    deleted += dictWord[j++];
                }
                ++i;
                ++j;
                penalty += findMinErrorVal("", deleted, maxErr);
                continue;
            }
            //std::cout<<"e"<<std::endl;
            
            if (common[k] == dictWord[j])//insertion
            {
                std::string inserted = "";
                while ((common[k] != word[i])&&(i < word.length()))
                {
                    inserted += word[i++];
                }
                ++i;
                ++j;
                penalty += findMinErrorVal(inserted, "", maxErr);
                continue;
            }
            //std::cout<<"f"<<std::endl;
            std::string tStr = "";
            std::string cStr = "";
            
            //std::cout<<"i "<<i<<" j "<<j<<" k "<<k<<std::endl;
            
            while ((common[k] != dictWord[j])&&(j < dictWord.length()))
            {
                cStr += dictWord[j++];
            }
            
            while ((common[k] != word[i])&&(i < word.length()))
            {
                tStr += word[i++];
            }
            //std::cout<<"g"<<std::endl;
            //std::cout<<tStr<<" "<<cStr<<std::endl;
            penalty += findMinErrorVal(tStr, cStr, maxErr);
            ++i;
            ++j;
            //std::cout<<"h"<<std::endl;
        }
        
        if (penalty <= maxErr)
            continue;
        
        if ((i < word.length()) || (j < dictWord.length()))
        {
            std::string tStr = "";
            std::string cStr = "";
            
            while (j < dictWord.length())
            {
                cStr += dictWord[j++];
            }
            
            while (i < word.length())
            {
                tStr += word[i++];
            }
            penalty += findMinErrorVal(tStr, cStr, maxErr);
        }
        
        if (penalty <= maxErr)
            continue;
        
        double total = penalty + log(score) - wordPenalty;
        wordList->push_back(entry(dictWord, total));
        if (total > (*bestFound))
        {
            (*allowedMinFreq) = std::max((*allowedMinFreq), 
                (int)(score*exp(penalty)));
            (*bestFound) = total;
        }
        //std::cout<<dictWord<<" "<<penalty<<" "<<score<<std::endl;
    }
}

void corrector::fillPossibleWordListLin( std::list<entry> * wordList, 
    std::list<dictEntry> * choices, double wordPenalty,
    std::string word, double maxErr, int forcedMinFreq, int allowedMinFreq)
{
    double bestFound = maxErr - log(nWords);
    if (word.length() < 1)
        return;
    do_fillPossibleWordList( wordList, word, maxErr, forcedMinFreq,
        &allowedMinFreq, choices, &bestFound, wordPenalty);
    
    wordList->sort();
    wordList->reverse();
}

void corrector::fillPossibleWordListLin( std::list<entry> * wordList, 
    std::string word, double maxErr, int forcedMinFreq, int allowedMinFreq, 
    int maxLenDev)
{
    int len = word.length();
    double bestFound = maxErr - log(nWords);
    if (len < 1)
        return;
    double wordPenalty = log(nWords);
    
    if (len <= dictionary->size())
    {
        do_fillPossibleWordList( wordList, word, maxErr, forcedMinFreq, 
            &allowedMinFreq, dictionary->at(len - 1), &bestFound, wordPenalty);
    }
    for (int i = 1; i <= maxLenDev; i++)
    {
        if ((len + i) <= dictionary->size())
        {
            do_fillPossibleWordList( wordList, word, maxErr, forcedMinFreq, 
                &allowedMinFreq, dictionary->at(len + i - 1), 
                &bestFound, wordPenalty);
        }
        if ((len - i) < 1)
            continue;
        do_fillPossibleWordList( wordList, word, maxErr, forcedMinFreq, 
            &allowedMinFreq, dictionary->at(len - i - 1), 
            &bestFound, wordPenalty);
    }
    //std::cout<<allowedMinFreq<<std::endl;
    wordList->sort();
    wordList->reverse();
}

void corrector::tryAllIns(entry e, std::list<entry> * wordList, 
    std::list<entry> * poss, double forcedErr, double * maxErr, bool add)
{
    error_item item = errors->at("deletion");
    std::unordered_map<std::string, double> * mp = item.pv;
    std::string word;
    double penalty;
    entry e2 ("", 0);
    int score;
    for (auto it = mp->begin(); it != mp->end(); ++it)
    {
        penalty = e.d + log(it->second) - log(nTotal);
        if ((penalty > (*maxErr)) || (penalty > forcedErr))
        {
            for (int i = 0; i <= e.str.length(); i++)
            {
                word = e.str.substr(0, i) + it->first + e.str.substr(i);
                if (score = getWordFreq(word))
                {
                    e2 = entry(word, penalty + log(score));
                    wordList->push_back(e2);
                    (*maxErr) = std::max((*maxErr), 2*penalty);
                    std::cout<<word<<" "<<penalty<<" "<<score<<" "<<(*maxErr)<<std::endl;
                }
                else if (wordList->empty())
                {
                    e2 = entry(word, penalty);
                    poss->push_back(e2);
                }
            }
        }
    }
}

void corrector::tryAllDel(entry e, std::list<entry> * wordList, 
    std::list<entry> * poss, double forcedErr, double * maxErr, bool add)
{
    error_item item = errors->at("insertion");
    std::unordered_map<std::string, double> * mp = item.pv;
    std::string word;
    std::string del;
    double penalty;
    entry e2 ("", 0);
    int score;
    for (int i = 0; i < e.str.length(); i++)
    {
        for (int j = 0; (j<=MAX_ERROR_LEN) && ((i+j) <= e.str.length()); j++)
        {
            del = e.str.substr(i, j);
            if (!mp->count(del))
                continue;
            penalty = e.d + log(mp->at(del)) - log(nTotal);
            
            if ((penalty < (*maxErr)) && (penalty < forcedErr))
                continue;
            
            word = e.str.substr(0, i) + e.str.substr(i + j);
            
            if (score = getWordFreq(word))
            {
                e2 = entry(word, penalty + log(score));
                wordList->push_back(e2);
                (*maxErr) = std::max((*maxErr), 2*penalty);
                std::cout<<word<<" "<<penalty<<" "<<score<<" "<<(*maxErr)<<std::endl;
            }
            else if (wordList->empty())
            {
                e2 = entry(word, penalty);
                poss->push_back(e2);
            }
        }
    }
}

void corrector::tryAllSub(entry e, std::list<entry> * wordList, 
    std::list<entry> * poss, double forcedErr, double * maxErr, bool add)
{
    error_item item;
    std::unordered_map<std::string, double> * mp;
    std::string word;
    std::string sub;
    double penalty;
    entry e2 ("", 0);
    int score;
    for (int i = 0; i < e.str.length(); i++)
    {
        for (int j = 0; (j<=MAX_ERROR_LEN) && ((i+j) <= e.str.length()); j++)
        {
            sub = e.str.substr(i, j);
            if (!errors->count(sub))
                continue;
            item = errors->at(sub);
            mp = item.pv;
            for (auto it = mp->begin(); it != mp->end(); ++it)
            {
                penalty = e.d + log(it->second) - log(nTotal);
                
                if ((penalty < (*maxErr)) && (penalty < forcedErr))
                    continue;
                
                word = e.str.substr(0, i) + it->first + e.str.substr(i + j);
                
                if (score = getWordFreq(word))
                {
                    e2 = entry(word, penalty + log(score));
                    wordList->push_back(e2);
                    (*maxErr) = std::max((*maxErr), 2*penalty);
                    std::cout<<word<<" "<<penalty<<" "<<score<<" "<<(*maxErr)<<std::endl;
                }
                else if (wordList->empty())
                {
                    e2 = entry(word, penalty);
                    poss->push_back(e2);
                }
            }
        }
    }
}

void corrector::fillPossibleWordListExp( std::list<entry> * wordList, 
    std::string word, double maxErr, double forcedErr)
{
    std::list<entry> poss;
    
    entry e (word, 0);
    poss.push_back(e);
    
    bool add = true;
    
    while (!poss.empty())
    {
        e = poss.front();
        poss.pop_front();
        
        if (e.d < maxErr)
            break;
        
        tryAllIns(e, wordList, &poss, forcedErr, &maxErr, add);
        
        tryAllDel(e, wordList, &poss, forcedErr, &maxErr, add);
        
        tryAllSub(e, wordList, &poss, forcedErr, &maxErr, add);
        
        if (add)
        {
            poss.sort();
            poss.reverse();
        }
        
        add = wordList->empty();
    }
    
    wordList->sort();
    wordList->reverse();
}





