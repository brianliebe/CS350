//Optimal Replacement Policy
//Should lead to the least misses
//So it evicts the page that it used last


//Least Recently USed
//Evicts a page that has been used least recently
//Based on frequency and recency
//*Upon each page access, update data structure to move this page to the front




//First in First Out
//Evicts the page that was first in



//Random
//Should be a little better than FIFO
//Evicts a random page

//Clock
//Whenever a page is referenced, the use bit of the page is set to 1
//Imagine all pages in a circular list
//Clock hand points to a particular page (doesn't matter which)
//If a replacement is needed, check if the pointed page has a use bit of 1 or 0
//If 1, don't replace ---
//Set the use bit of that page to 0
//Clock hand increments to the next page
//Continue until it finds a page with a 0 bit

//Modification to clock algorithm:
//If page has been modified (dirty), it must be written back to disk to evict it
//Have a modified bit that is set anytime a page is written
//Clock algorithm checks for both use bit and modified bit
/* ---------------------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

using namespace std;

int fifo(int pages[], int length){
        int pageFaults = 0;

        int blocks = 5;

        int temp[blocks];

        //This is to initialize the temp array which will basically be used to compare
        //pages numbers against the pages[] array
        for(int i = 0; i < blocks; i++){
                temp[blocks] = -1;
        }

        for(int i = 0; i < length; i++){
                //This variable indicates if the page number entry is new or not
                int j = 0;
                for(int m = 0; m < blocks; m++){
                        //If the first page number is equal to the first slot in the block
                        //decrease the amount of pageFault so that you can just increment resulting
                        //in a net of zero pageFaults
                        if(pages[i] == temp[m]){
                                j++;
                                pageFaults--;
                        }
                }
                pageFaults++;
                //If there is a page fault, that means that there will be a new page number entry
                //thus temp[i] will contain that new page
                //j must equal 0 because this is a new entry and requires a new space
                //So pageFaults gives an idea of how many new entries there are
                //If there are still more block space available, then the page number will
                //go to the block spot as normal
                if((pageFaults <= blocks) && (j == 0)){
                        temp[i] = pages[i];
                }
                //Otherwise if there are more entries than there are blocks, this will move the
                //first page out and put the new page in
                else if(j == 0){
                        temp[((pageFaults - 1) % blocks)]= pages[i];
                }
        }
        cout << "Page faults (FIFO):\t " << pageFaults << endl;
        return 0;


}

int optimal(int pages[], int length){


}

int least_recently_used(char *filename) {
        int pageFaults = 0;
        int blocks = 5;
        vector<int> accesses;
        vector<pair<int, int>> cache; // <page, time>
        ifstream input(filename);

        // read the file, add ints to a vector
        string value;
        while (getline(input, value)) {
                accesses.push_back(atoi(value.c_str()));
        }
        reverse(accesses.begin(), accesses.end()); // reverse it so we pop the first value

        while (accesses.size()) { // pop one off at a time
                int page = accesses.at(accesses.size() - 1); // get the last access
                accesses.pop_back();

                int index_of_found_page = -1;
                int value_of_found_page = -1;

                // search the cache for the page to already be in there
                for (unsigned int i = 0; i < cache.size(); i++) {
                        if (cache[i].first == page) {
                                // it's in there, so save it's old time and make it's new time 0
                                value_of_found_page = cache[i].second;
                                cache[i].second = 0;
                                index_of_found_page = i;
                                break;
                        }
                }

                // if we found the page in the cache, increment all other page/time pairs by one, unless it was already less recently used
                if (index_of_found_page != -1) {
                        for (unsigned int i = 0; i < cache.size(); i++) {
                                if (i != index_of_found_page && cache[i].second < value_of_found_page) {
                                        cache[i].second++;
                                }
                        }
                }

                // if it's not in the cache, evict the oldest one and add the new one
                else {
                        pageFaults++;
                        for (unsigned int i = 0; i < cache.size(); i++) {
                                cache[i].second++; // all times will inc. by 1
                                if (cache[i].second == blocks) {
                                        // if this index has a time == blocks, that means it's the oldest, so we replace it
                                        cache[i].first = page;
                                        cache[i].second = 0;
                                }
                        }
                        if (cache.size() < blocks) {
                                // if the cache wasn't full yet, we just add it
                                cache.push_back(make_pair(page, 0));
                        }
                }
        }
        cout << "Page faults (LRU):\t " << pageFaults << endl;
}


int main(int argc, char* argv[]){
/*
        char *fileName = "pages.txt";
        FILE *f1;
        int *pages = malloc(sizeof(int) * 100);
        int numberOfIntegers = 100;

        //Read the file
        int a;
        f1 = fopen(fileName, "r");
        fscanf(f1, "%d", &a);
        fclose(f1);

        numberOfIntegers = a;

        int k = 0;
        int number;

        while(k < numberOfIntegers){
                fscanf(f1, "%d", &number);
                pages[k] = number;
                k++;
        }

        fclose(f1);
*/

        int pages[7];
        pages[0] = 1;
        pages[1] = 2;
        pages[2] = 3;
        pages[3] = 4;
        pages[4] = 5;
        pages[5] = 6;
        pages[6] = 7;

        //Length of the page array so that I can pass it on to the algorithm functions
        int length = (sizeof(pages)/sizeof(pages[0]));
        //First In First Out Replacement Policy
        fifo(pages, length);
        //Optimal Replacement Policy
        optimal(pages, length);
        // LRU Replacement Policy
        least_recently_used(argv[1]);




        //
        return 0;
}
