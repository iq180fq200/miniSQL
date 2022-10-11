/***
 * ������ʵ���� Data �࣬����Ķ���Ϊ����������͵���ϡ�
 * ����˽�б���Ϊ n:����������type:ÿ�����������ͣ�data:�洢�����ǣ�endpos:ÿ����������ֹλ�á� 
 * ���磺{char,int,double,int[3]}��ɵ�����У�
 *     n=4��type[]={Char,Int,Double,Int}��
 *   �ֽڱ�ţ�0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
 *     data[]={c, i, i, i, i, d, d, d, d, d, d, d, d, i, i, i, i, i, i, i, i, i, i, i, i}��
 *     endpos[]={1,5,13,25}�� 
 *
 * DataTypeUnit �����ڸ� Data ��ʼ����DataTypeUnit�����а���һ����ʾ���͵ı��� type����һ����ʾ�����ı��� num��
 * ������ DataTypeUnit ������������� Data ������й��졣
 * GetElementValue(sn) ���Է��ض����е� sn �����ݵ����ͺ�ֵ��sn �� 0 ��ʼ��š�
 *    �������غ���һ������ֵ�� Data ������һ��Ҫ���ṩһ���������ݵ� buffer ���顣
 * GetElementType��GetElementSize��GetElementLength�����ֱ��ǻ�ȡ���ݵ����͡��ռ��С�����鳤�ȡ� 
 **/

#include "Data.h"
//#include<assert.h>
#include<cstdio>
#include<cstdlib>
#include<initializer_list>
#include<cstring>
#include<vector>
using namespace std;
#define VoidSize 1
#define _MyAssert(_Expression,_ExceptionType,_ExceptionMsg) \
	(void)((!!(_Expression))|| \
    (printf("�ļ�����%s\n",__FILE__)),\
    printf("�кţ�%d\n",__LINE__),\
    printf("��������%s\n",__FUNCTION__),\
	throw err##_ExceptionType##Exception(_ExceptionMsg), \
    exit(1))
//	throw err##_ExceptionType##Exception(#_ExceptionMsg)

#define MyAssert(_Expression,_ExceptionType,_ExceptionMsg) _MyAssert(_Expression,_ExceptionType,_ExceptionMsg)

#define GenFunc_GetElementValueTYPE(_Type) \
const _Type Data::GetElementValue##_Type(uint serial_number,uint index=0) const{ \
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

uint Sizeof(enum DataType t) {
	switch(t) {
		case Void: return VoidSize;break;
		case Int: return sizeof(int);break;
		case Double: return sizeof(double);break;
		case Char: return sizeof(char);break;
		default: return -1;
	}
}

DataTypeUnit::DataTypeUnit(enum DataType _type=Void,uint _num=0):type(_type),num(_num) {}
DataTypeUnit::~DataTypeUnit() {}

Data::Data():data(0) {}
Data::Data(DataTypeUnit init_list[],unsigned int size):Type(init_list,size) {
	if(n) data=new uchar[endpos[n-1]]; // without initialization!
}
Data::Data(vector<DataTypeUnit> init_list):Type(init_list) {
	if(n) data=new uchar[endpos[n-1]]; // without initialization!
}
Data::Data(const Data& a):Type(a) {
	if(a.n) {
		data=new uchar[a.endpos[a.n-1]];
		memcpy(data,a.data,a.endpos[a.n-1]);
	}
}
Data::~Data() {if(n) delete[]data;}
	
void Data::SetElementValue(unsigned int serial_number,unsigned char value[],unsigned int num) { // ��һ��Ԫ�ظ�ֵ�� 
	MyAssert(serial_number<n,Range,"Serial number is out of range.");
	num*=Sizeof((enum DataType)type[serial_number]);
	if(serial_number) {
		MyAssert(num<=endpos[serial_number]-endpos[serial_number-1],Range,"Byte number is out of range.");
		memcpy(data+endpos[serial_number-1],value,num);
	}
	else {
		MyAssert(num<=endpos[0],Range,"Byte number is out of range.");
		memcpy(data,value,num);
	}
}
void Data::SetDataValue(unsigned char values[],unsigned int bytenum,unsigned int offset=0) { // ���ֽڸ�ֵ�� 
	MyAssert(bytenum+offset<=endpos[n-1],Range,"Byte number is out of range.");
	memcpy(data+offset,values,bytenum);
}
const Data Data::GetElementValue(unsigned int serial_number) {
	MyAssert(serial_number<n,Range,"Serial number is out of range.");
	unsigned int num=(endpos[serial_number]-(serial_number?endpos[serial_number-1]:0))/Sizeof((enum DataType)type[serial_number]);
	DataTypeUnit tmpDTU=DataTypeUnit((enum DataType)type[serial_number],num);
	Data tmp(&tmpDTU,1);
	tmp.SetElementValue(0,data+(serial_number?endpos[serial_number-1]:0),num);
	return tmp;
}
void Data::GetElementValue(unsigned int serial_number,unsigned char ret[]) {
	MyAssert(serial_number<n,Range,"Serial number is out of range.");
	if(serial_number) memcpy(ret,data+endpos[serial_number-1],endpos[serial_number-1]-endpos[serial_number]);
	else memcpy(ret,data,endpos[0]);
}
GenFunc_GetElementValueTYPE(char);
GenFunc_GetElementValueTYPE(int);
GenFunc_GetElementValueTYPE(double);
bool HaveSameData(const Data& a,const Data& b);  // data ��ͬ�� 
bool IsSame(const Data& a,const Data& b); // ��ȫ��ͬ�� 
bool LessThan(const Data& a,const Data& b,uint serial_number) { // �Ƚϵ� serial_number �����Ĵ�С�� 
	MyAssert(serial_number<a.n,Range,"Serial number is out of range.");
	MyAssert(serial_number<b.n,Range,"Serial number is out of range.");
	MyAssert(a.type[serial_number]==b.type[serial_number],Type,"Compare two elements with different Type.");
	if(a.type[serial_number]==Double) return a.GetElementValuedouble(serial_number)<b.GetElementValuedouble(serial_number);
	if(a.type[serial_number]==Int) return a.GetElementValueint(serial_number)<b.GetElementValueint(serial_number);
	if(a.type[serial_number]==Char) return a.GetElementValuechar(serial_number)<b.GetElementValuechar(serial_number);
//		else {
//			for(uint i=(serial_number==0?0:a.endpos[serial_number-1]);i<a.endpos[serial_number];++i)  if(a.data[i]!=b.data[i]) return a.data[i]<b.data[i];
//			return false;
//		}
}
bool Equal(const Data& a,const Data& b,uint serial_number) {
	MyAssert(serial_number<a.n,Range,"Serial number is out of range.");
	MyAssert(serial_number<b.n,Range,"Serial number is out of range.");
	MyAssert(a.type[serial_number]==b.type[serial_number],Type,"Compare two elements with different Type.");
	if(a.type[serial_number]==Double) return a.GetElementValuedouble(serial_number)==b.GetElementValuedouble(serial_number);
	else {
		for(uint i=(serial_number==0?0:a.endpos[serial_number-1]);i<a.endpos[serial_number];++i)  if(a.data[i]!=b.data[i]) return false;
		return true;
	}
}
bool NotEqual(const Data& a,const Data& b,uint serial_number) {return !Equal(a,b,serial_number);}
bool operator ==(const Data& a,const Data& b) {
	if(!HaveSameType(a,b)) throw errCompareException("Compare two Datas with different types.");
	for(uint i=0;i<a.n;++i) if(NotEqual(a,b,i)) return false;
	return true;
}
bool operator < (const Data& a,const Data& b) {
	if(!HaveSameType(a,b)) throw errCompareException("Compare two Datas with different types.");
	for(uint i=0;i<a.n;++i) if(NotEqual(a,b,i)) return LessThan(a,b,i);
	return false; 
} 
//	friend bool operator < (const Data& a,const Data& b) {
//		int n=MyMin(a.endpos[a.n],b.endpos[b.n]);
//		for(int i=0;i<n;++i) if(a.data[i]!=b.data[i]) return a.data[i]<b.data[i];
//		return a.endpos[a.n-1]<b.endpos[b.n-1];
//	}
bool operator > (const Data& a,const Data& b) {return b<a;}
bool operator <=(const Data& a,const Data& b) {return !(a>b);}
bool operator >=(const Data& a,const Data& b) {return !(b>a);}
Data& Data::operator =(const Data& b) {
	if(IsSame(*this,b)) return *this;
	if(n) delete []data,delete []type,delete []endpos;
	n=b.n;
	data=new unsigned char[b.endpos[b.n-1]];
	type=new unsigned char[b.n];
	endpos=new unsigned int[b.n];
	memcpy(data,b.data,b.endpos[b.n-1]);
	memcpy(type,b.type,b.n);
	memcpy(endpos,b.endpos,b.n);
	return *this;
}
 
cmpData::cmpData(unsigned int _begin=-1,unsigned int _end=0):begin(_begin),end(_end) {}
void cmpData::SetBegin(unsigned int _begin) {begin=_begin;}
void cmpData::Setend(unsigned int _end) {end=_end;}
bool cmpData::cmp(const Data& a,const Data& b) { // return a.data[begin...end) < b.data[begin,end); 
	MyAssert(begin>=0 && begin<end && end<=a.n && end<=b.n,Range,"Compare section is out of range.");
	for(unsigned int i=begin;i<end;++i) if(a.data[i]!=b.data[i]) return a.data[i]<b.data[i];
	return false;
}
#define QAQ_CMP(_Type,a,b) \
	if(t.type[sn]==My##_Type) { \
		_Type* bga=(_Type*)(a+(sn?t.endpos[sn-1]:0)); \
		_Type* bgb=(_Type*)(b+(sn?t.endpos[sn-1]:0)); \
		_Type* end=(_Type*)(a+t.endpos[sn]); \
		while(bga!=end) if(*bga!=*bgb) return *bga<*bgb;else ++bga,++bgb;return false; \
	}
bool cmpTable::cmp(uchar* a,uchar *b) {
	QAQ_CMP(double,a,b);
	QAQ_CMP(int,a,b);
	QAQ_CMP(char,a,b);
}
#undef QAQ_CMP
bool HaveSameData(const Data& a,const Data& b) { // data ��ͬ�� 
	if(a.endpos[a.n-1]!=b.endpos[b.n-1]) return false;
	for(int i=a.endpos[a.n-1]-1;i>=0;--i) if(a.data[i]!=b.data[i]) return false;
	return true;
}
bool IsSame(const Data& a,const Data& b) { // ���� Data ��ȫ��ͬ�� 
	return HaveSameType(a,b) && HaveSameData(a,b);
}
