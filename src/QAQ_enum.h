#ifndef _MINISQL_ENUM_H_
#define _MINISQL_ENUM_H_

typedef enum _OperationType {
	MyCreate = 0,
	MyDrop,
	MyInsert,
	MyDelete,
	MySelect,
	MyCreate_index,
	MyDrop_index,
	MyExecfile,
	MyQuit
}OperationType;
enum DataType {
	Void=0,
	Int,
	Double,
	Char,
};

#endif
