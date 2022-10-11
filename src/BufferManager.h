

#ifndef _MINISQL_BUFFERMANAGER_H_
#define _MINISQL_BUFFERMANAGER_H_

#include "Page.h"
#include "global.h"
using namespace std;

class BufferManager{
private:
    static int lruCounter[CACHE_CAPACITY];
    static Page cachePages[CACHE_CAPACITY]; 
    static bool pined[CACHE_CAPACITY];
    static bool isDirty[CACHE_CAPACITY];

public:
    BufferManager(){
        memset(lruCounter, 0, CACHE_CAPACITY);
        memset(pined, false, CACHE_CAPACITY);
        memset(isDirty, false, CACHE_CAPACITY);
    }

    ~BufferManager(){
        for( int i = 0; i < CACHE_CAPACITY; ++i) 
            forceWritePageToFile (cachePages[i]);
    }

    bool readPage( Page& );
    bool writePage( Page& );
    Page& recordManagerGetBlankPage();
    bool pinPage( Page& );
    bool unpinPage( Page& );
    PageIndex findPageInCache(Page&);
private:
    bool forceReadPageFromFile( Page& );
    bool forceWritePageToFile( Page& );
    void lruCounterAddExceptCurrent( int );
    PageIndex getUnpinedBiggestPageFromCache();

};

#endif
