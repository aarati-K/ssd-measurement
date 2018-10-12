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
#include <set>

using namespace std;

#define KB 1024L
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define FILESIZE (64*MB)
#define FILENAME "/home/aarati/workspace/CS736/ssd-measurements/test_files/test_16mb_seq"
#define RECORD_FILE "/mnt/hdd/record/garbage_collection/latency_16kb_128_write.txt"
#define SKIPPED_BLOCKS "/mnt/hdd/record/garbage_collection/skipped_offsets.txt"

long R = 64*KB;
long r = 8;
long B = r*R;
int dist = 7; // adjacent blocks are at distance 0
long N = FILESIZE/B;

long W = 16*KB;
long w = 4;
long num_write = 128;
long block_size = w*W;
long num_blocks = FILESIZE/block_size;
long total_writes = 500000;

long getTimeDiff(struct timeval startTime, struct timeval endTime) {
    return (long)((endTime.tv_sec - startTime.tv_sec)*1000000 +
        (endTime.tv_usec - startTime.tv_usec));
}

int main() {
	int fd;
	struct timeval startTime, endTime;
	void* buf;
	long offset;
	std::vector<int> blocks;
    std::set<int> skipped_blocks;
	std::vector<int>::iterator it;
	long offset1, offset2;
	int i, j;
	ofstream record(RECORD_FILE);
    ifstream skipped_file(SKIPPED_BLOCKS);

    if (!record.is_open() || !skipped_file.is_open()) {
        cout << "could not open stream files" << endl;
        exit(1);
    }

	// Random seed
	srand(time(0));

	fd = open(FILENAME, O_RDWR | O_DIRECT);
    if (fd < 0) {
    	cout << "Failed to open file" << endl;
        exit(1); 
    }

    if (posix_memalign(&buf, 4*KB, W) != 0) {
        cout << "Posix memalign failed" << endl;
        exit(1);
    }

    // for (int i=0; i<N; i++) {
    // 	blocks.push_back(i);
    // }

    while (skipped_file >> i) {
        skipped_blocks.insert(i);
    }

    // Skipping the last block, as writing to this will overshoot the filesize boundary
    for (int i=0; i<num_blocks-1; i++) {
        if (skipped_blocks.find(i) != skipped_blocks.end()) {
            continue;
        }
    	blocks.push_back(i);
    }
    random_shuffle(blocks.begin(), blocks.end());

    // Sanity check
    cout << "Blocks that can be overwritten count: " << blocks.size() << endl;

    it = blocks.begin();
    for (i=0; i<total_writes;) {
    	gettimeofday(&startTime, NULL);

    	for (j=0; j<num_write; j++) {
    		if (it == blocks.end()) {
    			random_shuffle(blocks.begin(), blocks.end());
    			it = blocks.begin();
    		}
    		offset = (*it)*block_size + block_size/2 + (rand() % w)*W;
    		lseek(fd, (off_t)offset, SEEK_SET);
    		write(fd, (char*)buf, W);
    		i++;
    		it++;
    	}
    	fsync(fd);

    	gettimeofday(&endTime, NULL);
    	record << getTimeDiff(startTime, endTime) << endl;
    }

    // random_shuffle(blocks.begin(), blocks.end());
    // for (it=blocks.begin(); it!=blocks.end(); it++) {
    // 	offset1 = (*it)*B + (rand() % (r-dist-1))*R;
    // 	offset2 = offset1 + dist*R;

    // 	gettimeofday(&startTime, NULL);
    	
    // 	lseek(fd, (off_t)offset1, SEEK_SET);
    // 	read(fd, (char*)buf, R);
    // 	lseek(fd, (off_t)offset2, SEEK_SET);
    // 	read(fd, (char*)buf, R);
    // 	fsync(fd);

    // 	gettimeofday(&endTime, NULL);
    // 	record << getTimeDiff(startTime, endTime) << endl;
    // }
    record.close();
    close(fd);
	return 0;
}