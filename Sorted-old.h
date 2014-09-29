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


struct SortInfo
{
    OrderMaker *myOrder;
    int runLength;
};

class Sorted : public GenericDBFile {

private:
    SortInfo *sortInfo;
    BigQ *bigQ;
    Pipe *inPipe;
    Pipe *outPipe;
    File file;
    Page page;
    bool readingMode;
    string metadataFile;
    bool isFileOpen;
    int currentPageNumber;
    int lastPageNumber;
    char* fileName;
    bool isPageDirty;
    void WriteMetadata();
    
    
public:
	Sorted ();
    ~Sorted();
    
	int Create (char *fpath, void *startup);
	int Open (char *fpath);
	int Close ();
    
	void Load (Schema &myschema, char *loadpath);
    
	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);
    void WritePageToFileIfDirty();
    bool InsertIntoBigQ();
    //Returns the metadata file name
    inline string GetMetadataFile(){
        return metadataFile;
    }
    
};
#endif
