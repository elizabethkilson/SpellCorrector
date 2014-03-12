#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

std::string LCS(std::string X, std::string Y)
{
    //std::cout<<"a"<<std::endl;
    int m = X.length();
    int n = Y.length();
    //std::cout<<"a1 "<<m<<" "<<n<<std::endl;
    std::vector< std::vector<int> > c(m + 1, std::vector<int>(n + 1, 0));
    //std::cout<<"a2"<<std::endl;
    int i, j;
    for (i = 0; i <= m; i++)
    {
        for (j = 0; j <= n; j++)
        {
            c[i][j] = 0;
        }
    }
    //std::cout<<"b"<<std::endl;
    for (i = 1; i <= m; i++)
    {
        for (j = 1; j <= n; j++)
        {
            if (X[i - 1]==Y[j - 1])
            {
                c[i][j] = c[i - 1][j - 1] + 1;
            }
            else
            {
                c[i][j]=std::max(c[i][j-1],c[i-1][j]);
            }
        }
    } 
    
    --i;
    --j;
    //std::cout<<"c"<<std::endl;
    char * Z = new char[c[i][j] + 1];
    Z[c[i][j]] = '\0';
    //std::cout<<"d"<<std::endl;
    for (int k = c[i][j]; k > 0;)
    {
        if (i==0 || j==0)
            break;
        if (X[i - 1] == Y[j - 1])
        {
            Z[--k] = X[i - 1];
            --i;
            --j;
        }
        else if (c[i][j] == c[i - 1][j])
        {
            --i;
        }
        else
        {
            --j;
        }
    }
    //std::cout<<"e"<<std::endl;
    std::string ret (Z);
    delete[] Z;
    //std::cout<<"f"<<std::endl;
    return ret;
}

std::string format_white_space(std::string orig)
{
    char * ret = new char[orig.length() + 1];
    int i = 0;
    while (isspace(orig[i]) && (i < orig.length()))
    {
        ++i;
    }
    bool inWS = false;
    int j;
    for (j = 0; i < orig.length(); i++)
    {
        if (isspace(orig[i]))
        {
            if (!inWS)
            {
                ret[j++] = ' ';
                inWS = true;
            }
        }
        else
        {
            inWS = false;
            ret[j++] = orig[i];
        }
    }
    ret[j] = '\0';
    std::string r (ret);
    delete[] ret;
    return r;
}

std::string strip_punc(std::string orig)
{
    char * ret = new char[orig.length() + 1];
    int j;
    for (j = 0; j < orig.length(); j++)
    {
        if (ispunct(orig[j]) && ((orig[j] != '\'') || (orig[j] != '-') || (orig[j] != '.') || (orig[j] != ',')))
        {
            ret[j] = ' ';
        }
        else
        {
            ret[j] = orig[j];
        }
    } 
    ret[j] = '\0';
    std::string r (ret);
    delete[] ret;
    return r;
}

int word_count(std::string orig)
{
    std::string stripped = strip_punc(orig);
    std::stringstream sLine (stripped);
    int count = 0;
    while (sLine>>stripped)
    {
        count++;
    }
    return count;
}

bool eoph (std::string const &fullString, std::string const &endings)
{
    if (fullString.length() < 1)
        return false;
    char end = fullString[fullString.length() - 1];
    for (int i = 0; i < endings.length(); i++)
    {
        if (end == endings[i])
            return true;
    }
    return false;
}
