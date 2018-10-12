#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <math.h>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <vector>
#include <math.h>
#include <string>

using namespace std;

#define KB 1024L
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define FILESIZE (64*MB) // Just considering the first 64MB of the file
#define FILENAME "/home/aarati/workspace/CS736/ssd-measurements/test_files/test_16mb_seq"
#define RECORD_FILE "/mnt/hdd/record/garbage_collection/skipped_offsets.txt"
#define SKIPPED_OFFSET_PERCENT 25
#define BLOCK_SIZE (64*KB) // Block size is write size. Each block has 1 offset.
#define TOTAL_OFFSETS (FILESIZE/BLOCK_SIZE)

int main() {
	ofstream record(RECORD_FILE);
	vector<int> offsets;
	vector<int>::iterator it;
	int num_skipped;

	if (!record.is_open()) {
		cout << "Record file is not open" << endl;
		exit(1);
	}
	// Random seed
	srand(time(0));

	for (int i=0; i<TOTAL_OFFSETS; i++) {
		offsets.push_back(i);
	}

	random_shuffle(offsets.begin(), offsets.end());
	it = offsets.begin();
	num_skipped = (offsets.size()*SKIPPED_OFFSET_PERCENT)/100;

	// Sanity Check
	cout << "Total offsets: " << TOTAL_OFFSETS << endl;
	cout << "Offsets skipped: " << num_skipped << endl;

	for (int i=0; i<num_skipped; i++, it++) {
		record << (*it) << endl;
	}
	record.close();
	return 0;
}