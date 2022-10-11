#include "pch.h"
typedef unsigned char uchar;
typedef unsigned int uint;

class Type {
protected:
	uint n; // 数量。
	uchar* type;; // 类型。 
	unsigned int* endpos; // 每个数据段的结尾偏移量。 
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
	friend bool HaveSameType(const Type& a,const Type& b);  // type 相同。 
	
	Type& operator = (const Type& b);
};


