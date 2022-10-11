//实现一个MyException类
#pragma once
#include<iostream>
#include<string>
using namespace std;
//MyException 父类
class MyException
{
public:
	MyException(string _exception) :message(_exception)
	{

	}
	virtual void printError()const                                        //虚函数实现多态
	{
		cout << "MyException：" << message << endl;
	}

protected:
	string message;
};

//抛出异常 errChoiceException 类,代表用户既没有选择输入SQL又没有选择推出
class errChoiceException :public MyException
{
public:
	errChoiceException(string _exception) :MyException(_exception) {}       //使用初始化列表对子类初始化
	const virtual void printError()
	{
		cout << "errChoiceException：" << message << endl;
	}
};

//抛出异常 errSyntaxException 类,代表用户没有输入正确的SQL语法（如缺少from等）
class errSyntaxException :public MyException
{
public:
	errSyntaxException(string _exception) :MyException(_exception) {}       //使用初始化列表对子类初始化
	const virtual void printError()
	{
		cout << "errSyntaxException：" << message << endl;
	}
};

//抛出异常 errSQLSyntax 类,代表用户输入的SQL中的表或者字段不存在
class errExistException :public MyException
{
public:
	errExistException(string _exception) :MyException(_exception) {}       //使用初始化列表对子类初始化
	const virtual void printError()
	{
		cout << "errExistException：" << message << endl;
	}
}; 

class errTypeException :public MyException
{
public:
	errTypeException(string _exception) :MyException(_exception) {}       //使用初始化列表对子类初始化
	const virtual void printError()
	{
		cout << "errTypeException：" << message << endl;
	}
};

class errCompareException:public MyException
{
public:
	errCompareException(string _exception) :MyException(_exception) {}       //使用初始化列表对子类初始化
	const virtual void printError()
	{
		cout << "errCompareException：" << message << endl;
	}
};

class errRangeException :public MyException
{
public:
	errRangeException(string _exception) :MyException(_exception) {}       //使用初始化列表对子类初始化
	const virtual void printError()
	{
		cout << "errRangeException：" << message << endl;
	}
};