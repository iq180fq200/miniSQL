#include "pch.h"
class Table:public Type {
private:
	uchar** data;
	void MemoryAllocation(int num) { // Allocate memory without initialization!
		record_num=num;
		uchar* tmp=new uchar[endpos[n-1]*num];
		data=new uchar*[num];
		data[num-1]=tmp+(endpos[n-1]-1)*num;
		for(int i=num-2;i>=0;--i) data[i]=data[i+1]-n;
	}
public:
	uint record_num;
	Table():record_num(0),data(0) {}
	Table(DataTypeUnit init_list[],uint size,uint num):Type(init_list,size) {
		if(n) MemoryAllocation(num);
	}
	Table(vector<DataTypeUnit> init_list,uint num):Type(init_list) {
		if(n) MemoryAllocation(num);
	}
	Table(const Table& a):Type(a) {
		if(a.n) MemoryAllocation(a.record_num);
	}
	virtual ~Table() {if(n) delete[]data;}
	
	void SetDataValue(uchar values[],uint bytenum,uint row_number) { // Öð×Ö½Ú¸³Öµ¡£ 
		MyAssert(bytenum<=endpos[n-1],Range,"Byte number is out of range.");
		MyAssert(row_number<n,Range,"Row number is out of range.");
		memcpy(data[row_number],values,bytenum);
	}
	void SetDataValue(uchar values[],uint bytenum) {
		MyAssert(bytenum<=endpos[n-1]*record_num,Range,"Byte number is out of range.");
		memcpy(data[0],values,bytenum);
	}
};
