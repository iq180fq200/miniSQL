#include"src/Catalog.h"
#include"src/global.h"
#include"src/IndexManager.h"
#include"src/RecordManager.h"
#include"src/Interpreter.h"
#include"src/BufferManager.h"
#include"src/Page.h"
#include <time.h>
using namespace std;
bool infile = 0;
long long start, endtime;
int BufferManager::		lruCounter[CACHE_CAPACITY];
Page BufferManager::	cachePages[CACHE_CAPACITY];
bool BufferManager::	pined[CACHE_CAPACITY];
bool BufferManager::	isDirty[CACHE_CAPACITY];

Interpreter i;
RecordManager record;
IndexManager index;
CatalogManager catalog;
BufferManager buf;

int main()
{
	bool execfile = false;
	static auto _ = []()
	{
		ios::sync_with_stdio(false);
		cin.tie(0);
		cout.tie(0);
		return 0;
	}();
	while (i.flag == true)
	{
		cout << "MINI SQL >>>" << ends;
		i.Query();
		if (i.query != "")
		{
			i.Choice();
		}
	}
	return 0;
}
