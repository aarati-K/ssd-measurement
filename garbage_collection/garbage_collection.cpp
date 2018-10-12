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
#define FILESIZE (1*GB)
#define FILENAME "/home/aarati/workspace/CS736/ssd-measurements/test_files/test_16kb_seq"
#define RECORD_FILE "/mnt/hdd/record/garbage_collection/write_bw.txt"
#define WRITE_PERCENTAGE 50 // Not important

long R = (16*KB);
long N = (FILESIZE/R);

long getTimeDiff(struct timeval startTime, struct timeval endTime) {
    return (long)((endTime.tv_sec - startTime.tv_sec)*1000000 +
        (endTime.tv_usec - startTime.tv_usec));
}

int main() {
	int fd;
	struct timeval startTime, endTime;
	void* buf;
	long offset;
	int o;
	int trials=100000;
	int i;
	long j;
	ofstream record(RECORD_FILE);

    if (!record.is_open()) {
        cout << "Could not open record file" << endl;
        exit(1);
    }

	// Random seed for rand() function
	srand(time(0));

	if (posix_memalign(&buf, 4*KB, R) != 0) {
        cout << "Posix memalign failed" << endl;
        exit(1);
    }

    fd = open(FILENAME, O_RDWR | O_DIRECT | O_SYNC);
    if (fd < 0) {
    	cout << "Failed to open file" << endl;
        exit(1); 
    }

    for (i=0, j=0; i<trials; j++) {
    	if (rand() % 100 > WRITE_PERCENTAGE) {
    		continue;
    	}

    	offset = (j%N)*R;
    	lseek(fd, (off_t)offset, SEEK_SET);
    	gettimeofday(&startTime, NULL);
    	write(fd, (char*)buf, R);
    	gettimeofday(&endTime, NULL);
    	record << getTimeDiff(startTime, endTime) << endl;
    	i++;
    }
    close(fd);
    record.close();
	return 0;
}