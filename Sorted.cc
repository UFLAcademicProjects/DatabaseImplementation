#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"
#include "Sorted.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <ctime>
#include <sys/time.h>
#define DEBUG
#define BUFF_SIZE 100


//Just a simple copy of Heap.cc.
//Need to change the datastructure
using namespace std;


Sorted::Sorted () {
    numRecsInPipe=0;
    numRecsIn=1;
    getnextcount=0;
    readingMode=true;
    inPipe = new Pipe(BUFF_SIZE);
    outPipe = new Pipe (BUFF_SIZE);
    bigQ=NULL;
    pageFound=false;
    pageExists=false;;
    queryCreated=false;;
    queryMaker=NULL;
    
}

Sorted::~Sorted(){
    //delete fileName;
    //delete bigQ;
    
    
}

//Create Done
int Sorted::Create (char *f_path, void *startup) {
#ifdef DEBUG
//    cout<<"Sorted:Saving Metadata"<<endl;
#endif
    //Save metadata file path name
    metadataFile=f_path;
    
    /*This is needed because when we move data from merged file,
     we need original file path*/
    fileName = f_path;
    sortInfo=(SortInfo*)startup;
    
    this->file.Open(0, f_path);
    
    if(this->file.GetFileStatus()<0)
        return 0;
    this->isPageDirty=false;
    this->currentPageNumber=1;
    
    //Save metadata to file
    WriteMetadata();
    return 1;
}

//Load the schema
void Sorted::Load (Schema &f_schema, char *loadpath) {
    
    //Set writing mode to true i.e. reading mode is false
    if(readingMode)
        readingMode=false;
    
    //Open the file
    FILE *fileToLoad = fopen(loadpath, "r");
    if(!fileToLoad){
        cout<< "Can't open file name :" + string(loadpath);
    }
    
    Record record;
    //Load the schema and add each record
    while(record.SuckNextRecord(&f_schema,fileToLoad)==1){
        readingMode = false;
        Add(record);
        
    }
}

//Open Done
//Open the file
int Sorted::Open (char *f_path) {
    
    //cout<<"Sorted:Opening a sorted file"<<endl;
    string fileType;
    string type;
   	metadataFile=f_path;
    int numberOfAttributes;
    string metadataFilePath=metadataFile+".metadata";
    ifstream readMetadata;
    
    readMetadata.open(metadataFilePath.c_str());
//     if(!sortInfo)
    {
        sortInfo=new SortInfo();
        sortInfo->myOrder=new OrderMaker();
        
        readMetadata>>fileType;
        readMetadata>>sortInfo->runLength;
        readMetadata>>numberOfAttributes;
        int whichAtts[MAX_ANDS];
        
        Type whichTypes[MAX_ANDS];
        for(int count=0;count<numberOfAttributes;count++){
            //Set which attribute;
            readMetadata>>whichAtts[count];
            readMetadata>>type;
            if(type.compare("Int")==0){
                whichTypes[count]=Int;
            }
            else if(type.compare("Double")==0){
                whichTypes[count]=Double;
            }
            else
                whichTypes[count]=String;
            
        }
        //Set numberOfAttributes whichAtts and whichTypes
        sortInfo->myOrder->SetAttributeMetadata(numberOfAttributes,whichAtts,whichTypes);
        
    }
    
    this->file.Open(1, f_path);
    //cout<<"***********Sorted File Length after opeing the file: **********"<<file.GetLength()<<endl;
    //cout<<"FileName: "<<f_path<<endl;
    if(this->file.GetFileStatus()<0)
        return 0;
    
    //cout<<"File opened successfully"<<endl;
    
    //Initialize parameters
    this->isPageDirty=false;
    this->currentPageNumber=1;
    //numRecsInPipe=0;
    return 1;
    
}

void Sorted::MoveFirst () {
    /*Since we are moving to the first page, page number is 1*/
    //MergeRecordsWithSortedFile();
    if(file.GetLength()!=0) {
        queryCreated=false;
//        cout<<"File length"<<file.GetLength()<<endl;
        this->file.GetPage(&this->page, 0);
        this->currentPageNumber = 1;
        queryMaker=NULL;
        pageFound=false;
        
    }
}

int Sorted::Close () {
#ifdef DEBUG
    //cout<<"Sorted:Calling close"<<endl;
#endif
    if(isPageDirty){
        //WritePageToFileIfDirty(&page);
        //While closing merge records in sorted file
        MergeRecordsWithSortedFile();
    }
    //readingMode=true;
    //WritePageToFileIfDirty(&this->page, this->currentPageNumber);
    return this->file.Close();
}

void Sorted::Add (Record &rec) {
    
    /**
     initialize the big queue instance
     */
    if(readingMode){
        readingMode=false;
    }
    
    //Declared BUFF SIZE as macro
    //	if(numRecsInPipe/BUFF_SIZE == 1){
    //		cout<<"Pipe is full.Calling MERGERECORDS"<<endl;
    //        /*The input pipe is full and we need to
    //         merge the records into sorted file*/
    //        inPipe->ShutDown();
    //
    //        MergeRecordsWithSortedFile();
    //        isPageDirty=false;
    //        /*Reset the num recs in Pipe*/
    //        numRecsInPipe = 0;
    //
    //        /*Open the pipe again*/
    //        inPipe->Open();
    //
    //        /*Last recorded would not have been added
    //         Hence adding it now and incrementing the count.*/
    //        inPipe->Insert(&rec);
    //        numRecsInPipe++;
    //        isPageDirty=true;
    //
    //	}else{
    isPageDirty=true;
    //inPipe->Insert(&rec);
    numRecsIn++;
    if(!bigQ){
        bigQ = new BigQ(*inPipe, *outPipe, *(sortInfo->myOrder), sortInfo->runLength);
    }
    inPipe->Insert(&rec);
    //}
    
    
}

//void Sorted :: MergeRecordsWithSortedFile(){
//
//	/*This is going to maintain the newly inputed records in the sorted order*/
//#ifdef DEBUG
//    cout<<"BigQ exists????"<<endl;
//#endif
//    //isPageDirty=false;;
//
//    /*CULPRIT: The pipe has to be shutdown before bigq construction
//     If not it waits for the output pipe to remove the record and hangs*/
//    //inPipe->ShutDown();
//
//    //Create the instance only if it does not exist
//    if(!bigQ){
//        return;
//        //bigQ = new BigQ(*inPipe, *outPipe, *(sortInfo->myOrder), sortInfo->runLength);
//    }
//    inPipe->ShutDown();
//#ifdef DEBUG
//    cout<<"BigQ exists"<<endl;
//#endif
//
//
//	/*This merge queue merges the records from already sorted file with newly added records.
//     And sort order should be same as that of the sorted file*/
//	priority_queue<RecordStructForSorted *, vector<RecordStructForSorted *>, ComparePQSorted> mergeQueue(&(*sortInfo->myOrder));
//
//    /*This file is created just for merging.*/
//    /*Create a new file with timestamp associated.Without timestamp
//     few irregularities appear*/
//    std::time_t seconds = std::time(0);
//    stringstream ss;
//    ss<<seconds;
//    string mFile="mergedFile"+ss.str();
//    char* mergeFileName= new char[mFile.size()+1];
//    std::copy(mFile.begin(), mFile.end(), mergeFileName);
//    mergeFileName[mFile.size()] = '\0';
//#ifdef DEBUG
//    cout<<"*********MergingRecords :::: Sorted File Length before opening is :  "<<file.GetLength()<<endl;
//#endif
//
//    //file.Open(1, "supplier.bin");
//    this->mergeFile.Open(0, mergeFileName);
//    this->mergePage.EmptyItOut();
//    file.Open(1,const_cast<char*>(metadataFile.c_str()));
//    #ifdef DEBUG
//    cout<<"*********MergingRecords :::: Sorted File Length after opening is :  "<<file.GetLength()<<endl;
//    #endif
//    int pagesTotal=0;
//
//    /*Check if the main file exists*/
//    if(file.GetLength() != 0){
//        #ifdef DEBUG
//        cout<<"MergingRecords :::: Sorted File Length is not zero ";
//        #endif
//        pagesTotal=file.GetLength()-1;
//        RecordStructForSorted *pipeRecStruct, *fileRecStruct,*nextRecStruct;
//        Record *pipeRec = new Record;
//        Record *fileRec = new Record;
//        Record *nextRec = new Record;
//
//        /*Get initial record from output pipe and put into merge queue*/
//        if(outPipe->Remove(pipeRec)){
//            cout<<"Fetching first record from outpipe"<<endl;
//            pipeRecStruct = new RecordStructForSorted;
//            pipeRecStruct->record = *pipeRec;
//            pipeRecStruct->sourceName = 0;
//            /*Insert it into the queue*/
//            mergeQueue.push(pipeRecStruct);
//        }
//        /*Get initial record from sorted file and put into merge queue*/
//        file.GetPage(&(this->page), 0);
//
//#ifdef DEBUG
//        cout<<"MergingRecords :::: Fetching the first page from Sorted file ";
//#endif
//        if(page.GetFirst(fileRec)){
//            fileRecStruct = new RecordStructForSorted;
//            fileRecStruct->record = *fileRec;
//            fileRecStruct->sourceName = 1;
//	        /*Insert it into the queue*/
//            mergeQueue.push(fileRecStruct);
//        }
//
//        while(!mergeQueue.empty())
//        {
//            #ifdef DEBUG
//            cout<<"MergingRecords :::: MergeQueue is not empty.Insert the records to temp merge file"<<endl;
//            #endif
//            //Fetch the record from priority queue
//            RecordStructForSorted *candidate=new RecordStructForSorted;
//            candidate=mergeQueue.top();
//            bool sourceCandidate = candidate->sourceName;
//
//            //Delete the record from priority queue
//            mergeQueue.pop();
//
//            /*insert the record into the merge file using similar function as heap*/
//
//            InsertRecordIntoFile(candidate->record);
//
//            cout<<"Inside mergequeue : Source Candiate"<<sourceCandidate<<endl;
//            /*Pull the next record from the same source*/
//            nextRecStruct = new RecordStructForSorted;
//            if(!sourceCandidate){
//            cout<<"Inserting a record from outpipe"<<endl;
//                if(outPipe->Remove(nextRec)){
//                    nextRecStruct->sourceName = 0;
//                    nextRecStruct->record = *nextRec;
//                    /*Insert it into the queue*/
//                    mergeQueue.push(nextRecStruct);
//                }
//
//            }else{
//                /*Get next record from file into nextRec*/
//                if(GetNextRecord(*nextRec)){
//                    cout<<"Inserting a record from sorted file"<<endl;
//                    nextRecStruct->sourceName = 1;
//                    nextRecStruct->record = *nextRec;
//                    /*Insert it into the queue*/
//                    mergeQueue.push(nextRecStruct);
//                }
//
//            }
//
//        }
//
//    }else{
//        cout<<"Inside mergequeue : Inserting record from pipe"<<endl;
//
//        Record *rec = new Record;
//
//        while(outPipe->Remove(rec)){
//
//            /*insert the record into the sorted file using similar
//             function as heap. We need not create the merge file,
//             since there is no original sorted file*/
//            InsertRecordIntoFile(*rec);
//            //rec = new Record;
//        }
//
//    }
//    /*CULPRIT: Write the last page to file after insertion*/
//#ifdef DEBUG
//    cout<<"MergingRecords :::: Finished inserting records into pages.Now writing dirty page to file."<<endl;
//#endif
//    WritePageToFileIfDirty(&(this->mergePage));
//#ifdef DEBUG
//    cout<<"MergingRecords :::: Have written the pending pages to file."<<endl;
//#endif
//
//    /*CULPRIT: If the file is not closed,then changes are not reflected
//     in sorted file such as fetching filelength always returns 0*/
//    mergeFile.Close();
//    cout<<"MergeFile Length: "<<mergeFile.GetLength();
//    if(mergeFile.GetLength()!=0)
//	{
//        if(remove(metadataFile.c_str())!=0){
//            perror( "Error deleting old sorted file" );
//        }
//    }
//    if(rename(mergeFileName,metadataFile.c_str())!=0){
//        perror( "Error renaming merged file " );
//    }
//cout<<"Number of records added in this session is : "<<numRecsInPipe/2<<endl;
//    delete bigQ;
//    bigQ=NULL;
//    //    cout<<"Now old file length is: "<<file.GetLength()<<endl;
//    //    char* c= new char[metadataFile.size()+1];
//    //    std::copy(metadataFile.begin(), metadataFile.end(), c);
//    //    c[metadataFile.size()] = '\0';
//    //    file.Open(1, c);
//    //    cout<<"Now new file length after opening again is: "<<file.GetLength()<<endl;
//
//    //Delete bigQ?
//    //Reset queryordermarker?
//
//    //
//    //    //Should it be here?
//    // std::ifstream  src("mergedFile.bin", std::ios::binary);
//    // std::ofstream  dst(fileName,std::ios::binary);
//
//    //  dst << src.rdbuf();
//
//    //  remove(mergeFileName);
//
//}



void Sorted :: MergeRecordsWithSortedFile(){
    
    /*This is going to maintain the newly inputed records in the sorted order*/
#ifdef DEBUG
//    cout<<"BigQ exists????"<<endl;
#endif
    //isPageDirty=false;;
    
    /*CULPRIT: The pipe has to be shutdown before bigq construction
     If not it waits for the output pipe to remove the record and hangs*/
    //inPipe->ShutDown();
    
    //Create the instance only if it does not exist
    if(!bigQ){
        return;
        //bigQ = new BigQ(*inPipe, *outPipe, *(sortInfo->myOrder), sortInfo->runLength);
    }
    inPipe->ShutDown();
#ifdef DEBUG
//    cout<<"BigQ exists"<<endl;
#endif
    
    /*This file is created just for merging.*/
    /*Create a new file with timestamp associated.Without timestamp
     few irregularities appear*/
    std::time_t seconds = std::time(0);
    stringstream ss;
    ss<<seconds;
    string mFile="mergedFile"+ss.str();
    char* mergeFileName= new char[mFile.size()+1];
    std::copy(mFile.begin(), mFile.end(), mergeFileName);
    mergeFileName[mFile.size()] = '\0';
#ifdef DEBUG
    //cout<<"*********MergingRecords :::: Sorted File Length before opening is :  "<<file.GetLength()<<endl;
#endif
    
    //file.Open(1, "supplier.bin");
    this->mergeFile.Open(0, mergeFileName);
    this->mergePage.EmptyItOut();
    
 	file.Open(1,const_cast<char*>(metadataFile.c_str()));
    
    if(file.GetLength() != 0){
        this->file.GetPage(&this->page, 0);
        this->currentPageNumber = 1;
    }
#ifdef DEBUG
    //cout<<"*********MergingRecords :::: Sorted File Length after opening is :  "<<file.GetLength()<<endl;
#endif
    
    
    // fetch sorted records from BigQ and old-sorted-file
    // and do a 2-way merge and write into new-file (tmpFile)
    Record * pipeRec = NULL, *fileRec = NULL;
    ComparisonEngine ce;
    
    
    int fetchedFromPipe = 0, fetchedFromFile = 0;
    
    //if file on disk is empty (initially it will be) then don't fetch anything
    if(file.GetLength() == 0)
        fetchedFromFile = 0;
    //cout<<"Fetching from file is : "<<fetchedFromFile<<endl;
    
    // if (pipeRec == NULL)
    // {
    //cout<<"----------------------------Fetching record from pipe------------------------"<<endl;
    numRecsInPipe++;
    Record *temprec = new Record;
    fetchedFromPipe = outPipe->Remove(temprec);
    InsertRecordIntoFile(*temprec);
    //cout<<"??????????FetchFormPipe---------------"<<fetchedFromPipe;
    // }
    
    do{
        if (pipeRec == NULL)
        {
            //cout<<"----------------------------Fetching record from pipe------------------------"<<endl;
            pipeRec = new Record;
            fetchedFromPipe = outPipe->Remove(pipeRec);
            
        }
        if (fileRec == NULL && file.GetLength() != 0)
        {
            //cout<<"?????????????????????????Fetching record from sorted file-------------------"<<endl;
            fileRec = new Record;
            fetchedFromFile = GetNextRecord(*fileRec);
        }
        
        if (fetchedFromPipe && fetchedFromFile)
        {
            
            //cout<<"Comparing records: "<<pipeRec<<endl;
            if (ce.Compare(pipeRec, fileRec, sortInfo->myOrder) <= 0)
            {
                
                numRecsInPipe++;
                InsertRecordIntoFile(*pipeRec);
                delete pipeRec;
                pipeRec = NULL;
                //cout<<"-----------------PIpe Rec is set to null---------"<<endl;
            }
            else
            {
                
                numRecsInPipe++;
                InsertRecordIntoFile(*fileRec);
                delete fileRec;
                fileRec = NULL;
                //cout<<"FileRec is set to NULL--------------"<<endl;
            }
            
        }
        
    }while(fetchedFromPipe && fetchedFromFile);
    
    
    
    if(fetchedFromFile != 0){
        //cout<<"REMANINDE IN FILE"<<endl;
        numRecsInPipe++;
        InsertRecordIntoFile(*fileRec);
    }    if(fetchedFromPipe != 0){
        numRecsInPipe++;
        InsertRecordIntoFile(*pipeRec);
        //cout<<"REAMINDE IN PIPE"<<endl;
    }
    
    
    Record rec;
    while (outPipe->Remove(&rec))
    {
        //cout<<"OUTPIPE HAS LEFT OVER RECS"<<endl;
        numRecsInPipe++;
        InsertRecordIntoFile(rec);
    }
    
    while (file.GetLength() != 0 && GetNextRecord(rec))
    {	//cout<<"FILE HAS LEFT OVER RECS"<<endl;
        numRecsInPipe++;
        InsertRecordIntoFile(rec);
    }
    
    
    /*CULPRIT: Write the last page to file after insertion*/
    WritePageToFileIfDirty(&(this->mergePage));
    /*CULPRIT: If the file is not closed,then changes are not reflected
     in sorted file such as fetching filelength always returns 0*/
    mergeFile.Close();
    //	file=mergeFile;
    //cout<<"MergeFile Length: "<<mergeFile.GetLength()<<endl;
    //cout<<"SortedFile Length: "<<file.GetLength()<<endl;
    if(mergeFile.GetLength()!=0)	{
        if(remove(metadataFile.c_str())!=0){
            perror( "Error deleting old sorted file" );
        }
    }
    if(rename(mergeFileName,metadataFile.c_str())!=0){
        perror( "Error renaming merged file " );
    }
	//file.Close();
	//cout<<"Sorted File Length: "<<file.GetLength()<<endl;
	//file=mergeFile;
    //file.Open(1,const_cast<char*>(metadataFile.c_str()));
    //cout<<"Number of records added in this session is : "<<numRecsInPipe<<endl;
    //cout<<"Number of records added in this session is : "<<numRecsInPipe<<"  Added to pipe "<<numRecsIn<<"  get next calls"<<getnextcount<<endl;
    delete bigQ;
    bigQ=NULL;
    queryMaker = NULL;
    pageFound = false;
    
    //    cout<<"Now old file length is: "<<file.GetLength()<<endl;
    //    char* c= new char[metadataFile.size()+1];
    //    std::copy(metadataFile.begin(), metadataFile.end(), c);
    //    c[metadataFile.size()] = '\0';
    //    file.Open(1, c);
    //    cout<<"Now new file length after opening again is: "<<file.GetLength()<<endl;
    
    //Delete bigQ?
    //Reset queryordermarker?
    
    //
    //    //Should it be here?
    // std::ifstream  src("mergedFile.bin", std::ios::binary);
    // std::ofstream  dst(fileName,std::ios::binary);
    
    //  dst << src.rdbuf();
    
    //  remove(mergeFileName);
    
}

int Sorted::GetNextRecord (Record &fetchme) {
    //cout<<"GetNextRecord:Fetching next record from file"<<endl;
    int noOfPagesInFile =(int) this->file.GetLength()-1;
    /*Decrementing the length by 1 because first page of the file
     does not contain the data*/
	//cout<<"getNext: no of pages in file"<<noOfPagesInFile<<endl;
	//cout<<"currentPageNumber"<<currentPageNumber<<endl;
    
    if(currentPageNumber==0){
        this->file.GetPage(&this->page, currentPageNumber);
    }
    //	cout<< "File length is :: " << noOfPagesInFile <<endl;
    if(this->page.GetFirst (&fetchme)){
        getnextcount++;
        //cout << "Fetching first record" <<endl;
        return 1;
    }else{
        
        /*This means that we have scanned all the records on that page
         So we should move to next page
         */
        this->page.EmptyItOut();
        //this->currentPageNumber++;
        
        //		cout << "Current Page number is " <<  this->currentPageNumber << endl;
        //		cout << "No of pages in file is " <<  noOfPagesInFile << endl;
        
        if(this->currentPageNumber < noOfPagesInFile){
            this->file.GetPage(&this->page, this->currentPageNumber);
            this->currentPageNumber++;
            if(this->page.GetFirst(&fetchme)){
                getnextcount++;
                //cout<<"GetNextRecord:Got a record.returning it"<<endl;
                return 1;
            }
            else{
                //this case should never occur ideally
                return 0;
            }
        }else{
            /*This means that we have scanned the file till the end*/
            return 0;
        }
        
    }
}



int Sorted::GetNext (Record &fetchme) {
    /*For each page, perform getPage to bring the page into the buffer.
     Then perform GetFirst() to get records from the page.
     The moment GetFirst returns 0 means scanning of that page is completed.
     So fetch next page i.e. continue the loop.
     */
    queryCreated=false;
    if(!readingMode){
        MergeRecordsWithSortedFile();
        isPageDirty=false;
        readingMode=true;
    }
    pageExists=true;
    int noOfPagesInFile = (this->file.GetLength()) - 1;
    /*Decrementing the length by 1 because first page of the file
     does not contain the data*/
	//cout<<"getNext: no of pages in file"<<noOfPagesInFile<<endl;
	//cout<<"currentPageNumber"<<currentPageNumber<<endl;
    
    if(currentPageNumber==0){
        file.GetPage(&page, currentPageNumber);
    }
    //	cout<< "File length is :: " << noOfPagesInFile <<endl;
    if(this->page.GetFirst (&fetchme)){
        //cout << "Fetching first record" <<endl;
        return 1;
    }else{
        
        /*This means that we have scanned all the records on that page
         So we should move to next page
         */
        this->page.EmptyItOut();
        //this->currentPageNumber++;
        
        //		cout << "Current Page number is " <<  this->currentPageNumber << endl;
        //		cout << "No of pages in file is " <<  noOfPagesInFile << endl;
        
        if(this->currentPageNumber < noOfPagesInFile){
            this->file.GetPage(&this->page, this->currentPageNumber);
            this->currentPageNumber++;
            if(this->page.GetFirst(&fetchme)){
                return 1;
            }
            else{
                //this case should never occur ideally
                return 0;
            }
        }else{
            /*This means that we have scanned the file till the end*/
            return 0;
        }
        
    }
}

int Sorted::FetchNextFromFile (Record &fetchme) {
    /*For each page, perform getPage to bring the page into the buffer.
     Then perform GetFirst() to get records from the page.
     The moment GetFirst returns 0 means scanning of that page is completed.
     So fetch next page i.e. continue the loop.
     */
    
    //    if(!readingMode){
    //        MergeRecordsWithSortedFile();
    //        isPageDirty=false;
    //        readingMode=true;
    //    }
    
    int noOfPagesInFile = (int)(this->file.GetLength()) - 1;
    /*Decrementing the length by 1 because first page of the file
     does not contain the data*/
	//cout<<"getNext: no of pages in file"<<noOfPagesInFile<<endl;
	//cout<<"currentPageNumber"<<currentPageNumber<<endl;
    
    if(currentPageNumber==0){
        file.GetPage(&page, currentPageNumber);
    }
    //	cout<< "File length is :: " << noOfPagesInFile <<endl;
    if(this->page.GetFirst (&fetchme)){
        //cout << "Fetching first record" <<endl;
        return 1;
    }
    else{
        return 0;
    }
    //    }else{
    //
    //        /*This means that we have scanned all the records on that page
    //         So we should move to next page
    //         */
    //        this->page.EmptyItOut();
    //        //this->currentPageNumber++;
    //
    //        //		cout << "Current Page number is " <<  this->currentPageNumber << endl;
    //        //		cout << "No of pages in file is " <<  noOfPagesInFile << endl;
    //
    //        if(this->currentPageNumber < noOfPagesInFile){
    //            this->file.GetPage(&this->page, this->currentPageNumber);
    //            this->currentPageNumber++;
    //            if(this->page.GetFirst(&fetchme)){
    //                return 1;
    //            }
    //            else{
    //                //this case should never occur ideally
    //                return 0;
    //            }
    //        }else{
    //            /*This means that we have scanned the file till the end*/
    //            return 0;
    //        }
    //
    //    }
}

int Sorted::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
    ComparisonEngine comp;
    
    if(!readingMode){
        MergeRecordsWithSortedFile();
        isPageDirty=false;
        readingMode=true;
    }
    
    /*Get the first page in case no page is found*/
    if(pageExists==false){
        file.GetPage(&this->page, 0);
        this->currentPageNumber=1;
        pageExists=true;
    }
    
    if(queryCreated==false){
        
        queryMaker=cnf.CreateQueryMaker(*(sortInfo->myOrder));
        queryCreated=true;
        if(queryMaker==NULL){
//            cout<<"Query Maker is Null";
        }
    }
    
    //If query maker could not be constructed return the first record
    if(!queryMaker){
        ComparisonEngine ce;
        while (GetNext(fetchme))
        {
            if (ce.Compare(&fetchme, &literal, &cnf))
                return 1;
        }
        return 0;
        
    }
    else{
        ComparisonEngine ce;
        if(pageFound==false){
            pageFound=true;
            PageLoad(literal);
            bool recordFound=false;
            //Chenage GetNext
            while(FetchNextFromFile(fetchme))
            {
                if (ce.Compare(&literal, queryMaker, &fetchme, sortInfo->myOrder) == 0)
                {
                    if (ce.Compare(&fetchme, &literal, &cnf)){
                        return 1;
                        recordFound = true;
                        break;
                    }
                }
            }
            if(!recordFound){
                return 0;
            }
        }
        while(1){
            if(GetNext(fetchme)){
                if (ce.Compare(&literal, queryMaker, &fetchme, sortInfo->myOrder) == 0)
                {
                    if (ce.Compare(&fetchme, &literal, &cnf))
                        return 1;
                    
                }
                else
                {
                    return 0;
                }
            }
            else{
                return 0;
            }
        }
        
    }
    return 0;
}


int Sorted:: PageLoad(Record& lit){
    Page oldPage;
    int pageNumber;
    SetState(oldPage, pageNumber);
    int low = pageNumber - 1;
    int high = file.GetLength()-2;
    int foundPage = BinarySearch(low, high, lit, pageNumber-1);
    
    
    if (foundPage == -1)    // nothing found
    {
        // put file in old state, as binary search might have changed it
        GetState(oldPage, pageNumber);
        return foundPage;
    }
    else
    {
        if (foundPage > 0)
        {
            // fetch that page
            file.GetPage(&this->page, foundPage);
            this->currentPageNumber=foundPage+1;
            
            if (foundPage == (pageNumber-1))
            {
                GetState(oldPage, pageNumber);
                return foundPage;
            }
            
            Record rec;
            ComparisonEngine coe;
            
            int pageNum = foundPage;
            while (pageNum > 0)
            {
                
                if (GetNext(rec) && coe.Compare(&lit, queryMaker, &rec, sortInfo->myOrder) > 0)
                    break;
                
                
                pageNum--;
                if (pageNum == (pageNumber-1))
                    GetState(oldPage, pageNumber);
                else
                    file.GetPage(&this->page, pageNum);
                this->currentPageNumber=pageNum+1;
            }
            foundPage = pageNum;
        }
        
        return foundPage;
    }
}


int Sorted::BinarySearch(int low, int high, Record &literal, int pageNumber)
{
    // if there is only one page, then return that page
    if (low == high)
        return low;
    
    // error condition, should never reach here
    if (low > high)
        return -1;
    
    Record rec;
    ComparisonEngine ce;
    int middle = (low + high)/2;
    file.GetPage(&this->page, middle);
    this->currentPageNumber=middle+1;
    
    if (GetNext(rec))
    {
        if (ce.Compare(&literal, queryMaker, &rec, sortInfo->myOrder) == 0)
            return middle;
        // if record is greater search in first half
        else if (ce.Compare(&literal, queryMaker, &rec, sortInfo->myOrder) < 0)
        {
            if (low == middle)
                return middle;
            else
                return BinarySearch(low, middle-1, literal, pageNumber);
        }
        else // if record is smaller search in second half
            return BinarySearch(middle+1, high, literal, pageNumber);
    }
    return -1;
}


/*This function writes a page to the disk ONLY if it is dirty*/
void Sorted:: WritePageToFileIfDirty(Page* tempPage){
    
#ifdef DEBUG
//    cout<<"Writing dirty page to merge file"<<endl;
//    cout<<"********Merge file length before adding: "<<this->mergeFile.GetLength()<<endl;
#endif
    /*Write dirty page to the file*/
    if(this->isPageDirty){
        
        int whichPage = (int)this->mergeFile.GetLength()-1;
        if(whichPage==-1){
            whichPage=0;
        }
        
        //cout<<"File Status: "<<mergeFile.GetFileStatus()<<endl;
        //cout<<"Size: "<<tempPage->GetCurSize()<<endl;
        //cout<<"NumRecords: "<<tempPage->GetNumRecs()<<endl;
        this->mergeFile.AddPage(tempPage, whichPage);
        //cout<<"WriteingDirtyPage: Merge File Length after adding a page: "<<this->mergeFile.GetLength()<<endl;
        tempPage->EmptyItOut();
        this->mergePage.EmptyItOut();
        this->isPageDirty=false;
        //mergePage.EmptyItOut();
        //this->page.EmptyItOut();
    }
    
}
/*This function insert a new record into the file - heap add*/
void Sorted:: InsertRecordIntoFile(Record &rec){
    //numRecsInPipe++;
    /*This function will take care of writing the page out
     if the page becomes full before adding a record.
     
     But it will NOT take care of writing the page out once all records are added and
     still page is not full. Calling function should take care of it*/
    
	/*Check if the page is full before adding a record*/
    
    //    int pagesTotal=mergeFile.GetLength()-1;
    //    if(pagesTotal==-1){
    //        pagesTotal=0;
    //    }
    
    // cout<<"Inserting Record into File: MergeFileLength "<<mergeFile.GetLength()<<endl;
	if(!this->mergePage.Append(&rec)){
        
        
        //mergeFile.AddPage(&page,pagesTotal);
        //pagesTotal++;
        //page.EmptyItOut();
        //cout << "Page is full.Appending the record to next page."<<endl;
		WritePageToFileIfDirty(&(this->mergePage));
        
        
		/*Here, File::GetPage function can't be called. This is because
         that function returns an existing page whereas here we are adding a new page.
         But WritePageToFile... takes care of cleaning up of page instance, hence we
         just have to append the records to the new page*/
		
		/*Now load further records*/
		this->mergePage.Append(&rec);
		/*Since we added a record, we should set page as dirty*/
		this->isPageDirty = true;
        // numRecsInPipe++;
	}
    
    else{
        // numRecsInPipe++;
        //cout << "Records appended" << endl;
        /*This means record is appended so set page as dirty*/
        this->isPageDirty = true;
    }
    
    
}

void Sorted::WriteMetadata(){
    
    if(!GetMetadataFile().empty()){
        ofstream outputStream;
        outputStream.open(string(GetMetadataFile()
                                 +".metadata").c_str(),ios::trunc);
        
        outputStream<<"sorted\n";
        outputStream<<sortInfo->runLength<<"\n";
        outputStream<<sortInfo->myOrder->ToString();
        outputStream.close();
    }
    
    
}

void Sorted::SetState(Page& oldPage, int& pageNumber)
{
    char * pageBytes = new char[PAGE_SIZE];
    this->page.ToBinary(pageBytes);
    oldPage.FromBinary(pageBytes);
    pageNumber = this->currentPageNumber;
    delete pageBytes;
    pageBytes = NULL;
}

void Sorted::GetState(Page &oldPage, int &pageNumber)
{
    char* pageBytes = new char[PAGE_SIZE];
    oldPage.ToBinary(pageBytes);
    this->page.FromBinary(pageBytes);
    this->currentPageNumber = pageNumber;
    delete pageBytes;
    pageBytes = NULL;
}
