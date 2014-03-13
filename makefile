default: spellCheck.exe

spellCheck.exe: spellCheck.o corrector.o string_functions.o
	g++ -std=c++11 -o spellCheck.exe spellCheck.o corrector.o string_functions.o -lsqlite3
	
string_functions.o: string_functions.cpp
	g++ -std=c++11 -c string_functions.cpp

spellCheck.o: corrector.o spellCheck.cpp entry.h corrector.h
	g++ -std=c++11 -c spellCheck.cpp

corrector.o: corrector.cpp string_functions.cpp entry.h corrector.h
	g++ -std=c++11 -c corrector.cpp

clean:
	rm -f *.exe *.o *.stackdump *~
