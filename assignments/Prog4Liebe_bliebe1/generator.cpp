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
		for (int i = 0; i < num_access; i++) {
			output << rand() % num_pages << endl;
		}
	}
	else if (type == 1) {
		vector<int> often;
		vector<int> scarce;

		for (int i = 0; i < num_pages / 5; i++) {
			int value = rand() % num_pages;
			bool found = false;
			for (unsigned int j = 0; j < often.size(); j++) {
				if (often[j] == value) {
					found = true;
					break;
				}
			}
			if (!found) {
				often.push_back(value);
			}
			else {
				i--;
			}
		}
		for (int i = 0; i < num_pages; i++) {
			bool found = false;
			for (unsigned int j = 0; j < often.size(); j++) {
				if (often[j] == i) {
					found = true;
					break;
				}
			}
			if (!found) {
				scarce.push_back(i);
			}
		}
		vector<int> accesses;
		for (unsigned int i = 0; i < (num_access*4)/5; i++) {
			accesses.push_back(often[rand() % often.size()]);
		}
		for (unsigned int i = 0; i < num_access/5; i++) {
			accesses.push_back(scarce[rand() % scarce.size()]);
		}
		random_shuffle(accesses.begin(), accesses.end());
		for (int i = 0; i < num_access; i++) {
			output << accesses[i] << endl;
		}
	}
	else if (type == 2) {
		int value = 0;
		for (int i = 0; i < num_access; i++) {
			output << value << endl;
			value++;
			if (value == num_pages) value = 0;
		}
	}
	output.close();
}

