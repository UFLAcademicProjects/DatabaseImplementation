#ifndef HEAP_H
#define HEAP_H

#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include <iostream>
#include "GenericDBFile.h"


class Heap : public GenericDBFile {

private:
    File file;
    Page page;
    string metadataFile;
    bool isFileOpen;
    int currentPageNumber;
    int lastPageNumber;
    char* fileName;
    bool isPageDirty;
    void WriteMetadata();
    
    
public:
	Heap ();
    ~Heap();
    
	int Create (char *fpath, void *startup);
	int Open (char *fpath);
	int Close ();
    
	void Load (Schema &myschema, char *loadpath);
    
	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);
    void WritePageToFileIfDirty(Page* page, int);
    //Returns the metadata file name
    inline string GetMetadataFile(){
        return metadataFile;
    }
    
};
#endif
