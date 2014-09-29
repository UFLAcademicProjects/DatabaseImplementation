#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"
#include "Heap.h"
#include <iostream>
#include <fstream>

using namespace std;


Heap::Heap () {

 
}

Heap::~Heap(){

}
//Creates the file
int Heap::Create (char *f_path, void *startup) {
    cout<<"Heap:Saving metadata"<<endl;
    //Save metadata file path name
    metadataFile=f_path;
    this->file.Open(0, f_path);
    if(this->file.GetFileStatus()<0)
        return 0;
    this->isPageDirty=false;
    this->currentPageNumber=1;
    //Store metadata
    WriteMetadata();
    return 1;
}

//Load the schema
void Heap::Load (Schema &f_schema, char *loadpath) {
    //Open the file
    FILE *fileToLoad = fopen(loadpath, "r");
    if(!fileToLoad){
        cout<< "Can't open file name :" + string(loadpath);
    }
    Record record;
    //Load the schema and append each record
    while(record.SuckNextRecord(&f_schema,fileToLoad)==1){
        Add(record);
    }
}

//Open the file
int Heap::Open (char *f_path) {
    
    this->file.Open(1, f_path);
    if(this->file.GetFileStatus()<0)
        return 0;
    
    cout<<"File opened successfully"<<endl;
    
    //Initialize parameters
    this->isPageDirty=false;
    this->currentPageNumber=1;
    
    return 1;
    
}
//Moves the pointer to point to the first record
void Heap::MoveFirst () {
    /*Since we are moving to the first page, page number is 1*/
    this->file.GetPage(&this->page, 1);
    this->currentPageNumber = 1;
}

int Heap::Close () {
    WritePageToFileIfDirty(&this->page, this->currentPageNumber);
    return this->file.Close();
}

/*This function is used to add records to the page and in turn to the database file*/
void Heap::Add (Record &rec) {
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
//Gets the next record
int Heap::GetNext (Record &fetchme) {
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
//Applies CNF and gets the next record
int Heap::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
    ComparisonEngine comp;
    
    while(GetNext(fetchme)){
        if(comp.Compare(&fetchme, &literal, &cnf)) {
            return 1;
        }
    }
    return 0;
}

/*This function writes a page to the disk ONLY if it is dirty*/
void Heap:: WritePageToFileIfDirty(Page* page, int whichPage){
    
    
    /*Write dirty page to the file*/
    if(this->isPageDirty){
        
        this->file.AddPage(page, whichPage);
        
        this->page.EmptyItOut();
    }
    
}
//Stores the file type
void Heap::WriteMetadata(){

    if(!GetMetadataFile().empty()){
        ofstream outputStream;
        outputStream.open(string(GetMetadataFile()
                                    +".metadata").c_str(),ios::trunc);
        outputStream<<"heap";
        outputStream.close();
    }
    
}