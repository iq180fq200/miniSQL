#pragma once
#include"Catalog.h"
#include"BufferManager.h"
#include"BPlusTree.h"
#include"global.h"
#include "IndexManager.h"
#include"Interpreter.h"
#include"RecordManager.h"
void APIcreateTable(Table& table);
void APIcreateIndex(Table&, Index&);
void APIDropTable(string tableName);
void APIDropIndex(string indexName);
