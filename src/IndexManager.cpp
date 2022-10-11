

#include "IndexManager.h"
#include "BPlusTree.h"

std::string GenIndexName(std::string table_name,std::string index_name) {return table_name+"_"+index_name+".index";}




IndexManager::IndexManager()
{
	bptInt = NULL;
	bptDouble = NULL;
	bptChar = NULL;
}

#define GenCase(_Type,__Type) \
	case _Type: \
		RstBpt##_Type(true); \
		bpt##_Type=new BPlusTree<__Type>(table.name,table.attributes[index.column].name,20); \
		bpt##_Type->Save(FileName.c_str()); \
		bpt##_Type##Name=FileName; \
		return true

bool IndexManager::CreateIndex(Table& table, Index& index)
{
	std::string FileName=GenIndexName(index.table_name,table.attributes[index.column].name);
	enum DataType type=table.attributes[index.column].type;
	switch(type) {
		GenCase(Int,int);
		GenCase(Char,std::string);
		GenCase(Double,double);
	}
	return false;
}

#undef GenCase
#define GenCase(_Type,__Type) \
	case _Type: \
		RstBpt##_Type(true); \
		bpt##_Type=new BPlusTree<__Type>(table.name,table.attributes[i].name,20); \
		bpt##_Type->Save(FileName.c_str()); \
		bpt##_Type##Name=FileName; \
		return true

bool IndexManager::CreateIndex(Table& table)
{
	int i;
	for(i = 0; i < table.attriNum; ++i)
		if(table.attributes[i].isPrimaryKey == true)
			break;
	if(i == table.attriNum) return false;
	std::string FileName=GenIndexName(table.name,table.attributes[i].name);

	switch(table.attributes[i].type) {
		GenCase(Int,int);
		GenCase(Char,std::string);
		GenCase(Double,double);
		default:
			break;
	}
	return false;
}

#undef GenCase

bool IndexManager::HasIndex(const std::string& TableName,std::string& AttribName)
{
	FILE* fp=fopen(GenIndexName(TableName,AttribName).c_str(),"rb");
	if(fp==NULL) return false;
	else {
		fclose(fp);
		return true;
	}
}

bool IndexManager::DropIndex(const std::string& TableName,std::string& AttribName)
{
	std::string FileName=GenIndexName(TableName,AttribName);
	FILE* fp=fopen(FileName.c_str(),"rb");
	if(fp==NULL) return false;
	else fclose(fp);
	if(remove(FileName.c_str())!= 0) perror("Error deleting file");
	if(bptIntName == FileName) RstBptInt(false);
	if(bptDoubleName == FileName) RstBptDouble(false);
	if(bptCharName == FileName)	RstBptChar(false);
	return true;
}

#define GenFunc(_Type) \
void IndexManager::RstBpt##_Type(bool save) { \
	if(bpt##_Type!=NULL) { \
		if(save) bpt##_Type->Save(bpt##_Type->GetSaveFileName().c_str()); \
		delete bpt##_Type; \
		bpt##_Type = NULL; \
	} \
}

GenFunc(Int)
GenFunc(Char)
GenFunc(Double)

#undef GenFunc

void IndexManager::SaveIndex() 
{
	if(bptInt) RstBptInt(true);
	else if(bptDouble) RstBptDouble(true);
	else if(bptChar) RstBptChar(true);
}

#define GenCaseInFuncTYPE(_Type,__Type) \
			case _Type: \
				RstBpt##_Type(true); \
				bpt##_Type=new BPlusTree<__Type>(FileName); \
				bpt##_Type##Name=FileName; \
				bpt##_Type->setAttribute(TableName,AttribName); \
				return true

bool IndexManager::LoadIndex(std::string& TableName,std::string& AttribName,enum DataType type)
{
	std::string FileName=GenIndexName(TableName,AttribName);
	if(FileName == bptIntName || FileName == bptDoubleName || FileName == bptCharName) return true;
	FILE* fp = fopen(FileName.c_str(),"rb");
	if(fp==NULL) return false;
	else fclose(fp);

	switch(type) {
		GenCaseInFuncTYPE(Int,int);
		GenCaseInFuncTYPE(Char,std::string);
		GenCaseInFuncTYPE(Double,double);
		default:
			break;
	}
	return false;
}

#undef GenCaseInFuncTYPE
#define GenCaseInFuncTYPE(_Type,_FuncName) \
			case _Type: \
				if(bpt##_Type==NULL) return false; \
				bpt##_Type -> _FuncName(ato##_Type(value.c_str()),p); \
				return true

bool IndexManager::InsertItem(const std::string& value,Pointer p,enum DataType type)
{
	switch(type) {
		GenCaseInFuncTYPE(Int,Insert);
		GenCaseInFuncTYPE(Char,Insert);
		GenCaseInFuncTYPE(Double,Insert);
		default:
			break;
	}
	return false;
}

#undef GenCaseInFuncTYPE



bool IndexManager::DeletePointers(vector<Pointer>& p,enum DataType type)
{
	switch(type) {
		case Int:
			return bptInt->DeletePointers(p);
		case Char:
			return bptChar->DeletePointers(p);
		case Double:
			return bptDouble->DeletePointers(p);
		default:
			break;
	}
	return false;
}

#define GenSwitCmpCondTYPE(_Type) \
	switch(cond.op) { \
		case Lt: \
			bpt##_Type->FindLess(data,false,p); \
			return true; \
		case Le: \
			bpt##_Type->FindLess(data,true,p); \
			return true; \
		case Gt: \
			bpt##_Type->FindLarger(data,false,p); \
			return true; \
		case Ge: \
			bpt##_Type->FindLarger(data,true,p); \
			return true; \
		case Eq: \
			Pointer po; \
			if(bpt##_Type->Find(data,po)) p.push_back(po); \
			return true; \
		case Ne: \
			bpt##_Type->FindNonEqual(data,p); \
			return true; \
		default: \
			break; \
	}

#define GenCaseInFuncTYPE(_Type) \
			case _Type: { \
				auto data=ato##_Type(cond.value.c_str()); \
				GenSwitCmpCondTYPE(_Type); \
				break; \
			}

bool IndexManager::SelectItem(Condition& cond,enum DataType type,std::vector<Pointer>& p)
{
	p.clear();
	switch(type) {
		GenCaseInFuncTYPE(Int);
		GenCaseInFuncTYPE(Char);
		GenCaseInFuncTYPE(Double);
		default:
			break;
	}
}

#undef GenCaseInFuncTYPE
#undef GenSwitCmpCondTYPE

#define GenCaseInFuncTYPE(_Type,_FuncName) \
			case _Type: \
				if(bpt##_Type==NULL) return 0; \
				return bpt##_Type -> _FuncName(ato##_Type(value.c_str()))

#define GenFunc(_RetType,_FuncName) \
	_RetType IndexManager::_FuncName(std::string& value,enum DataType type) { \
		switch(type) { \
			GenCaseInFuncTYPE(Int,_FuncName); \
			GenCaseInFuncTYPE(Char,_FuncName); \
			GenCaseInFuncTYPE(Double,_FuncName); \
			default: \
				break; \
		} \
		return 0; \
	}

GenFunc(Pointer,FindPointer)

#undef GenCaseInFuncTYPE
#define GenCaseInFuncTYPE(_Type,_FuncName) \
			case _Type: { \
				if(bpt##_Type==NULL) return false; \
				auto tmp=ato##_Type(value.c_str()); \
				bpt##_Type -> _FuncName(tmp); \
				return true; \
			}

GenFunc(bool,Delete)

#undef GenFunc
#undef GenCaseInFuncTYPE
#define GenCaseInFuncTYPE(_Type,_FuncName) \
			case _Type: \
				if(bpt##_Type==NULL) return false; \
				return bpt##_Type -> _FuncName(ato##_Type(value.c_str()),p)

bool IndexManager::Find(std::string& value,enum DataType type)
{
	Pointer p;
	switch(type) {
		GenCaseInFuncTYPE(Int,Find);
		GenCaseInFuncTYPE(Char,Find);
		GenCaseInFuncTYPE(Double,Find);
		default:
			break;
	}
	return 0;
}

#undef GenCaseInFuncTYPE

/*
void IndexManager::printS() {
	if(bptChar)
		bptChar->PrintAll();
}

*/
IndexManager::~IndexManager()
{
	RstBptInt(true);
	RstBptChar(true);
	RstBptDouble(true);
}

