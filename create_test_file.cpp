/**
* Create a (10GB) file by writing 1MB chunks sequentially
**/

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

#define KB 1024L
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define FILESIZE (1*MB) // 1GB usually
#define FILENAME "/home/aarati/workspace/CS736/ssd-measurements/test_files/test_temp_32KB"
#define RECORD_FILE "/mnt/hdd/record/16mb_seq_write_1.txt"
#define CHUNK_SIZE (32*KB) // 1MB usually
#define RANDOM_ORDER 0 // 0 for false, 1 for true

long getTimeDiff(struct timeval startTime, struct timeval endTime) {
    return (long)((endTime.tv_sec - startTime.tv_sec)*1000000 +
        (endTime.tv_usec - startTime.tv_usec));
}

void polluteSSDCache(int fd, void* buf, int numReads=30000) {
    for (int i=0; i<numReads; i++) {
        read(fd, (char*)buf, 4*KB);
    }
    fsync(fd);
}

int main() {
    int fd;
    void* buf;
    void* buf_pollute;
    long numChunks;
    struct timeval startTime, endTime;
    long timeTaken, totalTimeTaken;
    // ofstream record_time(RECORD_FILE);
    vector<int> chunkOrder;
    vector<int>::iterator it;
    long offset;

    // if (!record_time.is_open()) {
    //     cout << "Failed opening record file" << endl;
    //     exit(1);
    // }

    fd = open(FILENAME, O_RDWR | O_SYNC | O_DIRECT);
    if (fd <0) {
            cout << "Failed opening file" << endl;
            exit(1);
    }
    fsync(fd);

    // fd_pollute = open("/home/aarati/workspace/CS736/ssd-measurements/pollute", O_RDWR | O_DIRECT);
    // if  (fd_pollute < 0) {
    //     cout << "Failed to open pollute file" << endl;
    //     exit(3);
    // }

    // Write to the file sequentially in CHUNK_SIZE chunks
    if (posix_memalign(&buf, 4*KB, CHUNK_SIZE) != 0) {
            cout << "posix_memalign failed" << endl;
            exit(2);
    }

    // if (posix_memalign(&buf_pollute, 4*KB, 4*KB) != 0) {
    //     cout << "Posix memalign failed" << endl;
    //     exit(1);
    // }

    numChunks = (FILESIZE)/(CHUNK_SIZE);
    cout << numChunks << " chunks" << endl; 
    // system("blktrace -d /dev/nvme0n1p6 -o - | blkparse -f \"%p - %N\n\" -i - >> /mnt/hdd/random/blktrace.output &");
    for (int i=0; i<numChunks; i++) {
        chunkOrder.push_back(i);
    }

    if (RANDOM_ORDER) {
        srand(time(0));
        random_shuffle(chunkOrder.begin(), chunkOrder.end());
    }

    totalTimeTaken = 0;
    for (it=chunkOrder.begin(); it!=chunkOrder.end(); it++) {
        // polluteSSDCache(fd_pollute, buf_pollute, 50);
        offset = (*it) * CHUNK_SIZE;
        // cout << offset << endl;
        lseek(fd, (off_t)offset, SEEK_SET);
        // start time
        gettimeofday(&startTime, NULL);
        write(fd, (char*)buf, CHUNK_SIZE);
        // end time
        gettimeofday(&endTime, NULL);
        // fsync(fd);
        // fsync(fd_pollute);

        timeTaken = getTimeDiff(startTime, endTime);
        totalTimeTaken += timeTaken;
        // Time taken is in micro seconds
        // record_time << float(1000000 * CHUNK_SIZE)/(1 * MB * timeTaken) << endl;
        // fsync(fd);
    }
    // fsync(fd);
    close(fd);
    
    cout << "Total time taken: " << totalTimeTaken << endl;
    // system("pkill blkparse");
    // system("pkill blktrace");
    // record_time.close();
    return 0;
}
