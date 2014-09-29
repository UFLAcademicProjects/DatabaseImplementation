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


//Just a simple copy of Heap.cc.
//Need to change the datastructure
using namespace std;


Sorted::Sorted () {
    
}

Sorted::~Sorted(){
    
}

//Create Done
int Sorted::Create (char *f_path, void *startup) {
    cout<<"Sorted:Saving Metadata"<<endl;
    //Save metadata file path name
    metadataFile=f_path;
    sortInfo=(SortInfo*)startup;
    
    this->file.Open(0, f_path);
    
    if(this->file.GetFileStatus()<0)
        return 0;
    this->isPageDirty=false;
    this->currentPageNumber=1;
    
    
    
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
    
    cout<<"Sorted:Opening a sorted file"<<endl;
    string fileType;
    string type;
    string metadataFilePath=f_path;
    int numberOfAttributes;
    metadataFilePath=metadataFilePath+".metadata";
    ifstream readMetadata;
    
    readMetadata.open(metadataFilePath.c_str());
    if(!sortInfo){
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
    if(this->file.GetFileStatus()<0)
        return 0;
    
    cout<<"File opened successfully"<<endl;
    
    //Initialize parameters
    this->isPageDirty=false;
    this->currentPageNumber=1;
    
    return 1;
    
}

void Sorted::MoveFirst () {
    /*Since we are moving to the first page, page number is 1*/
    this->file.GetPage(&this->page, 1);
    this->currentPageNumber = 1;
}

int Sorted::Close () {
    WritePageToFileIfDirty(&this->page, this->currentPageNumber);
    return this->file.Close();
}

void Sorted :: WritePageToFileIfDirty(){
    
    /**
     Shutdown the inPipe so that no new records are added, when you are trying to insert existing records.
     */
    inPipe->ShutDown();
    
    bigQ = new BigQ(*inPipe, *outPipe, *sortInfo->myOrder, sortInfo->runLength);
    
    /**
     Consume the records and write to the file
     */
    while(outPipe->Remove(<#Record *removeMe#>)){
        
    }
    /*Free the bigQ instance*/
    delete bigQ;
    
    /*Open the inPipe*/
    inPipe->Open();
    
}

void Sorted::Add (Record &rec) {
    
    
    /**
     initialize the big queue instance
     */
    int buffSize = 100;
    inPipe = new Pipe(buffSize);
    outPipe = new Pipe (buffSize);
    
    /**
     Insert the record into inPipe*/
    inPipe->Insert(&rec);
    
    
    
    
    
    
    /**
     Now, we want to merge this record
     with the other records in the file
     */
    
    
    /*This function should check if the current page number is 0.
     If it is then it should set it to 1*/
	
	if(this->currentPageNumber == 0){
		this->currentPageNumber++;
        //		cout<< "Setting current page to " << this->currentPageNumber << " during add " << endl;
	}
	
    //	cout << "No of records in page are "<< this->page.GetNumRecs() << endl;
	
	/*This function will take care of writing the page out
     if the page becomes full before adding a record.
     
     But it will NOT take care of writing the page out once all records are added and
     still page is not full. Calling function should take care of it*/
    
	/*Check if the page is full before adding a record*/
	if(!this->page.Append(&rec)){
        
		WritePageToFileIfDirty(&this->page, this->currentPageNumber);
		cout << "Page is full.Appending the record to next page."<<endl;
		this->currentPageNumber++;
        
		/*Here, File::GetPage function can't be called. This is because
         that function returns an existing page whereas here we are adding a new page.
         But WritePageToFile... takes care of cleaning up of page instance, hence we
         just have to append the records to the new page*/
		
		/*Now load further records*/
		this->page.Append(&rec);
		/*Since we added a record, we should set page as dirty*/
		this->isPageDirty = true;
        
	}else{
		//cout << "Records appended" << endl;
		/*This means record is appended so set page as dirty*/
		this->isPageDirty = true;
	}
}

bool Sorted :: InsertIntoBigQ(){
    
    bigQ = new BigQ(*inPipe, *outPipe, *sortInfo->myOrder, sortInfo->runLength);
    
    /*This is created just for merging.*/
    char *mergeFileName="mergedFile.bin";
    File *mergeFile;
    
    
    int pagesTotal=0;
    currentPageNumber=0;
    /*Check if the main file exists*/
    if(file.GetLength() != 0){
        
        pagesTotal=file.GetLength()-1;
        mergeFile->Open(0, mergeFileName);
        Record *pipeRec, *fileRec;
        
        /*Get initial record from output pipe and file*/
        if(outPipe->Remove(pipeRec)){
            /*Insert it into the queue*/
        }
        file.GetPage(&page, currentPageNumber);
        if(page.GetFirst(fileRec)){
            /*Insert it into the queue*/
        }
        
        while(queue not empty){
            
            create a record structure copy
            insert the record into the merge file
            
            if(record is from output pipe){
                get next record(Rec, outputPipe)
            }else{
                get next record(Rec, file);
            }
            
            
        }
        
        /* This will be getNextRecord from file
         else{
            pagesTotal--;
            if(pagesTotal>0){
            currentPageNumber++;
            file.GetPage(&page,currentPageNumber);
            
            }
            
        }*/
        
        
        /*Delete merge file when job is done*/
        
        
    }else{
        Record *rec;
        
        while(outPipe->Remove(rec)){
            
            if(!page.Append(rec)){
                file.AddPage(&page,pagesTotal);
                pagesTotal++;
                page.EmptyItOut();
            }
            
            
            
        }
        
        return false;
    }
}

    int Sorted::GetNext (Record &fetchme) {
        /*For each page, perform getPage to bring the page into the buffer.
         Then perform GetFirst() to get records from the page.
         The moment GetFirst returns 0 means scanning of that page is completed.
         So fetch next page i.e. continue the loop.
         */
        
        int noOfPagesInFile = (this->file.GetLength()) - 1;
        /*Decrementing the length by 1 because first page of the file
         does not contain the data*/
        
        //	cout<< "File length is :: " << noOfPagesInFile <<endl;
        if(this->page.GetFirst (&fetchme)){
            //		cout << "Should print records" <<endl;
            return 1;
        }else{
            
            /*This means that we have scanned all the records on that page
             So we should move to next page
             */
            this->page.EmptyItOut();
            this->currentPageNumber++;
            
            //		cout << "Current Page number is " <<  this->currentPageNumber << endl;
            //		cout << "No of pages in file is " <<  noOfPagesInFile << endl;
            
            if(this->currentPageNumber < noOfPagesInFile){
                this->file.GetPage(&this->page, this->currentPageNumber);
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


    int Sorted::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
        ComparisonEngine comp;
        
        while(GetNext(fetchme)){
            if(comp.Compare(&fetchme, &literal, &cnf)) {
                return 1;
            }
        }
        return 0;
    }
    
    /*This function writes a page to the disk ONLY if it is dirty*/
    void Sorted:: WritePageToFileIfDirty(Page* page, int whichPage){
        
        
        /*Write dirty page to the file*/
        if(this->isPageDirty){
            
            this->file.AddPage(page, whichPage);
            
            this->page.EmptyItOut();
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