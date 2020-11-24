g++ = C:\Users\vssen\Downloads\MinGW\bin\g++.exe

options = -Wall -Wpedantic -Wextra

input = PMI.txt

run : Akinator.exe
	Akinator.exe $(input)

Akinator.exe : Akinator.cpp stack.o ReadFile.cpp
	$(g++) Akinator.cpp stack.o -o Akinator.exe $(options) -w

stack.o : stack.cpp stack.h
	$(g++) -c stack.cpp -o stack.o $(options) -w