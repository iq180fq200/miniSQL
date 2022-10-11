/***
 * 本代码实现了 Data 类，该类的对象为任意基本类型的组合。
 * 该类私有变量为 n:变量数量，type:每个变量的类型，data:存储变量们，endpos:每个变量的终止位置。 
 * 例如：{char,int,double,int[3]}组成的组合中：
 *     n=4，type[]={Char,Int,Double,Int}，
 *   字节编号：0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
 *     data[]={c, i, i, i, i, d, d, d, d, d, d, d, d, i, i, i, i, i, i, i, i, i, i, i, i}，
 *     endpos[]={1,5,13,25}。 
 *
 * DataTypeUnit 类用于给 Data 初始化，DataTypeUnit对象中包括一个表示类型的变量 type，和一个表示数量的变量 num。
 * 可以用 DataTypeUnit 的数组或向量对 Data 对象进行构造。
 * GetElementValue(sn) 可以返回对象中第 sn 个数据的类型和值。sn 从 0 开始编号。
 *    两个重载函数一个返回值是 Data 对象，另一个要求提供一个接收数据的 buffer 数组。
 * GetElementType、GetElementSize、GetElementLength函数分别是获取数据的类型、空间大小、数组长度。 
 **/

#ifndef _DATA_H_
#define _DATA_H_
//#include<assert.h>
#include<cstdio>
#include<cstdlib>
#include<initializer_list>
#include<cstring>
#include<vector>
using namespace std;
#define VoidSize 1

//#define MyAssert(_Expression,_ExceptionType,_ExceptionMsg) _MyAssert(_Expression,_ExceptionType,_ExceptionMsg)

#define GenFunc_GetElementValueTYPE(_Type) \
const  GetElementValue##_Type(uint serial_number,uint index=0) const{ \
	MyAssert(serial_number<n,Range,"Serial number is out of range."); \
	MyAssert(type[serial_number]==My##_Type,Type,"Element Type is not matched."); \
	MyAssert(index<GetElementLength(serial_number),Range,"Array index is out of range."); \
	return *((_Type*)(data+serial_number+index)); \
}

#define Myvoid Void
#define Myint Int
#define Mydouble Double
#define Mychar Char

typedef unsigned char uchar;
typedef unsigned int uint;

template<class T>const T& MyMin(const T& a,const T& b) {return a==b?a:a<b?a:b;}
template<class T>const T& MyMax(const T& a,const T& b) {return a==b?a:a<b?b:a;}

uint Sizeof(enum DataType t) {
	switch(t) {
		case Void: return VoidSize;break;
		case Int: return sizeof(int);break;
		case Double: return sizeof(double);break;
		case Char: return sizeof(char);break;
		default: return -1;
	}
}

class DataTypeUnit {
private:
	enum DataType type;
	uint num;
	friend class Type;
public:
	DataTypeUnit(enum DataType _type=Void,uint _num=0);
	virtual ~DataTypeUnit();
};

class Data:public Type {
private:
	uchar* data; // 数据。 
	friend class cmpData;
	friend class BufferManager;
public:
	Data();
	Data(DataTypeUnit init_list[],unsigned int size);
	Data(vector<DataTypeUnit> init_list);
	Data(const Data& a);
	virtual ~Data();
	
	void SetElementValue(unsigned int serial_number,unsigned char value[],unsigned int num);
	void SetDataValue(unsigned char values[],unsigned int bytenum,unsigned int offset=0);
	const Data GetElementValue(unsigned int serial_number);
	void GetElementValue(unsigned int serial_number,unsigned char ret[]);
	const char GetElementValueChar(uint serial_number,uint index=0);
	const int GetElementValueInt(uint serial_number,uint index=0);
	const double GetElementValueDouble(uint serial_number,uint index=0);
	friend bool HaveSameData(const Data& a,const Data& b);  // data 相同。 
	friend bool IsSame(const Data& a,const Data& b); // 完全相同。 
	friend bool LessThan(const Data& a,const Data& b,uint serial_number);
	friend bool Equal(const Data& a,const Data& b,uint serial_number);
	friend bool NotEqual(const Data& a,const Data& b,uint serial_number);
	friend bool operator ==(const Data& a,const Data& b);
	friend bool operator < (const Data& a,const Data& b);
//	friend bool operator < (const Data& a,const Data& b) {
//		int n=MyMin(a.endpos[a.n],b.endpos[b.n]);
//		for(int i=0;i<n;++i) if(a.data[i]!=b.data[i]) return a.data[i]<b.data[i];
//		return a.endpos[a.n-1]<b.endpos[b.n-1];
//	}
	friend bool operator > (const Data& a,const Data& b);
	friend bool operator <=(const Data& a,const Data& b);
	friend bool operator >=(const Data& a,const Data& b);
	Data& operator =(const Data& b);
}; 
class cmpData {
private:
	unsigned int begin;
	unsigned int end;
public:
	cmpData(unsigned int _begin=-1,unsigned int _end=0);
	void SetBegin(unsigned int _begin);
	void Setend(unsigned int _end);
	bool cmp(const Data& a,const Data& b);
};
class cmpTable {
private:
	Type t;
	uint sn;
public:
	
	void SetType(const Type& a);
	void SetSerialNumber(const uint& serial_number);
	bool cmp(uchar* a,uchar *b);
};
bool HaveSameData(const Data& a,const Data& b);
bool HaveSameType(const Type& a,const Type& b);
bool IsSame(const Data& a,const Data& b);
#endif
