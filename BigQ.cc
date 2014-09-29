#include "BigQ.h"

BigQ :: BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen) {
    
    // read data from in pipe sort them into runlen pages
    void *ret;
    //init data structures
    inPipe = &in;
    outPipe = &out;
    sortOrder = &sortorder;
    runLength=runlen;
    
    //Creating new thread.Generates runs
    pthread_t sortingThread;
    //Need to check this
    pthread_create(&sortingThread,NULL,&GenerateRuns,(void*)this);
    
    
    //Main thread would wait till sorting is done and the records are inserted to output
    //pipe
    //pthread_join(sortingThread, &ret);
    // finally shut down the out pipe
	//out.ShutDown ();
}



BigQ::~BigQ () {
    
    string mFile="runFile.bin";
    char* runFile= new char[mFile.size()+1];
    std::copy(mFile.begin(), mFile.end(), runFile);
    runFile[mFile.size()] = '\0';
    if(remove(runFile)){
        perror("error in removing old file");
    }
    
}

void* BigQ::GenerateRuns(void *ptr){
    
    ((BigQ*)ptr)->CreateSortedRuns();
    
    //FIX ME : Need to comment and test (San)
    return 0;
}

void* BigQ::CreateSortedRuns(){
    
	//cout << "Inside create sorted runs" <<endl;
    fileName="runFile.bin"; //probably requires timeStamp;
    
    pageCountPerRun=0;//Stores the page count per run
    pagesTotal=0;//Total number of pages in a file
    Record *record=new Record;
    Record *recordCopy;
    Page pagesForRunlength;
    //Need to check syntax
    priority_queue<Record *, vector<Record *>, CompareRec> queue(sortOrder);
    file.Open(0,fileName);
    
    
    //cout<<"BigQ :Removing from the inputpipe"<<endl;
    
	if(inPipe){
        //cout<<"Input pipe object exist"<<endl;
    }
    while(inPipe->Remove(record)){
        recordCopy=new Record;
        //Copy the record since appending would consume the record
        
        recordCopy->Copy(record);
        
        queue.push(recordCopy);
        
        
        if(!pagesForRunlength.Append(record)){
//            cout << "Page is completed in the run"<< endl;
            pageCountPerRun++;
            //pageCountPerRun is required to compare with runlength
            if(runLength==pageCountPerRun){
               // cout << "One run is complete" <<endl;
                Page page;
                int pageCount=0;
                //Store the page count per run
                //runIndices.push_back(pageCountPerRun);
                //Store the sort records in page
                
                while(queue.size()!=1){
                    if(!page.Append(queue.top())){
                        file.AddPage(&page,pagesTotal);
                        pagesTotal++;
                        pageCount++;
                        //cout << pagesTotal << "th page is written to file part 1" << endl;
                        
                        page.EmptyItOut();
                        page.Append(queue.top());
                    }
                    queue.pop();
                    
                }
                //Add the last page to file
                file.AddPage(&page,pagesTotal);
                pagesTotal++;
                pageCount++;
                //cout << pagesTotal << "th page is written to file part 2" << endl;
                runIndices.push_back(pageCount);
                page.EmptyItOut();
                queue.empty();
                //Reset the count for next run
                pageCountPerRun=0;
                
            }
            pagesForRunlength.EmptyItOut();
            pagesForRunlength.Append(record);
            
        }
        
    }
	//cout<<"Deleting record"<<endl;
    //Delete record
    delete record;
    //cout<<"BigQ : deleting the record object"<<endl;
    //If queue still contains record,append it to a new run
    if(queue.size()!=0){
//        cout<<"Run lenghth HAS NOT reached.Queue contains more records"<<endl;
        Page page;
        pageCountPerRun=0;
        while(queue.size()!=0){
            if(!page.Append(queue.top())){
                file.AddPage(&page,pagesTotal);
                pageCountPerRun++;
                pagesTotal++;
                //cout << pagesTotal << "th page is written to file part 3" << endl;
                
                page.EmptyItOut();
                page.Append(queue.top());
            }
            queue.pop();
        }
        //Add the last page to file
        file.AddPage(&page,pagesTotal);
        pageCountPerRun++;
        pagesTotal++;
        //cout << pagesTotal << "th page is written to file part 4" << endl;
        //file.AddPage(&page,pagesTotal);
        page.EmptyItOut();
        queue.empty();
        
    }
	
    runIndices.push_back(pageCountPerRun);
    for (int i = 0; i < runIndices.size(); i++) {
        //cout << "runIndex[" << i << "]: " << runIndices[i] << "\n";
    }
    //cout<<"File length - 1= "<<file.GetLength()-1;
    file.Close();
    
    // construct priority queue over sorted runs and dump sorted data
 	// into the out pipe
    MergeRuns();
    outPipe->ShutDown();


}
int BigQ::MergeRuns(){
    
    file.Open(1, fileName);
    //RecordStruct *record = new RecordStruct;
    int numOfRuns = runIndices.size();
    Run *runArray = new Run[numOfRuns];
    priority_queue<RecordStruct *, vector<RecordStruct *>, ComparePQ> mergeQueue(sortOrder);
    runArray[0].currentPageNumber = 0;
    runArray[0].totalPages = runIndices[0];
    file.GetPage(&(runArray[0].currentPage), runArray[0].currentPageNumber);
    if(numOfRuns>1){
        /*Initialize Run data structure*/
        for(int i = 1; i< numOfRuns; i++){
            //runArray[i].currentPageNumber = (runIndices[i-1])*(i);
            runArray[i].currentPageNumber = runIndices[i-1];
            //cout << "Initially we push " << runArray[i].currentPageNumber << " th page for run number " << i << endl;
            runArray[i].totalPages = runIndices[i];
            file.GetPage(&(runArray[i].currentPage), runArray[i].currentPageNumber);
            runIndices[i]+=runIndices[i-1];
        }
    }
    //cout<<"First set of records: "<<endl;
    //Fetch the record and store it
    for(int runCount=0;runCount<numOfRuns;runCount++){
        //Culprit
        RecordStruct *record = new RecordStruct;
        runArray[runCount].currentPage.GetFirst(&(record->record));
        record->run_num=runCount;
        //Insert the record into priority queue
        
        //cout<<"record : "<<record<<"RunNum: "<<runCount;
        mergeQueue.push(record);
        
    }
    //    cout<<"Finished fetching the first record from each page.Stored in the queue."<<endl;
    
    while(!mergeQueue.empty()){
        //      cout<<"MergeRuns:Pulling each record and adding it to output pipe"<<endl;
        //Fetch the record from priority queue
        RecordStruct *candidate=new RecordStruct;
        candidate=mergeQueue.top();
        int runNum=candidate->run_num;
        //cout<<&candidate->record<<endl;
        //cout<<"MergeRuns:Removed a record from priority queue with run number"<<runNum<<"and "<<runArray[runNum].currentPageNumber<<endl;
        //Delete the record from priority queue
        mergeQueue.pop();
        //Insert each record into output queue
        outPipe->Insert(&(candidate->record));
        //cout<<"MergeRuns:Inserted one record"<<endl;
        //cout<<"-------------------------------"<<endl;
        
        //Get the next record from the run
        RecordStruct *nextRecord=new RecordStruct;
        if(runArray[runNum].currentPage.GetFirst(&(nextRecord->record))){
            //cout<<"GetNext:Fetched next record"<<endl;
            nextRecord->run_num=runNum;
            mergeQueue.push(nextRecord);
        }
        
        //Check for more pages in the run
        else{
            //Decrement the number of pages in the run
            //cout<<"GetNext:Could not fetch a record.decrementing page number"<<endl;
            runArray[runNum].totalPages--;
            //cout<<"Now total pages: "<< runArray[runNum].totalPages<<"for run number is: "<<runNum<<endl;
            if(runArray[runNum].totalPages>0){
                //cout<<"GetNext:Additional Page exists"<<endl;
                //Increment the page number
                runArray[runNum].currentPageNumber++;
                //Get the next Page and set as the current page
                file.GetPage(&(runArray[runNum].currentPage), runArray[runNum].currentPageNumber);
                //Return the next record
                runArray[runNum].currentPage.GetFirst(&(nextRecord->record));
                nextRecord->run_num=runNum;
                mergeQueue.push(nextRecord);
                
                //status=1;
                //cout<<"GetNext:Fetched Record from next page"<<endl;
            }
            else{
                //cout<<"GetNext:No additional Page exists for run no: "<<runNum<<endl;
                //cout << "Run No :  " << runNum <<" exhausted" <<endl;
                //delete nextRecord;
                numOfRuns--;
                //status=0;
            }
        }
        
        //        if(!GetNextRecordFromRun(runNum,runArray,nextCandidate,mergeQueue))
        //        {
        //            cout<<"MergeRuns : Run number exhausted in GetNext.Decrementing numOfRuns"<<endl;
        //            numOfRuns--;
        //            //Delete the run if exhausted
        //            //delete (&runArray[runNum]);
        //        }
        //        else{
        //            //Add the next record to queue
        //            cout<<"MergeRuns : pushing record to queue."<<endl;
        //            //mergeQueue.push(nextCandidate);
        //            //nextCandidate->run_num=runNum;
        //            cout<<"MergeRuns:Record page number is: "<<runArray[runNum].currentPageNumber<<" from runNumber: "<<runNum<<endl;
        //        }
        //
        
    }
    // Check if all runs exhausted, exit
    file.Close();
    //delete &runIndices;
    //Enable this for debug
    //cout<<"totPages: "<<pagesTotal;
}


bool BigQ::GetNextRecordFromRun(int currentRunNumber,Run *array,RecordStruct *nextRecord,priority_queue<RecordStruct *, vector<RecordStruct *>, ComparePQ> mergeQueue){
    //RecordStruct *nextRecord=new RecordStruct;
    int status;
    if(array[currentRunNumber].currentPage.GetFirst(&(nextRecord->record))){
        //cout<<"GetNext:Fetched next record"<<endl;
        mergeQueue.push(nextRecord);
        nextRecord->run_num=currentRunNumber;
        status=1;
    }
    //Check for more pages in the run
    else{
        //Decrement the number of pages in the run
        //cout<<"GetNext:Could not fetch a record.decrementing page number"<<endl;
        array[currentRunNumber].totalPages--;
        //cout<<"Now total pages: "<< array[currentRunNumber].totalPages<<"for run number is: "<<currentRunNumber<<endl;
        if(array[currentRunNumber].totalPages>0){
            //cout<<"GetNext:Additional Page exists"<<endl;
            //Increment the page number
            array[currentRunNumber].currentPageNumber++;
            //Get the next Page and set as the current page
            file.GetPage(&(array[currentRunNumber].currentPage), array[currentRunNumber].currentPageNumber);
            //Return the next record
            array[currentRunNumber].currentPage.GetFirst(&(nextRecord->record));
            mergeQueue.push(nextRecord);
            nextRecord->run_num=currentRunNumber;
            status=1;
            //cout<<"GetNext:Fetched Record from next page"<<endl;
        }
        else{
            //cout<<"GetNext:No additional Page exists for run no: "<<currentRunNumber<<endl;
            //cout << "Run No :  " << currentRunNumber <<" exhausted" <<endl;
            //delete nextRecord;
            status=0;
        }
    }
    
    return status;
    
}
