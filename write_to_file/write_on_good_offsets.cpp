/**
* Write on good offsets, for a given ratio.
*/

#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <vector>

using namespace std;

#define KB 1024L
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define FILESIZE (1*GB)
#define WRITE_PERCENTAGE 25
#define FILENAME "/home/aarati/workspace/CS736/ssd-measurements/test_files/test_1mb_seq"

long M = (128L*KB); // Maximum page size expected
long m = (4L*KB); // offset unit
long W = (64*KB); // Write size

#define BLOCK_SIZE (2*M)
// #define GOOD_OFFSETS_IN_BLOCK {8, 24, 40, 56} // EDIT IN THE MAIN BODY!
#define NUM_BLOCKS_IN_FILE (FILESIZE/BLOCK_SIZE)

long getTimeDiff(struct timeval startTime, struct timeval endTime) {
    return (long)((endTime.tv_sec - startTime.tv_sec)*1000000 +
        (endTime.tv_usec - startTime.tv_usec));
}

int main() {
    int fd;
    int offsetsSkipped = 0, offsetsWritten = 0;
    int randomNumber;
    void* buf;
    long offset;
    vector<int> goodOffsetsInBlock;
    vector<int>::iterator it;
    ofstream overwritten("/mnt/hdd/record/overwritten_1.txt");
    ofstream skipped("/mnt/hdd/record/skipped_1.txt");
    struct timeval startTime, endTime;

    // Seed for rand function
    srand(time(0));

    fd = open(FILENAME, O_RDWR | O_DIRECT | O_SYNC);
    if (fd < 0) {
        cout << "Could not open file to write" << endl;
        exit(1);
    }

    if (!overwritten.is_open()) {
        cout << "Could not open offsets.txt file" << endl;
        exit(1);
    }

    goodOffsetsInBlock.push_back(8);
    goodOffsetsInBlock.push_back(24);
    goodOffsetsInBlock.push_back(40);
    goodOffsetsInBlock.push_back(56);

    if (posix_memalign(&buf, W, W) < 0) {
        cout << "posix_memalign failed" << endl;
        exit(2);
    }

    // start time
    gettimeofday(&startTime, NULL);

    for (long i=0; i<NUM_BLOCKS_IN_FILE; i++) {
        for (it=goodOffsetsInBlock.begin(); it!=goodOffsetsInBlock.end(); it++) {
            offset = i*BLOCK_SIZE + (*it)*m;
            randomNumber = rand() % 100;
            // cout << randomNumber << endl;
            if (randomNumber > (WRITE_PERCENTAGE - 1)) {
                // Do not write
                skipped << offset/m << endl;
                offsetsSkipped += 1;
                continue;
            }

            offsetsWritten += 1;
            lseek(fd, (off_t)offset, SEEK_SET);
            write(fd, (char*)buf, W);
            fsync(fd);

            overwritten << offset/m << endl;
        }
    }

    close(fd);

    // end time
    gettimeofday(&endTime, NULL);
    skipped.close();
    overwritten.close();

    // If time taken is too high, it means garbage collection kicked in
    // Includes the time taken to write to HDD
    cout << "Time taken: " << getTimeDiff(startTime, endTime) << endl;
    // Sanity checking
    cout << "Offsets skipped: " << offsetsSkipped << endl;
    cout << "Total offsets: " << (offsetsSkipped + offsetsWritten) << endl;
    return 0;
}
