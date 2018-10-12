#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <fstream>


using namespace std;

#define KB 1024L
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define FILESIZE 10*GB

long M = (128L*KB); // Maximum page size expected
long m = (4L*KB); // offset unit
long W = 64*KB; // Write size


#define CHUNK_SIZE 16 // 16 units, size = 16*m = 64KB
#define CHUNK_START_OFFSET 12 // Starts from 12 units = 12*m = 48*KB
#define BLOCK_SIZE (2*M)
#define GOOD_OFFSETS_IN_BLOCK {8, 24, 40, 56}
#define NUM_BLOCKS_IN_FILE (FILESIZE/BLOCK_SIZE)

long getAlignedOffset(long offsetInBlock=-1) {
    long blockNum = (long)(rand()) % NUM_BLOCKS_IN_FILE;
    if (offsetInBlock < 0) {
        long numOffsetsInBlock = (BLOCK_SIZE - CHUNK_START_OFFSET*m)/(m*CHUNK_SIZE) + 1;
        offsetInBlock = (long)(rand()) % numOffsetsInBlock;
    }

    // sanity check
    cout << "blockNum: " << blockNum;
    cout << " offsetInBlock: " << offsetInBlock << endl;
    return (blockNum*BLOCK_SIZE + (CHUNK_START_OFFSET + offsetInBlock*CHUNK_SIZE)*m);
}

int main() {
    int fd;
    int numWrites = 4000;
    long alignedOffset;
    // struct timeval startTime, endTime;
    void* buf;
    fd = open("/home/aarati/workspace/CS736/ssd-measurements/test", O_RDWR | O_DIRECT | O_SYNC);
    if (fd < 0) {
        cout << "Couldn't open file" << endl;
        exit(1);
    }

    // ofstream record_overwritten_offsets("/mnt/hdd/workloads/rand_writes.txt");
    // if (!record_overwritten_offsets.is_open()) {
    //     cout << "Failed to open file on HDD" << endl;
    //     exit(3);
    // }


    if (posix_memalign(&buf, 4*KB, W)) {
        cout << "posix memalign failed" << endl;
        exit(2);
    }

    // Write in chunk sizes of W, at aligned offsets
    for (int i=0; i<numWrites; i++) {
        alignedOffset = getAlignedOffset();
        // record_overwritten_offsets << alignedOffset << endl;
        lseek(fd, (off_t)alignedOffset, SEEK_SET);
        // start time
        // gettimeofday(&startTime, NULL);

        write(fd, (char*)buf, W);
        fsync(fd);

        // end time
        // gettimeofday(&endTime, NULL);
    }

    close(fd);
    return 0;
}
