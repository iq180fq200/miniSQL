#include"API.h"

extern RecordManager record;
extern IndexManager index;
extern CatalogManager catalog;
extern BufferManager buf;


//extern IndexManager index;
//extern Interpreter myinterpreter;
//extern RecordManager record;
//extern CatalogManager catalog;
//extern BufferManager buf;

void APIcreateTable(Table& table) {
	catalog.createTable(table);
	catalog.WriteToBuffer();
};

void APIcreateIndex(Table& tab, Index& id) {
	index.CreateIndex(tab, id);
	catalog.createIndex(id);
	catalog.WriteToBuffer();
	record.createIndex(tab, id);
}

void APIDropTable(string tableName) {
	catalog.dropTable(tableName);
	catalog.WriteToBuffer();
}

void APIDropIndex(string indexName) {
	catalog.dropIndex(indexName);
	catalog.WriteToBuffer();
}
