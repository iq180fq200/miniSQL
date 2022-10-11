#include "pch.h"
typedef unsigned char uchar;
typedef unsigned int uint;

class Type {
protected:
	uint n; // ������
	uchar* type;; // ���͡� 
	unsigned int* endpos; // ÿ�����ݶεĽ�βƫ������ 
	friend class cmpTable;
public:
	Type();
	Type(DataTypeUnit init_list[],uint size);
	Type(vector<DataTypeUnit> init_list);
	Type(const Type& a);
	virtual ~Type();
	
	enum DataType GetElementType(unsigned int serial_number);
	unsigned int GetElementSize(unsigned int serial_number);
	unsigned int GetElementLength(unsigned int serial_number);
	uint GetNumberofElements();
	uint GetSizeofData();
	friend bool HaveSameType(const Type& a,const Type& b);  // type ��ͬ�� 
	
	Type& operator = (const Type& b);
};


