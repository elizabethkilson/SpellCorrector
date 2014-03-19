default: threadedSpellCheck.exe

threadedSpellCheck.exe: threadedSpellCheck.o threadedSpellCorrector.o corrector.o string_functions.o
	clang++ -std=c++11 -o spellCheck.exe threadedSpellCheck.o threadedSpellCorrector.o corrector.o string_functions.o -lsqlite3

spellCheck.exe: spellCheck.o spellCorrector.o corrector.o string_functions.o
	g++ -std=c++11 -o spellCheck.exe spellCheck.o spellCorrector.o corrector.o string_functions.o -lsqlite3
	
string_functions.o: string_functions.cpp
	g++ -std=c++11 -c string_functions.cpp

spellCheck.o: spellCheck.cpp entry.h corrector.h spellCorrector.h
	g++ -std=c++11 -c spellCheck.cpp

threadedSpellCheck.o: threadedSpellCheck.cpp entry.h corrector.h threadedSpellCorrector.h
	clang++ -std=c++11 -c threadedSpellCheck.cpp

spellCorrector.o: spellCorrector.cpp spellCorrector.h entry.h corrector.h
	g++ -std=c++11 -c spellCorrector.cpp

threadedSpellCorrector.o: threadedSpellCorrector.cpp threadedSpellCorrector.h entry.h corrector.h
	clang++ -std=c++11 -c threadedSpellCorrector.cpp

corrector.o: corrector.cpp string_functions.cpp entry.h corrector.h
	g++ -std=c++11 -c corrector.cpp

clean:
	rm -f *.exe *.o *.stackdump *~
