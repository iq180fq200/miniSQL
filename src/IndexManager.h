

#ifndef _MINISQL_INDEXMANAGER_H_
#define _MINISQL_INDEXMANAGER_H_

#include "global.h"
#include "BPlusTree.h"
using namespace std;

std::string GenIndexName(std::string table_name,std::string index_name);

class IndexManager
{
public:
	std::string bptIntName;
	std::string bptDoubleName;
	std::string bptCharName;
	BPlusTree<int>* bptInt;
	BPlusTree<double>* bptDouble;
	BPlusTree<string>* bptChar;

public:
	IndexManager();
	bool CreateIndex(Table& table, Index& index);
	bool CreateIndex(Table& table);
	bool HasIndex(const std::string& TableName, std::string& AttribName);
	bool DropIndex(const std::string& TableName, std::string& AttribName);
	void RstBptInt(bool save);
	void RstBptChar(bool save);
	void RstBptDouble(bool save);
	void SaveIndex();
	bool LoadIndex(std::string& TableName, std::string& AttribName, enum DataType type);
	bool InsertItem(const std::string& value, Pointer pointer, enum DataType type);
	bool Delete(std::string& value, enum DataType type);
	bool DeletePointers(std::vector<Pointer>& pointers, enum DataType type);
	bool SelectItem(Condition& cond, enum DataType type, std::vector<Pointer>& pointers);
	Pointer FindPointer(std::string& value, enum DataType type);
	bool Find(std::string& value, enum DataType type);
	//void printS();//test
	virtual ~IndexManager();
};

#endif
