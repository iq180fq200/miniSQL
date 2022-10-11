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
	//bool flag = true;			//�ж��Ƿ�quit
	void GetQueryFromConsole();				//���ڶ�ȡָ��
	OperationType Choice();				//���ݶ����ָ���Ӧ��Ӧ�ӿ�
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
	//int NextPosition(int i);			//������һ�����ʵ�ĩβλ��
	//int NextPosition(int i, string str);//����str�ַ���������һ�����ʵ�ĩβλ��		//�����������Կո��;��'\t'��Ϊ�жϱ�־��
	string SQL;
};

//class Exception :std::exception //�쳣�׳�
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
