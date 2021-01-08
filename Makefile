g++ = g++

options = -Wall -Wpedantic -Wextra

input = Data.txt

bin = bin
src = src

run : $(bin)\Akinator.exe
	$(bin)\Akinator.exe $(input)

$(bin)\Akinator.exe : $(bin)\Akinator.o $(bin)\stack.o
	$(g++) $(bin)\Akinator.o $(bin)\stack.o -o $(bin)\Akinator.exe $(options) -w

$(bin)\stack.o : $(src)\stack.cpp $(src)\stack.h
	$(g++) -c $(src)\stack.cpp -o $(bin)\stack.o $(options) -w

$(bin)\Akinator.o : $(src)\Akinator.cpp $(src)\stack.h $(src)\ReadFile.cpp
	$(g++) -c $(src)\Akinator.cpp -o $(bin)\Akinator.o $(options) -w