#include <iostream>
#include <fstream>
#include <random>
#include <sstream>
#include <algorithm>

using namespace std;

int main(int argc, char **argv) {

	if (argc == 1) {
		cout << "Usage:\t <type> <num pages> <num accesses> <output filename>" << endl;
		cout << "\t -type is 0 for no-locality, 1 for 80-20, and 2 for looping" << endl;
	}

	int type = atoi(argv[1]);
	int num_pages = atoi(argv[2]);
	int num_access = atoi(argv[3]);
	ofstream output(argv[4]);

	srand(time(NULL));
	
	if (type == 0) {
		// No locality, so generate N integers between 0 and # of pages
		for (int i = 0; i < num_access; i++) {
			output << rand() % num_pages << endl;
		}
	}
	else if (type == 1) {
		vector<int> often; // the 20% of pages which are often used
		vector<int> scarce; // the 80% of pages which are scarcely used

		// take 20% of of the numbers 0 - #pages and set them as often used
		for (int i = 0; i < num_pages / 5; i++) {
			int value = rand() % num_pages;
			// Looks to see if the value is already selected
			bool found = false;
			for (unsigned int j = 0; j < often.size(); j++) {
				if (often[j] == value) {
					found = true;
					break;
				}
			}
			if (!found) { // if is new, push it
				often.push_back(value);
			}
			else { // if it's already there, we need to generate a new number
				i--;
			}
		}

		// take the remaining numbers and set them as scarcely used
		for (int i = 0; i < num_pages; i++) {
			// see if the value is already set as often used
			bool found = false;
			for (unsigned int j = 0; j < often.size(); j++) {
				if (often[j] == i) {
					found = true;
					break;
				}
			}
			if (!found) { // if it's not often used, push it
				scarce.push_back(i);
			}
		}
		
		// vector to hold all N accesses
		vector<int> accesses;

		// add 80% of N from the often used vector
		for (int i = 0; i < (num_access*4)/5; i++) {
			accesses.push_back(often[rand() % often.size()]);
		}

		// add the remaining 20% of N from the scarcely used vector 
		for (int i = 0; i < num_access/5; i++) {
			accesses.push_back(scarce[rand() % scarce.size()]);
		}

		// shuffle the data so that it's distributed normally, then add it to the file
		random_shuffle(accesses.begin(), accesses.end());
		for (int i = 0; i < num_access; i++) {
			output << accesses[i] << endl;
		}
	}
	else if (type == 2) {
		// just loop from 0 - #pages, then start from 0 again
		int value = 0;
		for (int i = 0; i < num_access; i++) {
			output << value << endl;
			value++;
			if (value == num_pages) value = 0;
		}
	}
	// close output
	output.close();
}

