/*
* Compare the good offsets which were overwritten, to those which were not
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
#include <math.h>
#include <algorithm>

using namespace std;

#define KB 1024L
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define FILESIZE (1*GB)

// long M = (128L*KB); // Maximum page size expected
long m = (4L*KB); // offset unit
long W = (64*KB); // Write size

#define OVERWRITTEN_1 "/mnt/hdd/record/updating_file/overwritten_1.txt"
// #define OVERWRITTEN_1 "/mnt/hdd/record/overwritten_1.txt"
#define OVERWRITTEN_2 "/mnt/hdd/record/updating_file/overwritten_2.txt"
#define OVERWRITTEN_4 "/mnt/hdd/record/updating_file/overwritten_4.txt"
#define SKIPPED "/mnt/hdd/record/updating_file/skipped.txt"
// #define SKIPPED "/mnt/hdd/record/skipped_1.txt"

#define OVERWRITTEN_1_LATENCY "/mnt/hdd/record/updating_file/overwritten_1_latency.txt"
#define OVERWRITTEN_2_LATENCY "/mnt/hdd/record/updating_file/overwritten_2_latency.txt"
#define OVERWRITTEN_4_LATENCY "/mnt/hdd/record/updating_file/overwritten_4_latency.txt"
#define SKIPPED_LATENCY "/mnt/hdd/record/updating_file/skipped_latency.txt"

#define POLLUTE "/home/aarati/workspace/CS736/ssd-measurements/pollute"
#define FILENAME "/home/aarati/workspace/CS736/ssd-measurements/test_files/test_1mb_seq"

void calculateStandardDeviation(int* latencies, int count) {
    long variance = 0;
    int skipped = 0;
    long totalLatency = 0;
    float avgLatency;

    // Calculate average latency
    for (int i=0; i<count; i++) {
        if (latencies[i] > 600) {
            skipped += 1;
            continue;
        }
        totalLatency += (long)latencies[i];
    }
    avgLatency = totalLatency/(float)(count - skipped);
    cout << "Average latency: " << avgLatency << endl;

    // for (int i=0; i<count; i++) {
    //     // cout << latencies[i] << endl;
    //     if (latencies[i] > 600) {
    //         continue;
    //     }
    //     variance += pow((long)latencies[i] - (long)avgLatency, 2);
    // }
    // cout << "Standard Deviation: " << sqrt(variance/(count - skipped)) << endl;
}

void polluteSSDCache(int fd, void* buf, int numReads=30000) {
    for (int i=0; i<numReads; i++) {
        read(fd, (char*)buf, m);
    }
    fsync(fd);
}

long getTimeDiff(struct timeval startTime, struct timeval endTime) {
    return (long)((endTime.tv_sec - startTime.tv_sec)*1000000 +
        (endTime.tv_usec - startTime.tv_usec));
}

int main() {
    int fd, fd_pollute;
    
    ifstream overwritten1(OVERWRITTEN_1);
    ifstream overwritten2(OVERWRITTEN_2);
    ifstream overwritten4(OVERWRITTEN_4);
    ifstream skipped(SKIPPED);

    ofstream overwritten1_latency(OVERWRITTEN_1_LATENCY);
    ofstream overwritten2_latency(OVERWRITTEN_2_LATENCY);
    ofstream overwritten4_latency(OVERWRITTEN_4_LATENCY);
    ofstream skipped_latency(SKIPPED_LATENCY);

    vector<int> skippedOffsets, overwritten1Offsets, overwritten2Offsets, overwritten4Offsets;
    vector<int>::iterator it;
    int offset, count;
    int batchSize = 15;
    int i, j;
    void* buf;
    void* buf_pollute;
    struct timeval startTime, endTime;
    long timeTaken;

    if (!overwritten1.is_open() || !overwritten1_latency.is_open()) {
        cout << "Couldn't open the overwritten record file" << endl;
        exit(1);
    }

    if (!overwritten2.is_open() || !overwritten2_latency.is_open()) {
        cout << "Couldn't open the overwritten record file" << endl;
        exit(1);
    }

    if (!overwritten4.is_open() || !overwritten4_latency.is_open()) {
        cout << "Couldn't open the overwritten record file" << endl;
        exit(1);
    }

    if (!skipped.is_open() || !skipped_latency.is_open()) {
        cout << "Couldn't open the skipped record file" << endl;
        exit(1);
    }

    fd = open(FILENAME, O_RDWR | O_DIRECT | O_SYNC);
    fd_pollute = open(POLLUTE, O_RDWR | O_DIRECT);

    if (fd < 0 || fd_pollute < 0) {
        cout << "Failed to open the read/write file(s)" << endl;
        exit(1);
    }

    if (posix_memalign(&buf, W, W) != 0) {
        cout << "Posix memalign failed" << endl;
        exit(2);
    }

    if (posix_memalign(&buf_pollute, m, m) != 0) {
        cout << "Posix memalign failed" << endl;
        exit(2);
    }

    // Read the overwritten_1 offsets
    count = 0;
    while (overwritten1 >> offset) {
        overwritten1Offsets.push_back(offset);
        count += 1;
    }
    cout << "Total offsets overwritten 1: " << count << endl;
    cout << overwritten1Offsets.size() << endl;
    overwritten1.close();

    // Read the overwritten_2 offsets
    count = 0;
    while (overwritten2 >> offset) {
        overwritten2Offsets.push_back(offset);
        count += 1;
    }
    cout << "Total offsets overwritten 2: " << count << endl;
    cout << overwritten2Offsets.size() << endl;
    overwritten2.close();

    // Read the overwritten_4 offsets
    count = 0;
    while (overwritten4 >> offset) {
        overwritten4Offsets.push_back(offset);
        count += 1;
    }
    cout << "Total offsets overwritten 4: " << count << endl;
    cout << overwritten4Offsets.size() << endl;
    overwritten4.close();

    // Read the skipped offsets
    count = 0;
    while (skipped >> offset) {
        skippedOffsets.push_back(offset);
        count += 1;
    }
    cout << "Total offsets skipped: " << count << endl;
    cout << skippedOffsets.size() << endl;
    skipped.close();

    // Don't want read ahead to interfere
    random_shuffle(skippedOffsets.begin(), skippedOffsets.end());
    random_shuffle(overwritten1Offsets.begin(), overwritten1Offsets.end());
    random_shuffle(overwritten2Offsets.begin(), overwritten2Offsets.end());
    random_shuffle(overwritten4Offsets.begin(), overwritten4Offsets.end());

    // Read from the overwritten1 offsets
    j = 0;
    it = overwritten1Offsets.begin();
    while (it != overwritten1Offsets.end()) {
        for (i=0; i<batchSize; i++) {
            offset = (*it)*m;
            lseek(fd, (off_t)offset, SEEK_SET);

            // start time
            gettimeofday(&startTime, NULL);

            read(fd, (char*)buf, W);

            // end time
            gettimeofday(&endTime, NULL);
            timeTaken = getTimeDiff(startTime, endTime);
            // if (timeTaken < 300) {
            //     weird_overwritten << offset/m << endl;
            // }
            overwritten1_latency << timeTaken << endl;

            j++;
            it++;
            if (it == overwritten1Offsets.end()) {
                break;
            }
        }
        // Pollute the SSD cache
        polluteSSDCache(fd_pollute, buf_pollute, 100000);
    }
    overwritten1_latency.close();
    polluteSSDCache(fd_pollute, buf_pollute, 1000000);

    // Read from the overwritten2 offsets
    j = 0;
    it = overwritten2Offsets.begin();
    while (it != overwritten2Offsets.end()) {
        for (i=0; i<batchSize; i++) {
            offset = (*it)*m;
            lseek(fd, (off_t)offset, SEEK_SET);

            // start time
            gettimeofday(&startTime, NULL);

            read(fd, (char*)buf, W);

            // end time
            gettimeofday(&endTime, NULL);
            timeTaken = getTimeDiff(startTime, endTime);
            // if (timeTaken < 300) {
            //     weird_overwritten << offset/m << endl;
            // }
            overwritten2_latency << timeTaken << endl;

            j++;
            it++;
            if (it == overwritten2Offsets.end()) {
                break;
            }
        }
        // Pollute the SSD cache
        polluteSSDCache(fd_pollute, buf_pollute, 100000);
    }
    overwritten2_latency.close();
    polluteSSDCache(fd_pollute, buf_pollute, 1000000);

    // Read from the overwritten4 offsets
    j = 0;
    it = overwritten4Offsets.begin();
    while (it != overwritten4Offsets.end()) {
        for (i=0; i<batchSize; i++) {
            offset = (*it)*m;
            lseek(fd, (off_t)offset, SEEK_SET);

            // start time
            gettimeofday(&startTime, NULL);

            read(fd, (char*)buf, W);

            // end time
            gettimeofday(&endTime, NULL);
            timeTaken = getTimeDiff(startTime, endTime);
            // if (timeTaken < 300) {
            //     weird_overwritten << offset/m << endl;
            // }
            overwritten4_latency << timeTaken << endl;

            j++;
            it++;
            if (it == overwritten4Offsets.end()) {
                break;
            }
        }
        // Pollute the SSD cache
        polluteSSDCache(fd_pollute, buf_pollute, 100000);
    }
    overwritten4_latency.close();
    polluteSSDCache(fd_pollute, buf_pollute, 1000000);

    // Read from the skipped offsets
    j = 0;
    it = skippedOffsets.begin();
    while (it != skippedOffsets.end()) {
        for (i=0; i<batchSize; i++) {
            offset = (*it)*m;
            lseek(fd, (off_t)offset, SEEK_SET);

            // start time
            gettimeofday(&startTime, NULL);

            read(fd, (char*)buf, W);
            fsync(fd);

            // end time
            gettimeofday(&endTime, NULL);
            timeTaken = getTimeDiff(startTime, endTime);
            skipped_latency << timeTaken << endl;

            j++;
            it++;
            if (it == skippedOffsets.end()) {
                break;
            }
        }
    }
    skipped_latency.close();
    close(fd);
    close(fd_pollute);
    return 0;
}
