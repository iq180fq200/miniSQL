#pragma once
#include"global.h"
#include"MyException.h"


class Interpreter
{
private:
	void EraseBrowser(string& SQL) {
		for (int i = 0; i < SQL.size(); i++) {
			if (SQL[i] == '(' || SQL[i] == ',' || SQL[i] == ')' || SQL[i] == ' ' || SQL[i] == ';')
				SQL[i] = ' ';
		}
		SQL = SQL + ";";
	};

	enum DataType ParseToDatatype(string _type) {
		if (_type == "char")
			return Char;
		else if (_type == "float")
			return Double;
		else if (_type == "int")
			return Int;
		else if (_type == "void")
			return Void;
		else
			throw errTypeException("error type: " + _type);
	}

	int ConvertToDigit(string num) {
		int nk = num.size();
		int digit = 0;
		for (int j = 0; j < nk; j++) {
			digit += (num[j]-'0') * pow(10, nk - 1 - j);
		}
		return digit;
	}
public:
	//bool flag = true;			//判断是否quit
	void GetQueryFromConsole();				//用于读取指令
	OperationType Choice();				//根据读入的指令对应相应接口
	void Exec(OperationType operationType);
	void Create_Table_Exec();
	void Create_Index_Exec();
	void Drop_Table_Exec();
	void Drop_Index_Exec();
	void Select_Exec();
	void Select_Exec_1();
	void Insert_Exec();
	void Delete_Exec();
	void Quit_Exec();
	void Execfile();
	//int NextPosition(int i);			//返回下一个单词的末尾位置
	//int NextPosition(int i, string str);//返回str字符串里面下一个单词的末尾位置		//这两个都是以空格和;或'\t'作为判断标志的
	string SQL;
};

//class Exception :std::exception //异常抛出
//{
//private:
//	std::string error;
//public:
//	Exception(std::string s) :error(s) {}
//	std::string what() 
//	{
//		return error;
//	}
//};
