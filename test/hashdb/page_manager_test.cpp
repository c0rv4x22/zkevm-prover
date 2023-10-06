#include <unistd.h>
#include "database.hpp"
#include "hashdb_singleton.hpp"
#include "poseidon_goldilocks.hpp"
#include "scalar.hpp"
#include "utils.hpp"
#include "timer.hpp"
#include "page_manager.hpp"
#include <unordered_set>
#include <fcntl.h>
#include "page_manager_test.hpp"
#include "omp.h"
#include <random>
#include <unordered_set>


uint64_t PageManagerTest (void)
{
    PageManagerAccuracyTest();
    PageManagerPerformanceTest();
    return 0;
}

uint64_t PageManagerPerformanceTest(void){

    std::cout << "PageManagerPerformanceTest" << std::endl;
    string fileName = "benchmark_file";
    uint64_t fileSize = 1ULL<<37;
    uint64_t nFiles = 2;
    string folderName = "db";

    // Create the state manager
    double start = omp_get_wtime();
    PageManager pageManagerFile(fileName, fileSize, nFiles, folderName);
    double end = omp_get_wtime();
    std::cout << std::endl << "Time to construct the PageManager: " << end - start << " seconds" << std::endl;

    // Evaluate 20K different random positions in the range [0,numPages)
    uint64_t numPages = pageManagerFile.getNumFreePages() +2;
    uint64_t numPositions = 20000;
    uint64_t *position = (uint64_t *)malloc(numPositions * sizeof(uint64_t));
    std::random_device rd;
    std::mt19937 rng(rd());

    std::cout  << "Total number of pages: " << numPages << std::endl;
    std::cout  << "Number of pages modified: " << numPositions << std::endl << std::endl;
    
    double numGBytes = (numPositions * 4096.0) / (1024.0  * 1024.0);
    double avgTimeFistWrite = 0;
    double avgTimeSecondWrite = 0;
    double avgTimeFlush = 0;
    double avgThroughputFirstWrite = 0;
    double avgThroughputSecondWrite = 0;
    double avgThroughputFlush = 0;
    double throughput = 0;
    uint64_t numReps = 100;
    uint64_t printFreq = 10;
    
    for(uint64_t k=0; k<numReps;++k){

        //Generate randoms
        for (uint64_t i = 0; i < numPositions; i++) {
            
            position[i] = std::uniform_int_distribution<uint64_t>(0, numPages - 1)(rng);
            assert(position[i] < numPages);
        }

        //Change first value of each page
        start = omp_get_wtime();
        for (uint64_t i = 0; i < numPositions; ++i) {
            uint64_t* pageData = (uint64_t *)pageManagerFile.getPageAddress(position[i]);
            pageData[0] = position[i];
        }
        end = omp_get_wtime();
        //if(k%printFreq == 0) std::cout << "Time to change first value of each page: " << end - start << " seconds" << std::endl;
        throughput = numGBytes / (end-start);
        //if(k%printFreq == 0) std::cout << "Throughput: "<< throughput << " MBytes/s" << std::endl;
        avgThroughputFirstWrite += throughput;
        avgTimeFistWrite += end - start;

        //Change second value of each page
        start = omp_get_wtime();
        for (uint64_t i = 0; i < numPositions; ++i) {
            uint64_t* pageData = (uint64_t *)pageManagerFile.getPageAddress(position[i]);
            assert(pageData[0] == position[i] );
            pageData[1] = position[i];    
        }
        end = omp_get_wtime();
        //if(k%printFreq == 0) std::cout << "Time to change the second value of each page: " << end - start << " seconds (cache efects)" << std::endl;
        throughput = numGBytes / (end-start);
        //if(k%printFreq == 0) std::cout << "Throughput: "<< throughput << " MBytes/s" << std::endl;
        avgThroughputSecondWrite += throughput;
        avgTimeSecondWrite += end - start;

        //flushPAges
        start = omp_get_wtime();
        pageManagerFile.flushPages();
        end = omp_get_wtime();
        
        //if(k%printFreq == 0) std::cout << "Time to flush "<<numPositions<<" pages: " << end - start << " seconds" << std::endl;
        throughput = numGBytes / (end-start);
        //if(k%printFreq == 0) std::cout << "Throughput: "<< throughput << " MBytes/s" << std::endl << std::endl;
        avgThroughputFlush += throughput;
        avgTimeFlush += end - start;
        
        //Check that positions are in the file
        PageManager pageManagerFile2(fileName, fileSize, nFiles, folderName);
        for (uint64_t i = 0; i < numPositions; ++i) {
            uint64_t* pageData = (uint64_t *)pageManagerFile2.getPageAddress(position[i]);
            if(position[i] != 0){
                assert(pageData[0] == position[i] );
                assert(pageData[1] == position[i] );
            }
        }
        if(k%printFreq == 0 && k>0){

            std::cout << "Iteration: " << k << std::endl;
            std::cout << "Average time first write: " << avgTimeFistWrite/k << " seconds" << std::endl;
            std::cout << "Average throughput first write: " << avgThroughputFirstWrite/k << " MBytes/s" << std::endl;
            std::cout << "Average time second write: " << avgTimeSecondWrite/k << " seconds" << std::endl;
            std::cout << "Average throughput second write: " << avgThroughputSecondWrite/k << " MBytes/s" << std::endl;
            std::cout << "Average time flush: " << avgTimeFlush/k << " seconds" << std::endl;
            std::cout << "Average throughput flush: " << avgThroughputFlush/k << " MBytes/s" << std::endl << std::endl;
        }
    }
    return 0;
}
uint64_t PageManagerAccuracyTest (void)
{
    TimerStart(PAGE_MANAGER_TEST);

    std::cout << "PageManagerAccuracyTest" << std::endl;
    //
    // Memory version
    //
    PageManager pageManagerMem(100);
    uint64_t page1 = pageManagerMem.getFreePage();
    zkassertpermanent(page1 == 2);
    zkassertpermanent(pageManagerMem.getNumFreePages() == 97);
    uint64_t page2 = pageManagerMem.getFreePage();
    zkassertpermanent(page2 == 3);
    zkassertpermanent(pageManagerMem.getNumFreePages() == 96);
    pageManagerMem.releasePage(page1);
    zkassertpermanent(pageManagerMem.getNumFreePages() == 97);
    pageManagerMem.releasePage(page2);
    zkassertpermanent(pageManagerMem.getNumFreePages() == 98);

    unordered_set<uint64_t> pages; 
    for(int i=0; i<98;++i){
        pages.insert(pageManagerMem.getFreePage());
    }
    zkassertpermanent(pages.size() == 98);
    zkassertpermanent(pageManagerMem.getNumFreePages() == 0);

    for(int i=2; i<100;++i){
        pageManagerMem.releasePage(i);
    }
    zkassertpermanent(pageManagerMem.getNumFreePages() == 98);
    
    uint64_t page3=pageManagerMem.getFreePage();
    uint64_t * page3Data = (uint64_t *)pageManagerMem.getPageAddress(page3);
    for(uint64_t i=0; i<256;++i){
        page3Data[i] = i;
    }
    uint64_t page4 = pageManagerMem.editPage(page3);
    zkassertpermanent(page4 != page3);
    uint64_t * page4Data = (uint64_t *)pageManagerMem.getPageAddress(page4);
    for(uint64_t i=0; i<256;++i){
        zkassertpermanent(page4Data[i] == i);
    }
    zkassertpermanent(pageManagerMem.getNumFreePages() == 96);

    zkassertpermanent(pageManagerMem.editPage(page3) == page4);
    zkassertpermanent(pageManagerMem.editPage(page4) == page4);
    zkassertpermanent(pageManagerMem.editPage(0) == 1);
    zkassertpermanent(pageManagerMem.editPage(1) == 1);


    pageManagerMem.flushPages();
    zkassertpermanent(pageManagerMem.getNumFreePages() == 97);
    for(uint64_t i=0; i<256;++i){
        zkassertpermanent(page3Data[i] == 0);
    }
    for(uint64_t i=0; i<256;++i){
        zkassertpermanent(page4Data[i] == i);
    }
    //
    // File version
    //

    //generate a file with 100 pages
    const string fileName = "page_manager_test";
    const string fineNameAll = fileName + "_0.db";
    const int file_size = 4096 * 100;  

    // Create a binary file and fill it with zeros
    int fd = open(fineNameAll.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        zklog.error("Failed to open file: " + (string)strerror(errno));
        exitProcess();
    }
    //fill file with zeros
    char *buffer = (char *)calloc(file_size, sizeof(char));
    if (buffer == NULL) {
        zklog.error("Failed to allocate buffer: " + (string)strerror(errno));
        exitProcess();
    }
    ssize_t  wirten_bytes = write(fd, buffer, file_size);
    zkassertpermanent(wirten_bytes == file_size);
    close(fd);
    free(buffer);

    // Same tests than with memory version:
    PageManager pageManagerFile(fileName, file_size, 1,"");
    page1 = pageManagerFile.getFreePage();
    zkassertpermanent(page1 == 2);
    zkassertpermanent(pageManagerFile.getNumFreePages() == 97);
    page2 = pageManagerFile.getFreePage();
    zkassertpermanent(page2 == 3);
    zkassertpermanent(pageManagerFile.getNumFreePages() == 96);
    pageManagerFile.releasePage(page1);
    zkassertpermanent(pageManagerFile.getNumFreePages() == 97);
    pageManagerFile.releasePage(page2);
    zkassertpermanent(pageManagerFile.getNumFreePages() == 98);

    pages.clear(); 
    for(int i=0; i<98;++i){
        pages.insert(pageManagerFile.getFreePage());
    }
    zkassertpermanent(pages.size() == 98);
    zkassertpermanent(pageManagerFile.getNumFreePages() == 0);

    for(int i=2; i<100;++i){
        pageManagerFile.releasePage(i);
    }
    zkassertpermanent(pageManagerFile.getNumFreePages() == 98);
    
    page3=pageManagerFile.getFreePage();
    page3Data = (uint64_t *)pageManagerFile.getPageAddress(page3);
    for(uint64_t i=0; i<256;++i){
        page3Data[i] = i;
    }
    page4 = pageManagerFile.editPage(page3);
    zkassertpermanent(page4 != page3);
    page4Data = (uint64_t *)pageManagerFile.getPageAddress(page4);
    for(uint64_t i=0; i<256;++i){
        zkassertpermanent(page4Data[i] == i);
    }
    zkassertpermanent(pageManagerFile.getNumFreePages() == 96);

    zkassertpermanent(pageManagerFile.editPage(page3) == page4);
    zkassertpermanent(pageManagerFile.editPage(0) == 1);
    zkassertpermanent(pageManagerFile.editPage(1) == 1);


    pageManagerFile.flushPages();
    zkassertpermanent(pageManagerFile.getNumFreePages() == 97);
    for(uint64_t i=0; i<256;++i){
        zkassertpermanent(page3Data[i] == 0);
    }
    for(uint64_t i=0; i<256;++i){
        zkassertpermanent(page4Data[i] == i);
    }

    //Let's check persistence of file
    PageManager pageManagerFile2(fileName, file_size, 1,"");
    page3Data = (uint64_t *)pageManagerFile2.getPageAddress(page3);
    page4Data = (uint64_t *)pageManagerFile2.getPageAddress(page4);
    for(uint64_t i=0; i<256;++i){
        zkassertpermanent(page3Data[i] == 0);
    }
    for(uint64_t i=0; i<256;++i){
        zkassertpermanent(page4Data[i] == i);
    }

    //delete file
    std::remove(fileName.c_str());

    TimerStopAndLog(PAGE_MANAGER_TEST);
    return 0;   
}