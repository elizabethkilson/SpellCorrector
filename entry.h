#ifndef _ENTRY_H
#define _ENTRY_H

#include <list>
#include <string>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>

class entry{
	public:
		std::string str;
		double d;
		
		entry(std::string c, double p){str = c; d = p;}
		
		bool operator<(const entry& b){return d < b.d;}
		
		entry(const entry &x)
		{
			str = x.str;
			d = x.d;
		}
		
		entry& operator=(const entry& x)
		{
			str = x.str;
			d = x.d;
			
			return *this;
		}
		
		~entry()
		{
			;//std::cout<<"~entry()"<<std::endl;
		}
};

class dictEntry{
	public:
		std::string str;
		int i;
		
		dictEntry(std::string c, int p){str = c; i = p;}
		
		bool operator<(const dictEntry& b){return i < b.i;}
		
		dictEntry(const dictEntry &x)
		{
			str = x.str;
			i = x.i;
		}
		
		dictEntry& operator=(const dictEntry& x)
		{
			str = x.str;
			i = x.i;
			
			return *this;
		}
		
		~dictEntry()
		{
			;//std::cout<<"~entry()"<<std::endl;
		}
};

/*class mispelled_word{
    public:
        std::string word;
		double word_conf;
		std::vector<double> sym_confs;
		
        class index{
            private:
                std::vector<double> * vec;
            public:
                int ind;
                
                index(int i, std::vector<double> * v){ ind = i; vec = v; }
                
                bool operator<(const index& b) const
                {
                    if ((ind >= vec->size())||(ind < 0))
                        return false;
                    if ((b.ind >= vec->size())||(b.ind < 0))
                        return true;
                    return vec->at(ind) < vec->at(b.ind);
                }
        };
        
		std::vector<index> indices;
		
		void sort_indices()
		{
		    std::sort(indices.begin(), indices.end());
		}
		
		mispelled_word(std::string w, double conf, double * confs)
		{
		    word = w;
		    word_conf = conf;
		    sym_confs = std::vector<double> (confs, confs + word.length());
		    index default_ind = index(0, &sym_confs);
		    indices = std::vector<index> (word.length(), default_ind);
		    for (int i = 1; i < indices.size(); i++)
		    {
		        indices[i].ind = i;
		    }
		    sort_indices();
		}
};*/

class error_item{
	public:
		std::unordered_map<std::string, double> * pv;
		int nObserved;
		
		error_item()
		{
		    pv = NULL;
		    nObserved = 0;
		}
		
		error_item(std::unordered_map<std::string, double> * p, int n)
		{
			pv = p;
			nObserved = n;
		}
		
		~error_item()
		{
			if (pv != NULL)
			    delete pv;
		}
		
		error_item(const error_item &x)
		{
			pv = new std::unordered_map<std::string, double>(*(x.pv));
			nObserved = x.nObserved;
		}
		
		error_item& operator=(const error_item& x)
		{
			delete pv;
			
			pv = new std::unordered_map<std::string, double>(*(x.pv));
			nObserved = x.nObserved;
			
			return *this;
		}
};

#endif
