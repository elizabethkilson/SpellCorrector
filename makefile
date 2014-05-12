SHELL := /bin/bash

CC := g++ --std=c++11

default: threadedSpellCheck.exe spellCheck.exe

threadedSpellCheck.exe: threadedSpellCheck.o threadedSpellCorrector.o corrector.o string_functions.o
	$(CC) -o threadedSpellCheck.exe threadedSpellCheck.o threadedSpellCorrector.o corrector.o string_functions.o -pthread -static -lsqlite3 -ldl

spellCheck.exe: spellCheck.o spellCorrector.o corrector.o string_functions.o
	$(CC) -o spellCheck.exe spellCheck.o spellCorrector.o corrector.o string_functions.o -lsqlite3

../libdistributed/ThreadPool.o: ../libdistributed/ThreadPool.cpp ../libdistributed/ThreadPool.hpp
	$(CC) -c -o ../libdistributed/ThreadPool.o ../libdistributed/ThreadPool.cpp
	
string_functions.o: string_functions.cpp
	$(CC) -c string_functions.cpp

spellCheck.o: spellCheck.cpp entry.h corrector.h spellCorrector.h
	$(CC) -c spellCheck.cpp

threadedSpellCheck.o: threadedSpellCheck.cpp entry.h corrector.h threadedSpellCorrector.h ../libdistributed/ThreadPool.hpp
	$(CC) -c threadedSpellCheck.cpp

spellCorrector.o: spellCorrector.cpp spellCorrector.h entry.h corrector.h
	$(CC) -c spellCorrector.cpp

threadedSpellCorrector.o: threadedSpellCorrector.cpp threadedSpellCorrector.h entry.h corrector.h
	$(CC) -c threadedSpellCorrector.cpp

corrector.o: corrector.cpp string_functions.cpp entry.h corrector.h
	$(CC) -c corrector.cpp

clean:
	rm -f *.exe *.o *.stackdump *~
