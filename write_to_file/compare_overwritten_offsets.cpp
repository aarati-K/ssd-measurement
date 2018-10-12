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

#define OVERWRITTEN "/mnt/hdd/record/overwritten_1.txt"
// #define OVERWRITTEN "/mnt/hdd/record/updating_file/overwritten_1.txt"
#define OVERWRITTEN_LATENCY "/mnt/hdd/record/overwritten_latency_1.txt"
#define SKIPPED "/mnt/hdd/record/skipped_1.txt"
// #define SKIPPED "/mnt/hdd/record/updating_file/skipped.txt"
#define SKIPPED_LATENCY "/mnt/hdd/record/skipped_latency_1.txt"
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

    for (int i=0; i<count; i++) {
        // cout << latencies[i] << endl;
        if (latencies[i] > 600) {
            continue;
        }
        variance += pow((long)latencies[i] - (long)avgLatency, 2);
    }
    cout << "Standard Deviation: " << sqrt(variance/(count - skipped)) << endl;
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
    ifstream overwritten(OVERWRITTEN);
    ifstream skipped(SKIPPED);
    ofstream overwritten_latency(OVERWRITTEN_LATENCY);
    ofstream skipped_latency(SKIPPED_LATENCY);
    // ofstream weird_overwritten(WEIRD_OVERWRITTEN);
    vector<int> skippedOffsets, overwrittenOffsets;
    vector<int>::iterator it;
    int offset, count;
    int batchSize = 15;
    int i, j;
    void* buf;
    void* buf_pollute;
    struct timeval startTime, endTime;
    long timeTaken;
    float avgLatency;

    if (!overwritten.is_open() || !overwritten_latency.is_open()) {
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

    // Read the overwritten offsets
    count = 0;
    while (overwritten >> offset) {
        overwrittenOffsets.push_back(offset);
        count += 1;
    }
    cout << "Total offsets overwritten: " << count << endl;
    cout << overwrittenOffsets.size() << endl;
    overwritten.close();

    // Read the skipped offsets
    count = 0;
    while (skipped >> offset) {
        skippedOffsets.push_back(offset);
        count += 1;
    }
    cout << "Total offsets skipped: " << count << endl;
    cout << skippedOffsets.size() << endl;
    skipped.close();

    int skippedOffsetsLatency[skippedOffsets.size()], overwrittenOffsetsLatency[overwrittenOffsets.size()];

    // Don't want read ahead to interfere
    random_shuffle(skippedOffsets.begin(), skippedOffsets.end());
    random_shuffle(overwrittenOffsets.begin(), overwrittenOffsets.end());

    // Read from the overwritten offsets
    // system("blktrace -d /dev/nvme0n1 -o - | blkparse -i - >> ./blktrace64k.output &");
    j = 0;
    it = overwrittenOffsets.begin();
    while (it != overwrittenOffsets.end()) {
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
            overwrittenOffsetsLatency[j] = timeTaken;
            overwritten_latency << timeTaken << endl;

            j++;
            it++;
            if (it == overwrittenOffsets.end()) {
                break;
            }
        }
        // Pollute the SSD cache
        polluteSSDCache(fd_pollute, buf_pollute, 100000);
    }
    cout << "Overwritten offsets results: " << endl;
    calculateStandardDeviation(overwrittenOffsetsLatency, overwrittenOffsets.size());
    // weird_overwritten.close();

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
            skippedOffsetsLatency[j] = timeTaken;
            skipped_latency << timeTaken << endl;

            j++;
            it++;
            if (it == skippedOffsets.end()) {
                break;
            }
        }
    }
    cout << "Skipped offsets results: " << endl;
    calculateStandardDeviation(skippedOffsetsLatency, skippedOffsets.size());

    // polluteSSDCache(fd_pollute, buf_pollute, 1000000);
    close(fd);
    close(fd_pollute);

    // // Write out overwritten offset latencies to a file
    // for (i=0; i<overwrittenOffsets.size(); i++) {
    //     overwritten_latency << overwrittenOffsetsLatency[i] << endl;
    // }
    overwritten_latency.close();
    skipped_latency.close();

    // system("pkill blkparse");
    // system("pkill blktrace");

    return 0;
}
