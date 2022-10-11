//ʵ��һ��MyException��
#pragma once
#include<iostream>
#include<string>
using namespace std;
//MyException ����
class MyException
{
public:
	MyException(string _exception) :message(_exception)
	{

	}
	virtual void printError()const                                        //�麯��ʵ�ֶ�̬
	{
		cout << "MyException��" << message << endl;
	}

protected:
	string message;
};

//�׳��쳣 errChoiceException ��,�����û���û��ѡ������SQL��û��ѡ���Ƴ�
class errChoiceException :public MyException
{
public:
	errChoiceException(string _exception) :MyException(_exception) {}       //ʹ�ó�ʼ���б�������ʼ��
	const virtual void printError()
	{
		cout << "errChoiceException��" << message << endl;
	}
};

//�׳��쳣 errSyntaxException ��,�����û�û��������ȷ��SQL�﷨����ȱ��from�ȣ�
class errSyntaxException :public MyException
{
public:
	errSyntaxException(string _exception) :MyException(_exception) {}       //ʹ�ó�ʼ���б�������ʼ��
	const virtual void printError()
	{
		cout << "errSyntaxException��" << message << endl;
	}
};

//�׳��쳣 errSQLSyntax ��,�����û������SQL�еı�����ֶβ�����
class errExistException :public MyException
{
public:
	errExistException(string _exception) :MyException(_exception) {}       //ʹ�ó�ʼ���б�������ʼ��
	const virtual void printError()
	{
		cout << "errExistException��" << message << endl;
	}
}; 

class errTypeException :public MyException
{
public:
	errTypeException(string _exception) :MyException(_exception) {}       //ʹ�ó�ʼ���б�������ʼ��
	const virtual void printError()
	{
		cout << "errTypeException��" << message << endl;
	}
};

class errCompareException:public MyException
{
public:
	errCompareException(string _exception) :MyException(_exception) {}       //ʹ�ó�ʼ���б�������ʼ��
	const virtual void printError()
	{
		cout << "errCompareException��" << message << endl;
	}
};

class errRangeException :public MyException
{
public:
	errRangeException(string _exception) :MyException(_exception) {}       //ʹ�ó�ʼ���б�������ʼ��
	const virtual void printError()
	{
		cout << "errRangeException��" << message << endl;
	}
};