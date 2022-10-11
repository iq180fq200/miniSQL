#ifndef __Record__Manager__header__
#define __Record__Manager__header__

#include "global.h"
#include "BufferManager.h"
#include "IndexManager.h"
//#include ""
using namespace std;


class RecordManager
{
public:
	DATA select(Table& T, vector<Condition>& con, char op ='a');

	RecordResult insert(Table& T, vector <string>& value);

	RecordResult deleteR (Table& T, vector<Condition>&con, char op='a');

	using TableResult = RecordResult;
	TableResult create(Table& T);
	TableResult drop(const Table& T);
	void createIndex(Table&, Index&);

private:
	void SetCondition(bool&, vector<Condition>&,char);

	void ScanTable(Table&,DATA&, vector<Condition>&, bool negative=0);
	void ScanTable(Table&, RecordResult&, vector<Condition>&, bool negative = 0);
	inline bool comp(Table&,Condition&, Condition&);
	void MySort(Table&, vector<Condition> &);
	inline void splitRow(const string&, Row&, Table&);
	inline void testifyOneRow(Row&, vector<Condition>&, bool&, DATA&,Table&);
	inline bool testifyOneRow(Row&, vector<Condition>&, bool&,Table&);
	void deleteIndex(Table&, Row&);
	inline void insertIndex(Table&, Row&,const Pointer&);
	inline void insertIndex(Table&, Row&, const Pointer&,string&);
	inline void checkDuplicate(Table&, vector<string>&, RecordResult&);

};




#endif
