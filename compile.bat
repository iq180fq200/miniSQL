cls && g++ -std=c++11 -g -c src/IndexManager.cpp src/global.cpp src/BPlusTree.cpp src/Page.cpp src/BufferManager.cpp src/Catalog.cpp src/Interpreter.cpp src/RecordManager.cpp main.cpp

cls && g++ -std=c++11 -g main.o global.o BPlusTree.o Page.o BufferManager.o Catalog.o Interpreter.o RecordManager.o IndexManager.o -o src/tast.exe

cls && rm -f main.o global.o BPlusTree.o Page.o BufferManager.o Catalog.o Interpreter.o RecordManager.o IndexManager.o 

pause