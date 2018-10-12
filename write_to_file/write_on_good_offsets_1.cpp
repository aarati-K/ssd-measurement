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
#define WRITE_PERCENTAGE_1 25
#define WRITE_PERCENTAGE_2 25
#define WRITE_PERCENTAGE_4 25   // Write of size W, with 4 sub writes
#define FILENAME "/home/aarati/workspace/CS736/ssd-measurements/test_files/test_1mb_seq"

// Files for recording
#define OVERWRITTEN_1 "/mnt/hdd/record/updating_file/overwritten_1.txt"
#define OVERWRITTEN_2 "/mnt/hdd/record/updating_file/overwritten_2.txt"
#define OVERWRITTEN_4 "/mnt/hdd/record/updating_file/overwritten_4.txt"
#define SKIPPED "/mnt/hdd/record/updating_file/skipped.txt"

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
    int offsetsSkipped = 0;
    int offsetsWritten_1 = 0;
    int offsetsWritten_2 = 0;
    int offsetsWritten_4 = 0;
    int randomNumber;
    void* buf;
    long offset;
    vector<int> goodOffsetsInBlock;
    vector<int>::iterator it;
    struct timeval startTime, endTime;

    ofstream overwritten_1(OVERWRITTEN_1);
    ofstream overwritten_2(OVERWRITTEN_2);
    ofstream overwritten_4(OVERWRITTEN_4);
    ofstream skipped(SKIPPED);

    // Seed for rand function
    srand(time(0));

    fd = open(FILENAME, O_RDWR | O_DIRECT | O_SYNC);
    if (fd < 0) {
        cout << "Could not open file to write" << endl;
        exit(1);
    }

    if (!overwritten_1.is_open() || !overwritten_2.is_open() || !overwritten_4.is_open() || !skipped.is_open()) {
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
            if (randomNumber < WRITE_PERCENTAGE_1) {
                overwritten_1 << offset/m << endl;
                offsetsWritten_1 += 1;
                lseek(fd, (off_t)offset, SEEK_SET);
                write(fd, (char*)buf, W);
                fsync(fd);

            } else if (randomNumber < WRITE_PERCENTAGE_1 + WRITE_PERCENTAGE_2) {
                overwritten_2 << offset/m << endl;
                offsetsWritten_2 += 1;
                lseek(fd, (off_t)offset, SEEK_SET);
                write(fd, (char*)buf, W/2);
                fsync(fd);
                write(fd, (char*)buf, W/2);
                fsync(fd);

            } else if (randomNumber < WRITE_PERCENTAGE_1 + WRITE_PERCENTAGE_2 + WRITE_PERCENTAGE_4) {
                overwritten_4 << offset/m << endl;
                offsetsWritten_4 += 1; 
                lseek(fd, (off_t)offset, SEEK_SET);
                write(fd, (char*)buf, W/4);
                fsync(fd);
                write(fd, (char*)buf, W/4);
                fsync(fd);
                write(fd, (char*)buf, W/4);
                fsync(fd);
                write(fd, (char*)buf, W/4);
                fsync(fd);

            } else {
                skipped << offset/m << endl;
                offsetsSkipped += 1;
            }
        }
    }

    close(fd);

    // end time
    gettimeofday(&endTime, NULL);
    skipped.close();
    overwritten_1.close();
    overwritten_2.close();
    overwritten_4.close();

    // If time taken is too high, it means garbage collection kicked in
    // Includes the time taken to write to HDD
    cout << "Time taken: " << getTimeDiff(startTime, endTime) << endl;
    // Sanity checking
    cout << "Offsets written 1: " << offsetsWritten_1 << endl;
    cout << "Offsets written 2: " << offsetsWritten_2 << endl;
    cout << "Offsets written 4: " << offsetsWritten_4 << endl;
    cout << "Offsets skipped: " << offsetsSkipped << endl;
    cout << "Total offsets: " << (offsetsWritten_1 + offsetsWritten_2 + offsetsWritten_4 + offsetsSkipped) << endl;
    return 0;
}
