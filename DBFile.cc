#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"


#include <iostream>
#include <fstream>

// stub file .. replace it with your own DBFile.cc
using namespace std;


DBFile::DBFile (): myInternalVar(NULL){
    
    
    
}
DBFile::~DBFile()
{
    delete myInternalVar;
    myInternalVar = NULL;
}


int DBFile::Create (char *f_path, fType f_type, void *startup) {
    //Phase-2
    //Create a base db file object based on the file type
    if(f_type==heap){
        myInternalVar=new Heap();
    }
    else if(f_type==sorted){
        myInternalVar=new Sorted();
    }
    
    //Check if myInternalVar exists
        if(!myInternalVar){
            cout<<"Not enough memory.Exit now."<<endl;
            //exit(1);
        }
    
    return myInternalVar->Create(f_path,startup);

}

void DBFile::Load (Schema &f_schema, char *loadpath) {
    
   myInternalVar->Load(f_schema,loadpath);
    
}

int DBFile::Open (char *f_path) {
    
    //Phase-2
    cout<<"DBFile: Opening the file"<<endl;
    if(!myInternalVar){
        struct stat fileBuffer;
        int fileStatus;
        string metadataFilePath=f_path;
        metadataFilePath=metadataFilePath+".metadata";
        fileStatus=stat(metadataFilePath.c_str(), &fileBuffer);
        //Check if file exists
        if(fileStatus!=0){
        cout<<"File: "<<metadataFilePath<<" does not exist"<<endl;
            return 0;
        }
        ifstream inputFile;
        inputFile.open(metadataFilePath.c_str());
        string line;
        //Get the file type from metadata
        while(!inputFile.eof()){
            getline(inputFile,line);
            if(line.compare("heap")==0){
                myInternalVar=new Heap();
                cout<<"DBFile: Created a Heap instance"<<endl;
                break;
            }
            else if(line.compare("sorted")==0){
                myInternalVar=new Sorted();
                cout<<"DBFile: Created a Sorted instance"<<endl;
                break;
            }
        }
    }
    return myInternalVar->Open(f_path);

    
    
}

void DBFile::MoveFirst () {
    //Phase-2
    myInternalVar->MoveFirst();
    
}

int DBFile::Close () {
    //Phase-2
    return myInternalVar->Close();
}

//void DBFile::initializePage(){
//
//	/*Since page is created just now, it does not represent any page of a file in the memory.
//	Hence currentPageNumber is 0.
//	Also currently nothing is written on the page, hence it is not dirty*/
//	this->currentPageNumber = 0;
//	this->isPageDirty = false;
//
//	cout<<"Buffer instance is initialized successfully."<<endl;
//
//}
//

/*This function is used to add records to the page and in turn to the database file*/
void DBFile::Add (Record &rec) {
	//Phase-2
    myInternalVar->Add(rec);
    
}


///*This function writes a page to the disk ONLY if it is dirty*/
//void DBFile :: WritePageToFileIfDirty(Page* page, int whichPage){
//    
//    /*Write dirty page to the file*/
//    if(this->isPageDirty){
//        
//        this->file.AddPage(page, whichPage);
//        
//        this->page.EmptyItOut();
//    }
//    
//    
//}


int DBFile::GetNext (Record &fetchme) {
    //Phase-2
    return myInternalVar->GetNext(fetchme);
}

                
                
int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
    //Phase-2
    return myInternalVar->GetNext(fetchme,cnf,literal);
}
