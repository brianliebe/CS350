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

void print_with_formatting(int pageFaults, string type, int blocks) {
	int hitRate = (int)((10000 - pageFaults) / (float)10000 * 100);        
	// cout << "Page faults (" << type << " " << blocks << " blocks):\t " << pageFaults << "\t Hit rate: " << hitRate << "%" << endl;
	cout << hitRate << endl;
}

void optimal(char *filename, int blocks){
        int pageFaults = 0;
        vector<int> accesses;
        vector<int> cache;
        ifstream input(filename);

        // read the file, add ints to a vector
        string value;
        while (getline(input, value)) {
                accesses.push_back(atoi(value.c_str()));
        }
        reverse(accesses.begin(), accesses.end()); // reverse it so we pop the first value

        while (accesses.size()) {
                int page = accesses.at(accesses.size() - 1);
                accesses.pop_back();

                // try to find the value in the cache
                bool found_in_cache = false;
                for (unsigned int i = 0; i < cache.size(); i++) {
                        if (cache[i] == page) {
                                found_in_cache = true;
                                break;
                        }
                }

                // if it's not there, we have to add it
                if (!found_in_cache) {
                        pageFaults++;

                        // if the cache isn't full, just push it
                        if ((int)cache.size() < blocks) {
                                cache.push_back(page);
                        }

                        // replace the one that will be accessed furthest in the future
                        else {
                                int distance_to_next_match[cache.size()]; // save these indexes in an array
                                for (unsigned int i = 0; i < cache.size(); i++) {
                                        distance_to_next_match[i] = accesses.size(); // set as max distance (aka it's never found again)
                                        for (int j = (int)accesses.size() - 1; j >= 0; j--) { // move backwards through the accesses
                                                if (accesses[j] == cache[i]) {
                                                        distance_to_next_match[i] = (accesses.size() - j); // set the index we have a match at
                                                        break;
                                                } 
                                        }
                                }
                                // here, just find the furthest distance of all our cache entries
                                int furthest_index = 0;
                                int furthest_distance = distance_to_next_match[0];
                                for (unsigned int i = 0; i < cache.size(); i++) {
                                        if (distance_to_next_match[i] > furthest_distance) {
                                                furthest_distance =  distance_to_next_match[i];
                                                furthest_index = i;
                                        }
                                }
                                cache.at(furthest_index) = page;
                        }
                }
        }
	print_with_formatting(pageFaults, "OPT", blocks);
}

void least_recently_used(char *filename, int blocks) {
        int pageFaults = 0;
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
                                if ((int)i != index_of_found_page && cache[i].second < value_of_found_page) {
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
                        if ((int)cache.size() < blocks) {
                                // if the cache wasn't full yet, we just add it
                                cache.push_back(make_pair(page, 0));
                        }
                }
        }
	print_with_formatting(pageFaults, "LRU", blocks);
}

void fifo(char *filename, int blocks){
        int pageFaults = 0;
        vector<int> accesses;
	vector<int> cache;
        ifstream input(filename);

        // read the file, add ints to a vector
        string value;
        while (getline(input, value)) {
                accesses.push_back(atoi(value.c_str()));
        }
        reverse(accesses.begin(), accesses.end()); // reverse it so we pop the first value

        while(accesses.size()){
		// get the last (first) value from the vector
		int page = accesses.at(accesses.size() - 1);
		accesses.pop_back();

		// try to find it in the cache
		bool found_in_cache = false;
		for (unsigned int j = 0; j < cache.size(); j++) {
			if (page == cache[j]) {
				found_in_cache = true;
				break;
			}
		}
		if (!found_in_cache) {
			// if it's not found, we need to add it
			pageFaults++;			
			if ((int)cache.size() < blocks) {
				// if the cache isn't full, add it
				cache.push_back(page);
			}
			else {
				// if the cache is full, remove the first value (oldest), and push the new page
				cache.erase(cache.begin() + 0);
				cache.push_back(page);
			}
		}
        }
	print_with_formatting(pageFaults, "FIFO", blocks);
}

void random_policy(char *filename, int blocks) {
        int pageFaults = 0;
        vector<int> accesses;
        vector<int> cache;
        ifstream input(filename);

        srand(time(NULL));

        // read the file, add ints to a vector
        string value;
        while (getline(input, value)) {
                accesses.push_back(atoi(value.c_str()));
        }
        reverse(accesses.begin(), accesses.end()); // reverse it so we pop the first value

        while (accesses.size()) {
                int page = accesses.at(accesses.size() - 1);
                accesses.pop_back();

                // check to see if the page is already in the cache
                bool found_in_cache = false;
                for (unsigned int i = 0; i < cache.size(); i++) {
                        if (cache[i] == page) {
                                // if it is, we're done
                                found_in_cache = true;
                                break;
                        }
                }
                if (!found_in_cache) {
                        // if it's not in the cache we need to add it
                        pageFaults++;
                        if ((int)cache.size() < blocks) {
                                cache.push_back(page); // cache wasn't full, just push
                        }
                        else {
                                int random_index = rand() % blocks; // randomly replace something
                                cache[random_index] = page;
                        }
                }
        }
	print_with_formatting(pageFaults, "RAND", blocks);
}

void clock_policy(char *filename, int blocks) {
        int pageFaults = 0;
        vector<int> accesses;
        ifstream input(filename);
	int clockValue = 0;

        // read the file, add ints to a vector
        string value;
        while (getline(input, value)) {
                accesses.push_back(atoi(value.c_str()));
        }
        reverse(accesses.begin(), accesses.end()); // reverse it so we pop the first value

	vector<pair<int, bool> > cache;

	while (accesses.size()) {
		int page = accesses.at(accesses.size() - 1);
		accesses.pop_back();

		bool found_in_cache = false;
		for (unsigned int i = 0; i < cache.size(); i++) {
			if (page == cache[i].first) {
				found_in_cache = true;
				cache[i].second = true;
				break;
			}
		}

		if (!found_in_cache) {
			pageFaults++;
			if((int) cache.size() < blocks){
				cache.push_back(make_pair(page, true));
				clockValue++;
			}
			//BRAND NEW ENTRY AND NEEDS REPLACEMENT
			else{

				while (true) {
					if (clockValue == blocks) clockValue = 0;
					if (cache[clockValue].second == false) {
						cache[clockValue].first = page;
						cache[clockValue].second = true;
						clockValue++;
						break;
					}
					else {
						cache[clockValue].second = false;
					}
					clockValue++;
				}
			}
		}
	}
	print_with_formatting(pageFaults, "CLCK", blocks);
}

int main(int, char* argv[]){
        //Optimal Replacement Policy
	cout << "Optimal Replacement Policy" << endl;
        for (int i = 5; i <= 100; i += 5) {
                optimal(argv[1], i);
        }
        // LRU Replacement Policy
	cout << "\nLeast-recently-used Replacement Policy" << endl;
        for (int i = 5; i <= 100; i += 5) {
                least_recently_used(argv[1], i);
        }
	// First-in-first-out Replacement Policy
	cout << "\nFIFO Replacement Policy" << endl;
	for (int i = 5; i <= 100; i += 5) {
                fifo(argv[1], i);
        }
        // Random Replacement Policy
	cout << "\nRandom Replacement Policy" << endl;
        for (int i = 5; i <= 100; i += 5) {
                random_policy(argv[1], i);
        }
	// Clock Replacement Policy
	cout << "\nClock Replacement Policy" << endl;
        for (int i = 5; i <= 100; i += 5) {
                clock_policy(argv[1], i);
        }
        return 0;
}
