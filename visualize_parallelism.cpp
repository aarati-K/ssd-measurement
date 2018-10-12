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
#define FILESIZE (64*MB)
#define FILENAME "/home/aarati/workspace/CS736/ssd-measurements/test_files/test_16mb_seq"
#define RECORD_DIRECTORY "/mnt/hdd/record/garbage_collection/after/"
#define BLOCK_FILE "/mnt/hdd/record/garbage_collection/skipped_offsets.txt"
// #define BLOCK_FILE ""

long M = (64L*KB);
long m = (4L*KB);
long I = (64*KB); // Initial request size
long O = (M/m);
long N = (FILESIZE/M);

long getTimeDiff(struct timeval startTime, struct timeval endTime) {
    return (long)((endTime.tv_sec - startTime.tv_sec)*1000000 +
        (endTime.tv_usec - startTime.tv_usec));
}

void polluteSSDCache(int fd, void* buf, int numReads=30000) {
    for (int i=0; i<numReads; i++) {
        read(fd, (char*)buf, m);
    }
    fsync(fd);
}

int main() {
	int fd, fd_pollute;
	struct timeval startTime, endTime;
	void* buf;
    void* buf_pollute;
    vector<int> blocklist;
    vector<int>::iterator it;
    int o;
    long offset;
    long requestSize;
    ifstream block_file;
    int i;

    // Random seed for rand() function
    srand(time(0));

	if (posix_memalign(&buf, 4*KB, M) != 0) {
        cout << "Posix memalign failed" << endl;
        exit(1);
    }

    if (posix_memalign(&buf_pollute, 4*KB, m) != 0) {
        cout << "Posix memalign failed" << endl;
        exit(1);
    }

    fd = open(FILENAME, O_RDWR | O_DIRECT | O_SYNC);
    if (fd < 0) {
        cout << "Failed to open file" << endl;
        exit(1);
    }

    fd_pollute = open("/home/aarati/workspace/CS736/ssd-measurements/pollute", O_RDWR | O_DIRECT);
    if  (fd_pollute < 0) {
        cout << "Failed to open pollute file" << endl;
        exit(3);
    }

    // Populate blocklist
    if (BLOCK_FILE) {
        block_file.open(BLOCK_FILE);
        if (!block_file.is_open()) {
            cout << "Could not open block file" << endl;
            exit(1);
        }
        while (block_file >> i) {
            blocklist.push_back(i);
        }
    } else {
        for (int i=0; i<N-1; i++) {
            blocklist.push_back(i);
        }
    }

    // Sanity check
    cout << "Num blocks: " << blocklist.size() << endl;

    requestSize = I;    // Initial request size
    for (o=0; o<O; o++) {
    	polluteSSDCache(fd_pollute, buf_pollute, 1000000);
    	random_shuffle(blocklist.begin(), blocklist.end());

    	ofstream record(string(RECORD_DIRECTORY) + to_string(o) + string(".txt"));
	    if (!record.is_open()) {
	    	cout << "Failed to open record file " << o << endl;
	    	exit(1);
	    }

	    for (it=blocklist.begin(); it!=blocklist.end(); it++) {
	    	offset = (*it)*M + o*m;
	    	lseek(fd, (off_t)offset, SEEK_SET);
	    	gettimeofday(&startTime, NULL);

	    	read(fd, (char*)buf, requestSize);

	    	gettimeofday(&endTime, NULL);
	    	record << getTimeDiff(startTime, endTime) << endl;
	    }

	    record.close();
    }
	return 0;
}
