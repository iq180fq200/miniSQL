

#ifndef _MINISQL_GLOBAL_H_
#define _MINISQL_GLOBAL_H_
#define  _CRT_SECURE_NO_WARNINGS
//#include <unistd.h>
#include <windows.h>
#include <sys/types.h>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <assert.h>
#include <map>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "QAQ_head.h"
using namespace std;

// pageSize is an integral multiple of KB, define as 4KB.
#define PAGE_SIZE 4096
// cache capacity, 10 page as defined
#define CACHE_CAPACITY 10
// not found the page in cache
#define PAGE_NOTFOUND -1

#define defaultMaxChildrens 3


#define INTLENGTH 11
#define FLOATLENGTH 10
#define INTLEN		11
#define FLOATLEN	10

typedef unsigned int Pointer;
typedef unsigned int uint;
typedef int PageIndex;
typedef int FileHandle;
typedef int BlockOffset;
enum Comparison { Lt, Le, Gt, Ge, Eq, Ne };
enum PageType {
    Undefined,
    RecordPage,
    IndexPage,
};

class Attribute
{
public:
	string name;
	enum DataType type;
	int length;
	bool isPrimaryKey;
	bool isUnique;
	Attribute()
	{
		isPrimaryKey = false;
		isUnique = false;
	}
	Attribute(string n, enum DataType t, int l, bool isP, bool isU)
		:name(n), type(t), length(l), isPrimaryKey(isP), isUnique(isU) {}
	virtual ~Attribute() {}
};

class Table
{
public:
	string name;
	int blockNum;
	int attriNum;
	int totalLength;
	vector<Attribute> attributes;
	Table() : blockNum(0), attriNum(0), totalLength(0) {}
	virtual ~Table() {}
};

class Index
{
public:
	string index_name;
	string table_name;
	string attribute_name;
	int column;
	int columnLength;
	int blockNum;
	Index() : column(0), blockNum(0) {}
	virtual ~Index() {}
};

class Condition {
public:
	Comparison op;
	int columnNum;
	string value;
	virtual ~Condition() {}
};

struct RecordResult
{
	bool status;
	string Reason;
	RecordResult() :status(1) {}
};
struct Row
{
	vector<string> DataField;
};

struct DATA
{
	vector<Row> ResultSet;
};
#endif
