#include"Catalog.h"
#include"global.h"
#include"IndexManager.h"
#include"RecordManager.h"
#include"Interpreter.h"
#include"BufferManager.h"
#include"Page.h"
#include<exception>
#include"MyException.h"
#include <time.h>
#include"API.h"
extern Interpreter myinterpreter;
extern RecordManager record;
extern IndexManager index;
extern CatalogManager catalog;
extern BufferManager buf;

//extern long long  start, endtime;
//extern bool infile;

//整个都进来，到SQL里,g
void Interpreter::GetQueryFromConsole()
{
	string s = "";
	string t;
	cin >> s;
	while (true) {
		cin >> t;
		int pos = t.size() - 1;
		if (t == ";" || t[pos] == ';') {
			s = s + ' ' + t;
			break;
		}
		s = s + ' ' + t;
		t = "";
	}
	SQL = s;
	return;

}

//g
OperationType Interpreter::Choice()
{
	istringstream str(SQL);
	string firstWord = "";
	string secondWord = "";
	string otherWord = "";
	str >> firstWord;
	str >> secondWord;
	if (firstWord == "select") {
		return (OperationType)MySelect;
	}
	else if (firstWord == "insert")
		return (OperationType)MyInsert;
	else if (firstWord == "create") {
		if (secondWord == "index")
			return (OperationType)MyCreate_index;
		return (OperationType)MyCreate;
	}
	else if (firstWord == "drop")
		return (OperationType)MyDrop;
	else if (firstWord == "quit")
		return (OperationType)MyQuit;
	else if (firstWord == "execfile")
		return (OperationType)MyExecfile;
	else
		throw  errSyntaxException("no such operation");
}
//g
void Interpreter::Exec(OperationType operationType)
{
	switch (operationType) {
	case MyCreate:Create_Table_Exec(); break;
	case MyDrop:Drop_Table_Exec(); break;
	case MyInsert:Insert_Exec(); break;
	case MyDelete:Delete_Exec(); break;
	case MySelect:Select_Exec(); break;
	case MyCreate_index:Create_Index_Exec(); break;
	case MyDrop_index:Drop_Index_Exec(); break;
	case MyExecfile:Execfile(); break;
	case MyQuit:exit(0);return;
	default:throw errSyntaxException("no such operation");
	}
}

//g
void Interpreter::Create_Table_Exec()
{
	Table tab;
	//get name
	string currentWord;
	string primaryKey = "";
	//去除不必要符号如括号,并分词
	EraseBrowser(SQL);
	//1.生成对应的SQL类，注意必须有主键
	istringstream str(SQL);
	str >> currentWord;
	str >> currentWord;
	if (currentWord != "table")
		throw errSyntaxException("must create with beginning of 'create table'");
	str >> currentWord;
	tab.name = currentWord;
	//获得属性与主键
	//开始输入各属性名 ID int unique name char 10
	//vector<DataTypeUnit> dataTypes;
	string name;
	string strtype;//得到类型过程中的临时变量
	//string primaryKey = "";
	enum DataType datatype;//构建DataTypeUnit过程中的临时变量
	//vector<bool> unique;
	uint num;//构建DataTypeUnit过程中的临时变量
	str >> currentWord;
	bool uniqueFlag;
	while (currentWord != ";") {
		if (currentWord == "primary") {
			str >> currentWord;
			if (currentWord != "key")
				throw errSyntaxException("‘primary’ can not be a attribute name!");
			if (primaryKey != "")
				throw errSyntaxException("can not have more than one primary key");
			str >> currentWord;
			primaryKey = currentWord;
			str >> currentWord;
			continue;
		}
		name=currentWord;
		str >> currentWord;
		strtype = currentWord;
		//转为一个datatype
		datatype = ParseToDatatype(strtype);
		//如果是char
		str >> currentWord;
		if (datatype == Char) {
			if (currentWord[0] > '0'&&currentWord[0] <= '9') {
				num = ConvertToDigit(currentWord);
				str >> currentWord;
			}
			else
				num = 1;
		}
		//int
		else if (datatype == Int) {
			num = 11;//不是char,不存在元素个数问题
		}
		//float
		else
			num = 10;
	//	judge unique
		if (currentWord == "unique") {
			uniqueFlag=true;
			str >> currentWord;
	     } 
		else
			uniqueFlag = false;
	//构造属性
		tab.attributes.push_back(Attribute(name, datatype, num, false, uniqueFlag));
		tab.totalLength += num;
	}
	tab.attriNum = tab.attributes.size();
	//构造primary key
	for (int i = 0; i < tab.attriNum; i++) {
		if (tab.attributes[i].name == primaryKey)
			tab.attributes[i].isUnique=tab.attributes[i].isPrimaryKey = true;
	}
	APIcreateTable(tab);
	//catalog.createTable(tab);
    cout << "Create table success" << endl;
    //catalog.reload();
}


//建立单值索引
void Interpreter::Create_Index_Exec()	
{
	string attributeName;
	string indexName;
	string currentWord;
	string tableName;
	//去除不必要符号如括号并分词
	EraseBrowser(SQL);
	//1.生成对应的SQL类，注意除语法外必须有主键
	istringstream str(SQL);
	str >> currentWord;
	str >> currentWord;
	str >> currentWord;
	indexName = currentWord;
	str >> currentWord;
	if (currentWord != "on")
		throw errSyntaxException("create index must followd by on");
	str >> currentWord;
	tableName = currentWord;
	str >> currentWord;
	if (currentWord == ";")
		throw errSyntaxException("can not create index without choosing any attribute");
	string AttributeName;
	attributeName = currentWord;
	Index id;Table tab;
	id.attribute_name = AttributeName;
	id.index_name = indexName;
	id.table_name = tableName;
	CatalogManager cat;
	catalog.WriteToBuffer();
	tab = catalog.getTable_info(tableName);
	id.column = catalog.getAttr_no(tab, AttributeName);
	if (id.column < 0) {
		errExistException("the attribute doesn't exist");
	}
	id.columnLength = tab.attributes[id.column].length;
	APIcreateIndex(tab, id);
	cout << "Succeed to create index " +id.index_name+" ."<< endl;
}


void Interpreter::Drop_Table_Exec()
{
	//1.生成对应的SQL类
	//去除不必要符号如括号并分词
	EraseBrowser(SQL);
	//1.生成对应的SQL类
	istringstream str(SQL);
	string currentWord;
	string tableName;
	str >> currentWord;
	str >> currentWord;
	if (currentWord != "table")
		throw errSyntaxException("must create with beginning of 'create table'");
	str >> currentWord;
	tableName = currentWord;
	Table T = catalog.getTable_info(tableName);
	if (T.name=="")
	{
		throw MyException("Invalid Query of Drop Table!");
	}
	else
	{
		APIDropTable(tableName);
	}

}
//
void Interpreter::Drop_Index_Exec()
{
	//去除不必要符号如括号并分词
	EraseBrowser(SQL);
	//1.生成对应的SQL类，注意必须有主键
	istringstream str(SQL);
	string currentWord;
	string tableName;
	str >> currentWord;
	str >> currentWord;
	str >> currentWord;
	string indexName = currentWord;
	str >> currentWord;
	if (currentWord != ";")
		throw errSyntaxException(SQL);
	APIDropIndex(indexName);
	std::cout << "Succeed to drop the index." << endl;
}

void Interpreter::Select_Exec_1() {
		Table tab; string table_name;
		table_name = SQL.substr(SQL.find("from") + 4);
		table_name = table_name.substr(table_name.find_first_not_of(" \t"));
		table_name = table_name.substr(0, table_name.find_last_not_of(" \t,") + 1);
		table_name = table_name.substr(0, table_name.find_first_of(", \t"));
		table_name = table_name.substr(0, table_name.find_last_not_of(" \t;") + 1);
		Condition con;
		Table T = catalog.getTable_info(table_name);
		vector <Condition> cons;
		size_t conditionStart = SQL.find("where");
		size_t subStart = conditionStart;
		string getAttri = SQL;
		vector<string> showAttri;
		string oper;
		//拿到需要展示的所有属性
		getAttri = getAttri.substr(getAttri.find("select") + 6);
		getAttri = getAttri.substr(0, getAttri.find("from"));

		if (getAttri.find_first_not_of(" \t") == string::npos) {
			cout << "Enter any attribute. Invalid sql syntax." << endl;
		}
		else
		{
			if ((getAttri.find("*") != string::npos)) {
				for (auto iter : T.attributes) {
					showAttri.push_back(iter.name);
				}
			}
			else
			{


				do {
					getAttri = getAttri.substr(getAttri.find_first_of(" \t,"));
					getAttri = getAttri.substr(getAttri.find_first_not_of(" \t,"));
					showAttri.push_back(getAttri.substr(0, getAttri.find_first_of(" \t,")));
				} while (getAttri.find(",") != string::npos);
			}
			if (conditionStart != string::npos)
			{
				conditionStart += 5;
				SQL = SQL.substr(conditionStart);
				char operation;
				if (SQL.find("or") != string::npos) {
					operation = 'o';
				}
				else
				{
					operation = 'a';
				}
				oper = operation == 'a' ? "and" : "or";
				string subCon = SQL = SQL.substr(SQL.find_first_not_of(" \t"));//where 后第一个非空格字符
				subCon = SQL = SQL.substr(0, SQL.find_last_of(";") + 1);
				while (SQL != ";")
				{
					string attri = subCon.substr(0, subCon.find_first_of(" "));
					con.columnNum = catalog.getAttr_no(T, attri);
					if (con.columnNum == -1) {
						cout << "Wroing data field name of \' " << attri << " \' of table " << T.name << endl;
						return;
						//提前结束 出错了  不存在的字段。
					}
					//attri += " ";
					subCon = subCon.substr(subCon.find_first_not_of(attri));// 舍去attribute name
					string op = subCon = subCon.substr(subCon.find_first_not_of(" \t"));
					op = subCon.substr(0, subCon.find_first_of(" \t"));
					if (op == ">=")
						con.op = Ge;
					else if (op == "<=")
						con.op = Le;
					else if (op == ">")
						con.op = Gt;
					else if (op == "<")
						con.op = Lt;
					else if (op == "!=" || op == "<>")
						con.op = Ne;
					else
						con.op = Eq;
					subCon = subCon.substr(subCon.find_first_not_of(op));
					if (T.attributes[con.columnNum].type == Char) {
						subCon = subCon.substr(subCon.find_first_of("\'\""));//
						con.value = subCon.substr(1).substr(0, subCon.substr(1).find_first_of("\"\'"));//
						SQL = subCon = subCon.substr(subCon.find_first_not_of(con.value + "\'\" " + oper));
						cons.push_back(con);
					}
					else {
						subCon = subCon.substr(subCon.find_first_not_of(" \t"));
						con.value = subCon.substr(0, subCon.find_first_of(" \t;"));
						SQL = subCon = subCon.substr(subCon.find_first_not_of(con.value + " \t" + oper));
						cons.push_back(con);
					}


				}
			}
		}
		DATA res = record.select(T, cons, oper[0]);
		cout << endl;
		vector<int>outCol;
		for (size_t i = 0, j = 0; i < T.attributes.size(); i++)
		{
			if (j < showAttri.size() && (T.attributes[i].name == showAttri[j]))
				outCol.push_back(i), j++;
		}
		for (size_t i = 0; i < res.ResultSet.size(); i++)
		{
			for (auto iter : outCol)
				cout << res.ResultSet[i].DataField[iter] << "    ";

			cout << endl;
		}
		if (!res.ResultSet.size())
			cout << "No matching records." << endl;
}




void Interpreter::Select_Exec() {
	EraseBrowser(SQL);
	istringstream str(SQL);
	string currentWord;
	string tableName;
	vector <Condition> cons;//条件
	str >> currentWord;//select
	vector<string> colums;//属性
	str >> currentWord;
	Table tab;
	if (currentWord != "*") {
		while (currentWord != "from"&&currentWord != ";") {
			colums.push_back(currentWord);
			str >> currentWord;
		}
		if (currentWord == ";")
			throw errSyntaxException(SQL + " must select from a exsiting table!");
		str >> currentWord;
		tableName = currentWord;
		tab = catalog.getTable_info(tableName);
	}
	else {
		str >> currentWord;
		str >> currentWord;
		tableName = currentWord;
		tab = catalog.getTable_info(tableName);
		for (int i = 0; i < tab.attriNum; i++) {
			colums.push_back(tab.attributes[i].name);
		}
	}
	str >> currentWord;
	if (currentWord == "where") {
		Select_Exec_1();
		return;
		str >> currentWord;
		Condition con;
		while (currentWord != ";") {
			string attributeName = currentWord;
			str >> currentWord;
			string op = currentWord;
			str >> currentWord;
			string strvalue = currentWord;
			str >> currentWord;
			if (currentWord != "and"&&currentWord != ";")
				throw errSyntaxException("syntax error");
			str >> currentWord;
			con.columnNum = catalog.getAttr_no(tab, attributeName);
			if (op == ">=")
				con.op = Ge;
			else if (op == "<=")
				con.op = Le;
			else if (op == ">")
				con.op = Gt;
			else if (op == "<")
				con.op = Lt;
			else if (op == "!=" || op == "<>")
				con.op = Ne;
			else
				con.op = Eq;
			con.value = strvalue;
			cons.push_back(con);
		}
	}
	else if (currentWord != ";")
		throw errSyntaxException("syntax error");
	DATA res = record.select(tab, cons, 'a');//直接调用record
	vector<int>outCol;
	for (size_t i = 0, j = 0; i < tab.attributes.size(); i++)
	{
		if (j < colums.size() && (tab.attributes[i].name == colums[j]))
			outCol.push_back(i), j++;
	}
	for (size_t i = 0; i < res.ResultSet.size(); i++)
	{
		for (auto iter : outCol)
			std::cout << res.ResultSet[i].DataField[iter] << "    ";

		std::cout << endl;
	}
	if (!res.ResultSet.size())
		std::cout << "No matching records." << endl;
}




void Interpreter::Insert_Exec() {
EraseBrowser(SQL);
//1.生成对应的SQL类
istringstream str(SQL);
string currentWord;
string tableName;
str >> currentWord;
str >> currentWord;
if (currentWord != "into")
throw errSyntaxException("insert must followed by into");
str >> currentWord;//3
tableName = currentWord;
Table tab= catalog.getTable_info(tableName);
str >> currentWord;
if (currentWord != "values")
throw errSyntaxException("insert into table must followed by values()");
str >> currentWord;
vector<string> informations;
string information;
for (size_t i = 0; i < tab.attributes.size(); i++)
{
	if (currentWord == ";")
		throw errSyntaxException("error attribute numbers");
	switch (tab.attributes[i].type)
	{

	case Int:
		if (currentWord.length() > INTLENGTH) {
			throw MyException("too long int");
		}
		information = currentWord;
		informations.push_back(information);
		str >> currentWord;
		break;

	case Double:
		if (currentWord.length() > FLOATLEN) {
			throw MyException("too long float");
		}
		information = currentWord;
		informations.push_back(information);
		str >> currentWord;
		break;
	case Char:
		if (currentWord.length() > tab.attributes[i].length) {
			throw MyException("too long char");
		}
		information = currentWord;
		informations.push_back(information);
		str >> currentWord;
		break;
	}
}
const RecordResult& res = record.insert(tab, informations);
cout << "Successful insertion" << endl;
}

void Interpreter::Delete_Exec()
{
	int i, j, length;
	string tableName, attribute_name;
	i = 12;
	//j=next(i)
	j = i + 1;
	while (j <= SQL.length() && SQL[j] != ' '&&SQL[j] != ';')
		j++;
	length = j - i;
	tableName = SQL.substr(i, length);
	Table T = catalog.getTable_info(tableName);
	if (SQL[j] == ';')			//例如：delete from student;
	{
		//删除表的接口
		vector<Condition> con;
		RecordResult res = record.deleteR(T, con, 'a');
		if (res.status) {
			cout << "Successful deletion." << endl;
		}
		else
		{
			cout << "Failed to execute the command. " << res.Reason << endl;
		}
	}
	else
	{
		Condition con;
		Table tab;
		vector <Condition> cons;
		string tableName(SQL.substr(SQL.find("from") + 4));
		tableName = tableName.substr(tableName.find_first_not_of(" \t"));
		tableName = tableName.substr(0, tableName.find_first_of(" \t"));
		tab = catalog.getTable_info(tableName);
		size_t conditionStart = SQL.find("where") + 5;
		size_t subStart = conditionStart;
		SQL = SQL.substr(conditionStart);
		char operation;
		if (SQL.find("or") != string::npos) {
			operation = 'o';
		}
		else
		{
			operation = 'a';
		}

		string oper = operation == 'a' ? "and" : "or";
		string subCon = SQL = SQL.substr(SQL.find_first_not_of(" \t"));//where 后第一个非空格字符
		subCon = SQL = SQL.substr(0, SQL.find_last_of(";") + 1);
		while (SQL != ";")
		{
			string attri = subCon.substr(0, subCon.find_first_of(" \t"));
			con.columnNum = catalog.getAttr_no(T, attri);
			if (con.columnNum == -1) {
				//提前结束 出错了  不存在的字段。
			}
			attri += " ";
			subCon = subCon.substr(subCon.find_first_not_of(attri));// 舍去attribute name
			string op = subCon.substr(0, subCon.find_first_of(" \t"));
			if (op == ">=")
				con.op = Ge;
			else if (op == "<=")
				con.op = Le;
			else if (op == ">")
				con.op = Gt;
			else if (op == "<")
				con.op = Lt;
			else if (op == "!=" || op == "<>")
				con.op = Ne;
			else
				con.op = Eq;
			subCon = subCon.substr(subCon.find_first_not_of(op));
			if (T.attributes[con.columnNum].type == Char) {
				subCon = subCon.substr(subCon.find_first_of("\'\""));//
				con.value = subCon.substr(1).substr(0, subCon.substr(1).find_first_of("\"\'"));//
				SQL = subCon = subCon.substr(subCon.find_first_not_of(con.value + "\'\" " + oper));
				cons.push_back(con);
			}
			else {
				subCon = subCon.substr(subCon.find_first_not_of(" \t"));
				con.value = subCon.substr(0, subCon.find_first_of(" \t;"));
				SQL = subCon = subCon.substr(subCon.find_first_not_of(con.value + " \t" + oper));
				cons.push_back(con);
			}

		}
		RecordResult res = record.deleteR(tab, cons, oper[0]);
		if (res.status) {
			cout << "Successful deletion." << endl;
		}
		else
		{
			cout << "Failed to execute the command. " << res.Reason << endl;
		}
	}
}
	

void Interpreter::Quit_Exec()
{
	cout << "Done ~ " << endl;
	return;
}

void Interpreter::Execfile()
{


	string file_name = SQL.substr(9, SQL.length() - 10);
	ifstream file(file_name);
	if (file.fail())
	{
		//throw Exception("The File Does Not Exist");
		cout << "The File Does Not Exist" << endl;
		return;
	}
	SQL.clear();
	while (!file.eof())
	{
		do
		{
			string s;
			getline(file, s);
			SQL += s;

		} while (SQL.find(";") == string::npos);
		Choice();
		SQL.clear();
	}

}
