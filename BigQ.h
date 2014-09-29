#ifndef BIGQ_H
#define BIGQ_H

#include <pthread.h>
#include <iostream>
#include "Pipe.h"
#include "File.h"
#include "Record.h"
#include "Comparison.h"
#include <vector>
//#include <algorithm>
#include <queue>

using namespace std;

class Run
{
public:
    Page currentPage;
    int currentPageNumber;
    int totalPages;
};

struct RecordStruct
{
    Record record;
    int run_num;
};

class ComparePQ
{
private:
    OrderMaker* sortOrder;
public:
    ComparePQ(OrderMaker *sortorder)
    {
        sortOrder = sortorder;
    }
    bool operator()(RecordStruct *leftRecord, RecordStruct *rightRecord)
    {
        ComparisonEngine comparisonEngine;
        if(comparisonEngine.Compare(&(leftRecord->record), &(rightRecord->record), sortOrder)<=0)
            return false;
        
        return true;
        
    }
};

class BigQ
{
//Phase - 1
private:
    char* fileName;
    File file;
    vector<int>runIndices;
    int pageCountPerRun;
    Pipe *inPipe, *outPipe;
    OrderMaker *sortOrder;
    int runLength;
    ComparisonEngine ce;
    int appendCount;
    int pagesTotal;
    //Phase - 2
    int MergeRuns();
  
public:
    BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlength);
    ~BigQ();
    //Phase-1
   static void *GenerateRuns(void *ptr);
    void* CreateSortedRuns();
    //Phase-2
    
    bool GetNextRecordFromRun(int runNumber,Run * array,RecordStruct * record,priority_queue<RecordStruct *, vector<RecordStruct *>, ComparePQ> mergeQueue);
    //void writePages( vector<Record *> &pqRecs);
};

class CompareRec
{
private:
    OrderMaker* sortOrder;
public:
    CompareRec(OrderMaker *sortorder)
    {
        sortOrder = sortorder;
    }
    bool operator()(Record* left,Record* right)
    {
        ComparisonEngine comparisonEngine;
        if(comparisonEngine.Compare(left,right,sortOrder)<0)
            return false;
        else
            return true;
    }
};
#endif