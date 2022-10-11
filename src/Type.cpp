#include "pch.h"
#include "Type.h"
typedef unsigned char uchar;
typedef unsigned int uint;

Type::Type():n(0),type(0),endpos(0) {}
Type::Type(DataTypeUnit init_list[],uint size) {
	n=size;
	if(size!=0) {
		unsigned int nowpos=0;
		endpos=new unsigned int[n];
		type=new unsigned char[n];
		auto p=init_list;
		for(unsigned int i=0;i<n;++p,++i) {
			type[i]=(unsigned char)(p->type);
			endpos[i]=nowpos+=(p->num)*Sizeof(p->type);
		}
	}
}
Type::Type(vector<DataTypeUnit> init_list) {
	n=init_list.size();
	if(n!=0) {
		unsigned int nowpos=0;
		endpos=new unsigned int[n];
		type=new unsigned char[n];
		auto p=init_list.begin();
		for(unsigned int i=0;i<n;++p,++i) {
			type[i]=(unsigned char)(p->type);
			endpos[i]=nowpos+=(p->num)*Sizeof(p->type);
		}
	}
}
Type::Type(const Type& a) {
	n=a.n;
	if(a.n) {
		type=new unsigned char[a.n];
		endpos=new unsigned int[a.n];
		memcpy(type,a.type,a.n);
		memcpy(endpos,a.endpos,a.n);
	}
}
Type::~Type() {if(n) delete[]type,delete[]endpos;}

enum DataType Type::GetElementType(unsigned int serial_number) const{
	MyAssert(serial_number<n,Range,"Serial number is out of range.");
	return (enum DataType)type[serial_number];
}
unsigned int Type::GetElementSize(unsigned int serial_number) const{
	MyAssert(serial_number<n,Range,"Serial number is out of range.");
	if(serial_number==0) return endpos[0];
	else return endpos[serial_number]-endpos[serial_number-1];
}
unsigned int Type::GetElementLength(unsigned int serial_number) const{
	return GetElementSize(serial_number)/Sizeof(GetElementType(serial_number));
}
uint Type::GetNumberofElements() {return n;}
uint Type::GetSizeofData() {return endpos[n-1];}

bool HaveSameType(const Type& a,const Type& b) { // type ÏàÍ¬¡£ 
	if(a.n!=b.n) return false;
	for(int i=a.n-1;i>=0;--i) if(a.type[i]!=b.type[i] || a.endpos[i]!=b.endpos[i]) return false;
	return true;
}

Type& Type::operator = (const Type& b) {
	if(HaveSameType(*this,b)) return *this;
	if(n) delete []type,delete []endpos;
	n=b.n;
	type=new unsigned char[b.n];
	endpos=new unsigned int[b.n];
	memcpy(type,b.type,b.n);
	memcpy(endpos,b.endpos,b.n);
	return *this;
}

