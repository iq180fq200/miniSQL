/*
 * File name: BufferManager.cpp
 * Author: Xuanming, Liu
 * Latest revised: 2020.06.18
 * Description: 
 * Implementation for BufferManagement.
**/
#include "BufferManager.h"
#include "global.h"
bool BufferManager::readPage( Page& page )
{
	//assert(page.pageIndex == PageType::Undefined);
    int pageIndex = findPageInCache(page);
    if( pageIndex != PAGE_NOTFOUND ) {
        // read directly from cache.
        page = cachePages[pageIndex];
        lruCounterAddExceptCurrent(pageIndex);
        return true;
    }
    else {
        forceReadPageFromFile(page);
        pageIndex = getUnpinedBiggestPageFromCache();
        if (pageIndex == PAGE_NOTFOUND) {
            return true;
        } else {
            if (isDirty[pageIndex]) {
                isDirty[pageIndex] = false;
                // If it's dirty page, write it again to file
                forceWritePageToFile(cachePages[pageIndex]);
            }
            // use the new page to replace the page
            cachePages[pageIndex] = page;
            lruCounterAddExceptCurrent(pageIndex);
            //cout << (void*)page.pageData<<endl;
            //cout << (void*)cachePages[pageIndex].pageData<<endl;
            return true;
        }        
    }
}

bool BufferManager::writePage( Page& page )
{
    assert(page.pageType != PageType::Undefined);
    int pageIndex = findPageInCache(page);
    if ( pageIndex != PAGE_NOTFOUND ) {
        cachePages[pageIndex] = page;
        isDirty[pageIndex] = true;
        lruCounterAddExceptCurrent(pageIndex);
        return true;
    }
    else {
        pageIndex = getUnpinedBiggestPageFromCache();
        if (pageIndex == PAGE_NOTFOUND) {
            forceWritePageToFile(page);// May cause bug
            return true;
        } 
        else {
            if (isDirty[pageIndex]) {
                isDirty[pageIndex] = false;
                forceWritePageToFile(cachePages[pageIndex]);
            }
            cachePages[pageIndex] = page;
            lruCounterAddExceptCurrent(pageIndex);
            return true;
        }
    }
    return false;
}

Page& BufferManager::recordManagerGetBlankPage() 
{
    Page* newPage = new Page();
    newPage->pageType = PageType::RecordPage;
    Page& newPageRef = (*newPage);
    return newPageRef;
}

bool BufferManager::pinPage(Page &page)
{
    int retFlag = false;
    for (int i = 0; i < CACHE_CAPACITY; ++i) {
        if (cachePages[i] == page) {
            pined[i] = true;
            retFlag = true;
            break;
        }
    }
    return retFlag;
}

bool BufferManager::unpinPage(Page &page)
{
    int retFlag = false;
    for (int i = 0; i < CACHE_CAPACITY; ++i) {
        if (cachePages[i] == page) {
            pined[i] = false;
            retFlag = true;
            break;
        }
    }
    return retFlag;
}

PageIndex BufferManager::findPageInCache( Page& page )
{
    int retIndex = PAGE_NOTFOUND;
    for (int i = 0; i < CACHE_CAPACITY; i++)
        if (cachePages[i] == page) {
            retIndex = i;
            break;
        }
    return retIndex;
}

void BufferManager::lruCounterAddExceptCurrent( int index )
{
    for( int i = 0; i < CACHE_CAPACITY; i++ )
        lruCounter[i]++;
    // newly used, set as 0. Least recently used algorithm.
    lruCounter[index] = 0;
}

bool BufferManager::forceReadPageFromFile( Page& page )
{
    string filename = (page.tableName + "." + "record");
    Page temp;    
    FILE* fp = fopen(filename.c_str(), "rb");
    if (fp) {
        fseek(fp, PAGE_SIZE * page.ofs, SEEK_SET);
        
	    fread( temp.pageData, PAGE_SIZE, SEEK_CUR, fp);
	    fclose(fp);

        temp.ofs = page.ofs;
        temp.pageIndex = page.pageIndex;
        temp.pageType = page.pageType;
        temp.tableName = page.tableName;
        page = temp;
        return true;
    }
    
    /*
    fstream file(filename, ios::out | ios::in);
    if (file.fail())
        return false;
    file.seekp(PAGE_SIZE * page.ofs,ios::beg);
    if (!file.eof()) {
        file.read(temp.pageData, PAGE_SIZE);
    }
    file.close();
    */
    return true;
}

bool BufferManager::forceWritePageToFile( Page& page )
{
    string filename = (page.tableName + "." + "record");
    FILE* fp = fopen(filename.c_str(), "rb+");
	if (fp) {
        fseek(fp, PAGE_SIZE * page.ofs, SEEK_SET);
		fwrite( page.pageData, PAGE_SIZE, SEEK_CUR, fp);
		fclose(fp);
       //delete& page;
       return true;
	}
    return false;
    
   
   /*
    ofstream output(filename, ios::out);
    if (output.fail())
        return false;
    output.seekp(PAGE_SIZE * page.ofs, ios::beg);
    output.write(page.pageData, PAGE_SIZE);
    output.close();
    return true;
    */
}

PageIndex BufferManager::getUnpinedBiggestPageFromCache()
{
    int retIndex = PAGE_NOTFOUND;
    int bigSaver = -1;
    for (int i = 0; i < CACHE_CAPACITY; i++) {
        if ((!pined[i]) && (lruCounter[i] > bigSaver)) {
            retIndex = i;
            bigSaver = lruCounter[i];
        }
    }
    return retIndex;
}

