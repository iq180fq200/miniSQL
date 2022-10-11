#include "RecordManager.h"
#include "BufferManager.h"
#include "IndexManager.h"
#include "Catalog.h"
#include "BPlusTree.h"
extern BufferManager buf;
extern CatalogManager catalog;
extern IndexManager index;
using namespace std;
//temporary name

RecordResult RecordManager::insert(Table& T, vector<string>& value) {

	RecordResult insertResult;
	string row;
	Row onerow;
	for (size_t i = 0; i < T.attriNum; ++i)
	{
		row += value[i];
		//if (value[i].length() < T.attributes[i].length)
		for (size_t j = 0; j < T.attributes[i].length - value[i].length(); j++)
		{
			row +="@";
		}
	}
		
	row +="0";

	//Page &readIn =buf.recordManagerGetBlankPage();
	//readIn.tableName = T.name;
	//readIn.ofs = T.blockNum;
	//7.3
	
	Page tempPage;
	tempPage.tableName = T.name;
	tempPage.ofs = T.blockNum;
	tempPage.pageType = RecordPage;
	Page& readIn = tempPage;
	

	char save;
	//readIn.pageIndex = T.blockNum-1;
	//read in the last block
	//try if i can insert
	buf.readPage(readIn);
	checkDuplicate(T, value,insertResult);
	//cout << (void*)readIn.pageData << endl;
	if (!insertResult.status)
	{ 
		return insertResult;
	}
	char* last1=nullptr ,*last2 = nullptr;
	for (size_t i = 0; i < strlen(readIn.pageData); i+=T.totalLength)
	{
		if (readIn.pageData[i + T.totalLength-1]=='1')
		{
			last2 = readIn.pageData + i + T.totalLength-1;
			break;
		}
	}	
	for (size_t i = PAGE_SIZE-1; i+1 ; i -= T.totalLength)
	{
		if (readIn.pageData[i]=='@')
		{
			i += T.totalLength - 1;
			continue;
		}
		if (readIn.pageData[i]=='1')
			continue;
		if (readIn.pageData[i]=='0') {
			last1 = readIn.pageData + i+1;
			break;
		}
		if (readIn.pageData[i]== 0) {
			last1 = readIn.pageData + i ;
			break;
		}
	}
	if (last2 >= readIn.pageData)
	{
		last2 -= T.totalLength;
		last2++;
	}
	if (last2)
	{
		save =*(last2 + T.totalLength);
		splitRow(row, onerow, T);
		insertIndex(T, onerow, (readIn.ofs) *PAGE_SIZE+last2-readIn.pageData );
		strcpy(last2 , row.c_str());
		last2[T.totalLength]= save;
	} 
	else if (last1<=readIn.pageData) {
		//save =*(readIn.pageData + T.totalLength);
		splitRow(row, onerow, T);
		insertIndex(T, onerow, (readIn.ofs) * PAGE_SIZE );
		strcpy(readIn.pageData, row.c_str());
		//readIn.pageData[T.totalLength]= save;
	}
	else if (PAGE_SIZE-(last1-readIn.pageData)<T.totalLength)
	{
		readIn.ofs =++T.blockNum;
		readIn.tableName = T.name;
		catalog.changeTable(T);
		buf.readPage(readIn);
		splitRow(row, onerow, T);
		insertIndex(T, onerow, (T.blockNum *PAGE_SIZE));
		//save =*(readIn.pageData + T.totalLength);
		strcpy(readIn.pageData, row.c_str());
		//readIn.pageData[T.totalLength]= save;
		insertResult.status = 1;
	}
	else
	{
		splitRow(row, onerow, T);
		insertIndex(T, onerow, (readIn.ofs) * PAGE_SIZE+(last1-readIn.pageData));
		//last1--;
		//save =*(last1 + T.totalLength);
		if (*last1 == 0)
			*last1 = 1;
		strcpy(last1, row.c_str());
		//last1[T.totalLength]= save;
	}
	buf.writePage(readIn);
	return insertResult;
}
//working fine.
DATA RecordManager::select(Table& T, vector<Condition>& con, char op) {

	bool negative = 0;
	DATA result;
	//will only output all tuples that satisfy the Conditions.
	//return in DATA pattern.
	if (T.name =="") {
		cout << "No such table!" << endl;
		return result;
	}
	negative =(op =='a') ? 0 : 1;
	if (con.size()) {
		SetCondition(negative, con, op);
		MySort(T, con);
	}
	ScanTable(T, result, con, negative);
	return result;


}

RecordResult RecordManager::deleteR(Table& T, vector<Condition>& con, char op) {
	bool negative = 0;
	RecordResult deleteResult;
	deleteResult.status = 1;
	//will only output all tuples that satisfy the Conditions.
	//return in DATA pattern.
	negative =(op =='o') ? 1 : 0;
	if (con.size()) {
		SetCondition(negative, con, op);
		MySort(T, con);
	}
	ScanTable(T, deleteResult, con, negative);
	//maybe should set status to sth but not yet figured out.
	return deleteResult;
}


using TableResult = RecordResult;

TableResult RecordManager::drop(const Table& T) {
	TableResult res;
	for (auto iter : T.attributes)
	{
		if (index.HasIndex(T.name, iter.name))
			if (!index.DropIndex(T.name, iter.name))
				res.status = 0, res.Reason ="Can't remove index files.";
	}
	string FileName = T.name + ".record";
	if (remove(FileName.c_str())!= 0)
		res.status=0,res.Reason="Can't remove record files!",perror("Error deleting file");
	res.Reason = 1;
	return res;
}
//
//

void RecordManager::SetCondition(bool& negative, vector<Condition>& con, char op ='a') {
	negative =(op =='o');
	if (negative) {
		for (size_t i = 0; i < con.size();++i) {
			switch (con[i].op)
			{
			case Lt:con[i].op = Ge; break;
			case Le:con[i].op = Gt; break;
			case Eq:con[i].op = Ne; break;
			case Gt:con[i].op = Le; break;
			case Ge:con[i].op = Lt; break;
			case Ne:con[i].op = Eq; break;
			}
		}
	}
	return;
}

void RecordManager::splitRow(const string& tuple, Row& row, Table& T) {
	int ofsInOneLine = 0;
	size_t end;
	for (auto attribute : T.attributes) {
		end = tuple.substr(ofsInOneLine, attribute.length).find_last_not_of("@");
		row.DataField.push_back(tuple.substr(ofsInOneLine, end+1));
		ofsInOneLine += attribute.length;
	}
}

void RecordManager::testifyOneRow(Row& oneRow, vector<Condition>& con, bool& negative, DATA& result,Table &T) {
	int satisfy = 0;
	for (auto singleCon : con) {
		bool flag = 0;
		switch (singleCon.op)
		{
		case Eq:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum]== singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum])== stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum])== stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;
		case Ne:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum]!= singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum])!= stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum])!= stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;
		case Gt:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum] > singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum]) > stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum]) > stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;
		case Ge:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum] >= singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum]) >= stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum]) >= stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;

		case Lt:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum] < singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum]) < stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum]) < stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;

		case Le:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum] <= singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum]) <= stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum]) <= stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;
		}
		if (!flag && !negative)
		{
			break;
		}
		//satisfy = 0;
	}

	if ((satisfy == con.size()) && !negative || (satisfy != con.size()) && negative)
	{
		result.ResultSet.push_back(oneRow);
	}

}

bool RecordManager::testifyOneRow(Row& oneRow, vector<Condition>& con, bool& negative,Table &T) {
	int satisfy = 0;
	for (auto singleCon : con) {
		bool flag = 0;
		switch (singleCon.op)
		{
		case Eq:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum]== singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum])== stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum])== stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;
		case Ne:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum]!= singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum])!= stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum])!= stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;
		case Gt:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum] > singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum]) > stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum]) > stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;
		case Ge:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum] >= singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum]) >= stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum]) >= stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;

		case Lt:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum] < singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum]) < stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum]) < stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;

		case Le:
			if (T.attributes[singleCon.columnNum].type == Char)
				if (oneRow.DataField[singleCon.columnNum] <= singleCon.value)
					satisfy++, flag = 1;
				else break;
			else if (T.attributes[singleCon.columnNum].type == Int)
				if (stoi(oneRow.DataField[singleCon.columnNum]) <= stoi(singleCon.value))
					satisfy++, flag = 1;
				else break;
			else {
				if (stof(oneRow.DataField[singleCon.columnNum]) <= stof(singleCon.value))
					satisfy++, flag = 1;
			}
			break;
		}
		if (!flag&&!negative)
		{
			break;
		}
		//satisfy = 0;
	}
	if ((satisfy == con.size()) && !negative || (satisfy != con.size()) && negative)
	{
		return 1;
	}
	return	0;
}

bool RecordManager::comp(Table& T, Condition& a, Condition& b) {
	if ((index.HasIndex(T.name, T.attributes[a.columnNum].name)) && a.op == Eq)
		return 1;
	else if ((index.HasIndex(T.name, T.attributes[b.columnNum].name)) && b.op == Eq)
		return 0;
	//case 1 index on sth + op==Eq  
	//only need to test a few Conditions
	if ((index.HasIndex(T.name, T.attributes[a.columnNum].name)) && a.op != Ne)
		return 1;
	else if ((index.HasIndex(T.name, T.attributes[b.columnNum].name)) && b.op != Ne)
		return 0;
	//case 2 index on sth + op !=Ne
	//only need to conduct all Condition testing on some of records;
	if ((index.HasIndex(T.name, T.attributes[a.columnNum].name)) && a.op == Ne)
		return 1;
	else if ((index.HasIndex(T.name, T.attributes[b.columnNum].name)) && b.op == Ne)
		return 0;
	//case 3 index on sth + op ==Ne
	//conduct testing for all records;

	return 1;
	//final test case—— only have to test all Conditions to all records;
}

//non increasing order
void RecordManager::MySort(Table& T, vector<Condition>& con) {
	for (size_t i = 0; i < con.size() - 1; ++i)
	{
		size_t max = i;
		for (size_t j = i + 1; j < con.size(); j++)
		{
			if (comp(T, con[j], con[max]))
				max = j;
		}
		swap(con[max], con[i]);
	}
}

//this one is for selecting
void RecordManager::ScanTable(Table& T, DATA& result, vector<Condition>& con, bool negative) {
	Page tempPage;
	Page& readIn = tempPage;
	readIn.pageType = RecordPage;
	//Page& readIn = buf.recordManagerGetBlankPage();
	if (con.empty()) {
		//readIn.pageType = RecordPage;
		readIn.tableName = T.name;
		for (readIn.ofs = 0; readIn.ofs<= T.blockNum; readIn.ofs++)
		{
			buf.readPage(readIn);
			//read in one block/page each time.
			string tempRow(readIn.pageData);
			// c string is horrible! transform to string.
			int recordNum = floor(tempRow.length() / T.totalLength);
			//calc num of records in this page.
			for (int offset = 0; offset < recordNum; offset++)
			{
				int positionInBlock = offset * T.totalLength;
				//ofs in a block, used to locate the entire record;
				string recordLine = tempRow.substr(positionInBlock, T.totalLength);
				if (recordLine[recordLine.length() - 1]=='1')
					continue;
				//saparate the record into a string.
				Row oneRow;
				splitRow(recordLine, oneRow, T);
				result.ResultSet.push_back(oneRow);
			}
		}
		return;
	}//empty Condition set , select all records;
	//at least one of attributes to be checked 
	//are equipped with indexes
	//take measures to simplify all process.

	//checked

	else if (index.HasIndex(T.name, T.attributes[con[0].columnNum].name))//(T.attributes[con[0].columnNum].isUnique|| T.attributes[con[0].columnNum].isPrimaryKey//))
	{
		//Page& readIn = buf.recordManagerGetBlankPage();
		readIn.tableName = T.name;
		string tuple;
		Row oneRow;
		int satisfy = 0;
		index.LoadIndex(T.name, T.attributes[con[0].columnNum].name, T.attributes[con[0].columnNum].type);
		vector<Pointer>blockOfs, inBlockOfs;
		Pointer singleBlock, singleBlockOfs;
		switch (con[0].op)
		{
		case Eq://fastest case
			switch (T.attributes[con[0].columnNum].type)
			{
			case Int:
				index.bptInt->Find(stoi(con[0].value), singleBlock);
				break;
			case Char:
				index.bptChar->Find(con[0].value, singleBlock);
				break;
			case Double:
				index.bptDouble->Find(atof(con[0].value.c_str()), singleBlock);
				break;
			}
			singleBlockOfs = singleBlock % PAGE_SIZE;
			singleBlock /= PAGE_SIZE;
			if (!negative) {//如果是合取操作，则将记录取出来 仅仅测试这一条记录即可
				readIn.ofs = singleBlock;
				buf.readPage(readIn);
				tuple = readIn.pageData;
				tuple = tuple.substr(singleBlockOfs, T.totalLength);
				if (tuple[tuple.length() - 1]=='1')
					break;
				splitRow(tuple, oneRow, T);
				testifyOneRow(oneRow, con, negative, result,T);
				oneRow.DataField.clear();
			}
			else
			{
				//当析取操作时，必须遍历所有的记录。
				for (size_t i = 0; i <= T.blockNum; ++i)
				{
					readIn.ofs = i;
					buf.readPage(readIn);
					string all(readIn.pageData);
					for (size_t j = 0; j < all.length(); j += T.totalLength)
					{
						string tuple = all.substr(j, T.totalLength);
						if (tuple[tuple.length() - 1]=='1')
							continue;
						splitRow(tuple, oneRow, T);
						testifyOneRow(oneRow, con, negative, result,T);
						oneRow.DataField.clear();
					}
				}
			}
			// speed 2nd case	
			// >>=< <=
			break;

		// speed 2nd case	
		// >>=< <=
		case Ge:case Gt:
			switch (T.attributes[con[0].columnNum].type)
			{
			case Int:
				index.bptInt->FindLarger(stoi(con[0].value), con[0].op == Ge, inBlockOfs);
				break;
			case Char:
				index.bptChar->FindLarger(con[0].value, con[0].op == Ge, inBlockOfs);
				break;
			case Double:
				index.bptDouble->FindLarger(atof(con[0].value.c_str()), con[0].op == Ge, inBlockOfs);
				break;
			}
			sort(inBlockOfs.begin(), inBlockOfs.end());
			for (size_t i = 0; i < inBlockOfs.size(); ++i) {
				blockOfs.push_back(inBlockOfs[i] / PAGE_SIZE);
				inBlockOfs[i] %= PAGE_SIZE;
			}

			for (size_t i = 0; i < inBlockOfs.size(); ++i)
			{
				readIn.ofs = blockOfs[i];
				buf.readPage(readIn);
				//已经读入对应的块了
				tuple = readIn.pageData;
				tuple = tuple.substr(inBlockOfs[i], T.totalLength);
				splitRow(tuple, oneRow, T);
				if (tuple[tuple.length() - 1]=='1')
					continue;
				testifyOneRow(oneRow, con, negative, result,T);
				oneRow.DataField.clear();
			}

			oneRow.DataField.clear();
			if (negative)//如果是or 则B+树底层左侧的全部都要push进来
			{
				inBlockOfs.clear(); blockOfs.clear();
				switch (T.attributes[con[0].columnNum].type)
				{
				case Int:
					index.bptInt->FindLess(stoi(con[0].value),!(con[0].op == Ge), inBlockOfs);
					break;
				case Char:
					index.bptChar->FindLess(con[0].value, !(con[0].op == Ge), inBlockOfs);
					break;
				case Double:
					index.bptDouble->FindLess(atof(con[0].value.c_str()),!(con[0].op == Ge), inBlockOfs);
					break;
				}
				sort(inBlockOfs.begin(), inBlockOfs.end());
				for (size_t i = 0; i < inBlockOfs.size(); ++i) {
					blockOfs.push_back(inBlockOfs[i] / PAGE_SIZE);
					inBlockOfs[i] %= PAGE_SIZE;
				}

				for (size_t i = 0; i < inBlockOfs.size(); ++i)
				{
					readIn.ofs = blockOfs[i];
					buf.readPage(readIn);
					//已经读入对应的块了
					tuple = readIn.pageData;
					tuple = tuple.substr(inBlockOfs[i], T.totalLength);
					splitRow(tuple, oneRow, T);
					if (tuple[tuple.length() - 1]=='1')
						continue;
					result.ResultSet.push_back(oneRow);
					oneRow.DataField.clear();
				}
			}
			break;
		case Lt: case Le:
			switch (T.attributes[con[0].columnNum].type)
			{
			case Int:
				index.bptInt->FindLess(stoi(con[0].value), con[0].op == Le, inBlockOfs);
				break;
			case Char:
				index.bptChar->FindLess(con[0].value, con[0].op == Le, inBlockOfs);
				break;
			case Double:
				index.bptDouble->FindLess(atof(con[0].value.c_str()), con[0].op == Le, inBlockOfs);
				break;
			}
			sort(inBlockOfs.begin(), inBlockOfs.end());
			for (size_t i = 0; i < inBlockOfs.size();++i) {
				blockOfs.push_back(inBlockOfs[i] / PAGE_SIZE);
				inBlockOfs[i] %= PAGE_SIZE;
			}

			for (size_t i = 0; i < inBlockOfs.size(); ++i)
			{
				readIn.ofs = blockOfs[i];
				buf.readPage(readIn);
				//已经读入对应的块了
				tuple = readIn.pageData;
				tuple = tuple.substr(inBlockOfs[i], T.totalLength);
				if (tuple[tuple.length() - 1]=='1')
					continue;
				splitRow(tuple, oneRow, T);
				testifyOneRow(oneRow, con, negative, result, T);
				oneRow.DataField.clear();
			}
			oneRow.DataField.clear();
			if (negative)
			{
				blockOfs.clear(); inBlockOfs.clear();
				index.LoadIndex(T.name, T.attributes[con[0].columnNum].name, T.attributes[con[0].columnNum].type);
				switch (T.attributes[con[0].columnNum].type)
				{
				case Int:
					index.bptInt->FindLarger(stoi(con[0].value),!(con[0].op == Le), inBlockOfs);
					break;
				case Char:
					index.bptChar->FindLarger(con[0].value, !(con[0].op == Le), inBlockOfs);
					break;
				case Double:
					index.bptDouble->FindLarger(atof(con[0].value.c_str()),!(con[0].op == Le), inBlockOfs);
					break;
				}
				sort(inBlockOfs.begin(), inBlockOfs.end());
				for (size_t i = 0; i < inBlockOfs.size(); ++i) {
					blockOfs.push_back(inBlockOfs[i] / PAGE_SIZE);
					inBlockOfs[i] %= PAGE_SIZE;
				}

				for (size_t i = 0; i < inBlockOfs.size(); ++i)
				{
					readIn.ofs = blockOfs[i];
					buf.readPage(readIn);
					//已经读入对应的块了
					tuple = readIn.pageData;
					tuple = tuple.substr(inBlockOfs[i], T.totalLength);
					splitRow(tuple, oneRow, T);
					if (tuple[tuple.length() - 1]=='1')
						continue;
					result.ResultSet.push_back(oneRow);
					oneRow.DataField.clear();
				}
			}

			break;
		default:
			//speed 3rd case
			//really need to scan the whole table, test every condition.
			while (buf.readPage(readIn)) {
				string thisPage = readIn.pageData;
				for (size_t i = 0; i < thisPage.length(); i += T.totalLength)
				{
					string tuple = thisPage.substr(i, T.totalLength);
					if (tuple[tuple.length() - 1]=='1')
						continue;
					splitRow(tuple, oneRow, T);
					testifyOneRow(oneRow, con, negative, result, T);
					oneRow.DataField.clear();
				}
				readIn.ofs++;
			}
		}
	}
	//checked

	//no index && have Conditions.
	else
	{
	//Page& readIn = buf.recordManagerGetBlankPage();
		//readIn.pageType = RecordPage;
		readIn.tableName = T.name;
		for (readIn.ofs= 0; readIn.ofs<= T.blockNum; readIn.ofs++)
		{
			buf.readPage(readIn);
			//read in one block/page each time.
			string tempRow(readIn.pageData);
			// c string is horrible! transform to string.
			int recordNum = tempRow.length() / T.totalLength;
			//calc num of records in this page.
			for (int offset = 0; offset < recordNum; offset++)
			{
				int positionInBlock = offset * T.totalLength;
				//ofs in a block, used to locate the entire record;
				string recordLine = tempRow.substr(positionInBlock, T.totalLength-1);
				// if marked as deleted
				if (recordLine[recordLine.length() - 1]=='1')
					continue;
				//saparate the record into a string.
				Row oneRow;
				splitRow(recordLine, oneRow, T);
				testifyOneRow(oneRow, con, negative, result, T);
				}
				//the code above separate one row ouT.
				
				//if the operation is to select
				//check all valid tuples and then return matching DATA
			}
		}

	//checked

		return;
}
	//the worst case, no indexes are avalible.

//this one is used to delete
void RecordManager::ScanTable(Table& T, RecordResult& deleteResult, vector<Condition>& con, bool negative) {
	Page tempPage;
	Page& readIn = tempPage;
	readIn.pageType = RecordPage;
	//readIn.pageType = RecordPage;
	readIn.tableName = T.name;
	if (con.empty()) {
		//将表删干净，索引也得删干净。因此先drop所有的，再重建。
		for (auto single : T.attributes) {
			if (index.HasIndex(T.name, single.name)&&!single.isPrimaryKey)
			{
				index.DropIndex(T.name, single.name);
			}
		}
		for (; readIn.ofs <= T.blockNum; readIn.ofs++)
		{
			buf.readPage(readIn);

			//read in one block/page each time.
			int recordNum = strlen(readIn.pageData) / T.totalLength;
			//calc num of records in this page.
			for (int offset = 0; offset < recordNum; offset++)
			{
				int positionInBlock = offset * T.totalLength;
				//ofs in a block, used to locate the entire record;
				readIn.pageData[positionInBlock + T.totalLength-1]='1';
				string tuple = readIn.pageData;
				tuple = tuple.substr(positionInBlock, T.totalLength);
				//Row onerow;
				//splitRow(tuple, onerow, T);
				//deleteIndex(T, onerow);
				
			}
			buf.writePage(readIn);
		}
		deleteResult.status = 1;
		index.CreateIndex(T);
		T.blockNum = 0;
		catalog.changeTable(T);
	}
	//empty Condition set , delete all records;


	//at least one of attributes to be checked 
	//are equipped with indexes
	//take measures to simplify all process.
	else if (index.HasIndex(T.name, T.attributes[con[0].columnNum].name))// (T.attributes[con[0].columnNum].isUnique || T.attributes[con[0].columnNum].isPrimaryKey)
	{

		readIn.tableName = T.name;
		string tuple;
		Row oneRow;
		int satisfy = 0;
		index.LoadIndex(T.name, T.attributes[con[0].columnNum].name, T.attributes[con[0].columnNum].type);
		vector<Pointer>blockOfs, inBlockOfs;
		Pointer singleBlock,singleBlockOfs;
		switch (con[0].op)
		{
		case Eq://contains the fastest case
			switch (T.attributes[con[0].columnNum].type)
			{
			case Int:
				index.bptInt->Find(stoi(con[0].value),singleBlock);
				break;
			case Char:
				index.bptChar->Find(con[0].value,  singleBlock);
				break;
			case Double:
				index.bptDouble->Find(atof(con[0].value.c_str()), singleBlock);
				break;
			}
			singleBlockOfs = singleBlock % PAGE_SIZE;
			singleBlock /= PAGE_SIZE;
			if (!negative) {//如果是合取操作，则将记录取出来 删掉即可
				readIn.ofs = singleBlock;
				buf.readPage(readIn);
				tuple = readIn.pageData;
				tuple = tuple.substr(singleBlockOfs, T.totalLength);
				splitRow(tuple, oneRow, T);
				if (testifyOneRow(oneRow, con, negative, T)) {
					readIn.pageData[singleBlockOfs + T.totalLength-1]='1';
					deleteIndex(T, oneRow);
				}
				buf.writePage(readIn);
			}
			else
			{
				//当析取操作时，必须遍历所有的记录。
				for (size_t i = 0; i <= T.blockNum; ++i)
				{
					readIn.ofs = i;
					buf.readPage(readIn);
					string all(readIn.pageData);
					for (size_t j = 0; j < all.length(); j+=T.totalLength)
					{
						splitRow(all.substr(j, T.totalLength), oneRow, T);
						if (!(readIn.ofs==singleBlock&&singleBlockOfs==j))
						{
							readIn.pageData[j + T.totalLength-1]='1';
							deleteIndex(T, oneRow);
							//因为后面会取反，此处已经有表达式=false 
						}
						else
						{
							if (testifyOneRow(oneRow, con, negative, T))
								readIn.pageData[j + T.totalLength-1]='1', deleteIndex(T, oneRow);
							
						}
						
						oneRow.DataField.clear();
					}
					buf.writePage(readIn);
				}
			}
		// speed 2nd case	
		// >>=< <=
			break;
		case Ge:case Gt:
			switch (T.attributes[con[0].columnNum].type)
			{
			case Int:
				index.bptInt->FindLarger(stoi(con[0].value), con[0].op == Ge, inBlockOfs);
				break;
			case Char:
				index.bptChar->FindLarger(con[0].value, con[0].op == Ge, inBlockOfs);
				break;
			case Double:
				index.bptDouble->FindLarger(atof(con[0].value.c_str()), con[0].op == Ge, inBlockOfs);
				break;
			}
			sort(inBlockOfs.begin(), inBlockOfs.end());
			for (size_t i = 0; i < inBlockOfs.size();++i) {
				blockOfs.push_back(inBlockOfs[i] / PAGE_SIZE);
				inBlockOfs[i] %= PAGE_SIZE;
			}

			for (size_t i = 0; i < inBlockOfs.size(); ++i)
			{
				readIn.ofs = blockOfs[i];
				buf.readPage(readIn);
				//已经读入对应的块了
				tuple = readIn.pageData;
				splitRow(tuple.substr(inBlockOfs[i], T.totalLength), oneRow, T);
				if (testifyOneRow(oneRow, con, negative, T))
				{
					//标记为已删除
					tuple[inBlockOfs[i] + T.totalLength]='1';
					deleteIndex(T, oneRow);
				}
				oneRow.DataField.clear();
				buf.writePage(readIn);
			}
			if (negative)//如果是or 则B+树底层左侧的全部都要push进来
			{
				blockOfs.clear(); inBlockOfs.clear();
				switch (T.attributes[con[0].columnNum].type)
				{
				case Int:
					index.bptInt->FindLess(stoi(con[0].value),!(con[0].op == Ge), inBlockOfs);
					break;
				case Char:
					index.bptChar->FindLess(con[0].value, !(con[0].op == Ge), inBlockOfs);
					break;
				case Double:
					index.bptDouble->FindLess(atof(con[0].value.c_str()),!(con[0].op == Ge), inBlockOfs);
					break;
				}
				sort(inBlockOfs.begin(), inBlockOfs.end());

				for (size_t i = 0; i < inBlockOfs.size(); ++i)
				{
					blockOfs. push_back(inBlockOfs[i] / PAGE_SIZE);
					inBlockOfs[i] %= PAGE_SIZE;
				}
				for (size_t i = 0; i < inBlockOfs.size(); ++i)
				{
					readIn.ofs = blockOfs[i];
					buf.readPage(readIn);
					//已经读入对应的块了
					tuple = readIn.pageData;
					splitRow(tuple.substr(inBlockOfs[i], T.totalLength), oneRow, T);
					if (testifyOneRow(oneRow, con, negative, T))
					{
						//标记为已删除
						tuple[inBlockOfs[i] + T.totalLength]='1';
						deleteIndex(T, oneRow);

					}
					oneRow.DataField.clear();
					buf.writePage(readIn);
				}
			}
			break;
		case Lt: case Le:
			switch (T.attributes[con[0].columnNum].type)
			{
			case Int:
				index.bptInt->FindLess(stoi(con[0].value), con[0].op == Le, inBlockOfs);
				break;
			case Char:
				index.bptChar->FindLess(con[0].value, con[0].op == Le, inBlockOfs);
				break;
			case Double:
				index.bptDouble->FindLess(atof(con[0].value.c_str()), con[0].op == Le, inBlockOfs);
				break;
			}
			sort(inBlockOfs.begin(), inBlockOfs.end());
			for (size_t i = 0; i < inBlockOfs.size();++i) {
				blockOfs.push_back(inBlockOfs[i] / PAGE_SIZE);
				inBlockOfs[i] %= PAGE_SIZE;
			}
			//index.printS();
			cout << endl;
			for (size_t i = 0; i < inBlockOfs.size(); ++i)
			{
				readIn.ofs = blockOfs[i];
				buf.readPage(readIn);
				//已经读入对应的块了
				tuple = readIn.pageData;
				splitRow(tuple.substr(inBlockOfs[i], T.totalLength), oneRow, T);
				if (testifyOneRow(oneRow, con, negative, T)) {
					tuple[inBlockOfs[i] + T.totalLength-1]='1';
					deleteIndex(T, oneRow);
					//index.printS();
				}
				buf.writePage(readIn);
				oneRow.DataField.clear();
			}
			inBlockOfs.clear(); blockOfs.clear();
			if (negative)
			{
				//右侧全push
				switch (T.attributes[con[0].columnNum].type)
				{
				case Int:
					index.bptInt->FindLarger(stoi(con[0].value),!(con[0].op == Le), inBlockOfs);
					break;
				case Char:
					index.bptChar->FindLarger(con[0].value, !(con[0].op == Le), inBlockOfs);
					break;
				case Double:
					index.bptDouble->FindLarger(atof(con[0].value.c_str()),!(con[0].op == Le), inBlockOfs);
					break;
				}
				sort(inBlockOfs.begin(), inBlockOfs.end());
				for (size_t i = 0; i < inBlockOfs.size(); ++i) {
					blockOfs.push_back(inBlockOfs[i] / PAGE_SIZE);
					inBlockOfs[i] %= PAGE_SIZE;
				}
				for (size_t i = 0; i < inBlockOfs.size(); ++i)
				{
					//即使是同一个块也无妨，buffer manager会自己处理
					readIn.ofs = blockOfs[i];
					buf.readPage(readIn);
					//已经读入对应的块了
					tuple = readIn.pageData;
					splitRow(tuple.substr(inBlockOfs[i], T.totalLength), oneRow, T);
					if (testifyOneRow(oneRow, con, negative, T))
					{
						//标记为已删除
						tuple[inBlockOfs[i] + T.totalLength-1]='1';
						deleteIndex(T, oneRow);

					}
					oneRow.DataField.clear();
					buf.writePage(readIn);
				}
			}

			break;
		}
	}
	//no index && have Conditions.
	else
	{
		for (readIn.ofs= 0; readIn.ofs <= T.blockNum; readIn.ofs++)
		{
			buf.readPage(readIn);
			string tempRow(readIn.pageData);
			// c string is horrible! transform to string.
			int recordNum = tempRow.length() / T.totalLength;
			//calc num of records in this page.
			for (int offset = 0; offset < recordNum; offset++)
			{
				int positionInBlock = offset * T.totalLength;
				//ofs in a block, used to locate the entire record;
				string recordLine = tempRow.substr(positionInBlock, T.totalLength);
				Row oneRow;
				splitRow(recordLine, oneRow, T);
				//the code above separate one row ouT.
				if (testifyOneRow(oneRow, con, negative, T))
				{
					readIn.pageData[T.totalLength + positionInBlock-1]='1';
					deleteIndex(T, oneRow);
				}
			}
			buf.writePage(readIn);
		}
		deleteResult.status = 1;
		
		
	}
	//the worst case, no indexes are avalible.
	char* last1 = nullptr;
	for (size_t i = PAGE_SIZE - 1; i + 1; i -= T.totalLength)
	{
		if (readIn.pageData[i]=='@')
		{
			i += T.totalLength - 1;
			continue;
		}
		if (readIn.pageData[i]=='1')
			continue;
		last1 = readIn.pageData + i;
		break;
	}
//	if ((int)last1 <= 0&&T.blockNum>0)
	if (last1 <= 0&&T.blockNum>0)
		T.blockNum--;
	return;
}

void RecordManager::deleteIndex(Table& T, Row& row) {
	for (size_t i = 0; i < row.DataField.size(); ++i)
	{
		if (index.HasIndex(T.name, T.attributes[i].name))
		{
			if (index.bptIntName==T.attributes[i].name){
				index.Delete(row.DataField[i], Int);
			}
			else if (index.bptDoubleName == T.attributes[i].name) {
				index.Delete(row.DataField[i], Double);

			}
			else if (index.bptCharName == T.attributes[i].name) {
				index.Delete(row.DataField[i], Char);
			}
			else
			{
				index.LoadIndex(T.name, T.attributes[i].name, T.attributes[i].type);
				index.Delete(row.DataField[i], T.attributes[i].type);
			}
		}
	}
}

void RecordManager::insertIndex(Table& T, Row& row, const Pointer& ptr) {
	for (size_t i = 0; i < row.DataField.size(); ++i)
	{
		if (index.HasIndex(T.name, T.attributes[i].name))
		{
			if (index.bptIntName == T.attributes[i].name) {
				index.InsertItem(row.DataField[i],ptr, Int);
			}
			else if (index.bptDoubleName == T.attributes[i].name) {
				index.InsertItem(row.DataField[i], ptr, Double);

			}
			else if (index.bptCharName == T.attributes[i].name) {
				index.InsertItem(row.DataField[i], ptr, Char);
			}
			else
			{
				index.LoadIndex(T.name, T.attributes[i].name, T.attributes[i].type);
				index.InsertItem(row.DataField[i], ptr, T.attributes[i].type);
			}
		}
	}
}

void RecordManager::checkDuplicate(Table&T, vector<string>& value, RecordResult& res) {
	int i = 0;
	for (auto attribute : T.attributes) {
		if (index.HasIndex(T.name, attribute.name))
		{
			index.LoadIndex(T.name, attribute.name, attribute.type);
			if (index.Find(value[++i], attribute.type))
			{
				res.status = 0;
					res.Reason ="Duplicate value for  \'" + T.name + "." + attribute.name + "\'"+"of value \'"+value[i-1]+"\' !!";
					return;
			}
		}
	}
}

void RecordManager::createIndex(Table&T, Index&id) {
	Page readIn;
	readIn.tableName = T.name;
	for (readIn.ofs = 0; readIn.ofs <= T.blockNum; readIn.ofs++)
	{
		buf.readPage(readIn);
		//read in one block/page each time.
		string tempRow(readIn.pageData);
		// c string is horrible! transform to string.
		int recordNum = floor(tempRow.length() / T.totalLength);
		//calc num of records in this page.
		for (int offset = 0; offset < recordNum; offset++)
		{
			int positionInBlock = offset * T.totalLength;
			//ofs in a block, used to locate the entire record;
			string recordLine = tempRow.substr(positionInBlock, T.totalLength);
			if (recordLine[recordLine.length() - 1]=='1')
				continue;
			//saparate the record into a string.
			Row oneRow;
			splitRow(recordLine, oneRow, T);
			insertIndex(T, oneRow, readIn.ofs * PAGE_SIZE + positionInBlock,id.attribute_name);
		}
	}
	//index.SaveIndex();
}

void RecordManager::insertIndex(Table& T, Row& row, const Pointer& ptr,string&attri) {
	for (size_t i = 0; i < row.DataField.size(); ++i)
	{
		if (T.attributes[i].name != attri)
			continue;
		if (index.HasIndex(T.name, T.attributes[i].name))
		{
			if (index.bptIntName == T.attributes[i].name) {
				index.InsertItem(row.DataField[i], ptr, Int);
			}
			else if (index.bptDoubleName == T.attributes[i].name) {
				index.InsertItem(row.DataField[i], ptr, Double);

			}
			else if (index.bptCharName == T.attributes[i].name) {
				index.InsertItem(row.DataField[i], ptr, Char);
			}
			else
			{
				index.LoadIndex(T.name, T.attributes[i].name, T.attributes[i].type);
				index.InsertItem(row.DataField[i], ptr, T.attributes[i].type);
			}
		}
	}
}
