/**
* Writes on all good offsets.
* Overwrites half the file essentially, for the 64KB chunk size.
*/

#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <fstream>
#include <vector>

using namespace std;

#define KB 1024L
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define FILESIZE 1*GB

long M = (128L*KB); // Maximum page size expected
long m = (4L*KB); // offset unit
long W = 64*KB; // Write size

#define BLOCK_SIZE (2*M)
// #define GOOD_OFFSETS_IN_BLOCK {8, 24, 40, 56} // EDIT IN THE MAIN BODY!
#define NUM_BLOCKS_IN_FILE (FILESIZE/BLOCK_SIZE)

long getTimeDiff(struct timeval startTime, struct timeval endTime) {
    return (long)((endTime.tv_sec - startTime.tv_sec)*1000000 +
        (endTime.tv_usec - startTime.tv_usec));
}

int main() {
    int fd;
    void* buf;
    long offset;
    int offsetsNotWritten = 0;
    struct timeval startTime, endTime;

    fd = open("/home/aarati/workspace/CS736/ssd-measurements/test1", O_RDWR | O_DIRECT | O_SYNC);
    vector<int> goodOffsetsInBlock;
    vector<int>::iterator it;

    if (fd < 0) {
        cout << "Couldn't open file" << endl;
        exit(1);
    }

    // Good offsets in a block
    goodOffsetsInBlock.push_back(8);
    goodOffsetsInBlock.push_back(24);
    goodOffsetsInBlock.push_back(40);
    goodOffsetsInBlock.push_back(56);

    if (posix_memalign(&buf, W/2, W)) {
        cout << "posix memalign failed" << endl;
        exit(2);
    }

    // system("blktrace -d /dev/nvme0n1 -o - | blkparse -f \"%N\n\" -i - >> /mnt/hdd/random/blktrace.output &");

    // start time
    gettimeofday(&startTime, NULL);

    // Write in chunk sizes of W, at aligned offsets
    for (long i=0; i<NUM_BLOCKS_IN_FILE; i++) {
        for (it=goodOffsetsInBlock.begin(); it!=goodOffsetsInBlock.end(); it++) {
            offset = i*BLOCK_SIZE + (*it)*m;
            if (rand() % 2 == 0) {
                lseek(fd, (off_t)offset, SEEK_SET);
                write(fd, (char*)buf, W);
                fsync(fd);
            } else {
                offsetsNotWritten++;
            }
        }
    }

    close(fd);

    // end time
    gettimeofday(&endTime, NULL);

    // If time taken is too high, it means garbage collection kicked in
    cout << "Time taken: " << getTimeDiff(startTime, endTime) << endl;
    cout << "Offsets not written: " << offsetsNotWritten << endl;

    // system("pkill blkparse");
    // system("pkill blktrace");

    return 0;
}
