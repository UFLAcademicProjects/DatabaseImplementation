#ifndef SORTED_H
#define SORTED_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include <iostream>
#include "GenericDBFile.h"
#include "BigQ.h"
#include <queue>


struct SortInfo
{
    OrderMaker *myOrder;
    int runLength;
};
/*This record struct keeps the record along with its source
i.e. whether it is coming from input pipe or from existing sorted file*/
struct RecordStructForSorted
{
    Record record;
        /*it will be 0 if record is from outputPipe else 1 if from sorted file*/
    bool sourceName;
};

class ComparePQSorted
{
private:
    OrderMaker* sortOrder;
public:
    ComparePQSorted(OrderMaker *sortorder)
    {
        sortOrder = sortorder;
    }
    bool operator()(RecordStructForSorted *leftRecord, RecordStructForSorted *rightRecord)
    {
        ComparisonEngine comparisonEngine;
        if(comparisonEngine.Compare(&(leftRecord->record), &(rightRecord->record), sortOrder)<=0)
            return false;

        return true;

    }
};


class Sorted : public GenericDBFile {

private:
    SortInfo *sortInfo;
    BigQ *bigQ;
    Pipe *inPipe;
    Pipe *outPipe;
    File file;
    File mergeFile;
    Page page;
    Page mergePage;
    bool readingMode;
    string metadataFile;
    bool isFileOpen;
    int currentPageNumber;
    int currentMergePageNumber;
    int lastPageNumber;
    int numRecsIn;
    char* fileName;
    bool isPageDirty;
    void WriteMetadata();
    /*Maintains number of records in the input pipe*/
    int numRecsInPipe;
    int getnextcount;
    bool pageExists;
    bool queryCreated;
    OrderMaker *queryMaker;
    bool pageFound;
    
    
public:
	Sorted ();
    ~Sorted();
    
	int Create (char *fpath, void *startup);
	int Open (char *fpath);
	int Close ();
    
	void Load (Schema &myschema, char *loadpath);
    
	void MoveFirst ();
	void Add (Record &addme);
    int GetNextRecord (Record &fetchme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);
    void WritePageToFileIfDirty(Page* tempPage);
    void MergeRecordsWithSortedFile();
    void InsertRecordIntoFile(Record &rec);
    int PageLoad(Record&lit);
    int BinarySearch(int low, int high, Record&lit, int pageNum);
    void SetState(Page& page, int& pageNumber);
    void GetState(Page& page, int& pageNumber);
    int FetchNextFromFile (Record &fetchme);
    //Returns the metadata file name
    inline string GetMetadataFile(){
        return metadataFile;
    }
    
    
    
};
#endif
