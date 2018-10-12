#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <math.h>
#include <ctime>

using namespace std;

#define KB 1024L
#define MB (KB*KB)
#define GB (KB*KB*KB)
#define FILESIZE (1*MB)
// #define FILENAME "/home/aarati/workspace/CS736/ssd-measurements/test_files/test_temp_32KB"
#define FILENAME "/home/aarati/workspace/SQLite4/32kb_bad_layout.db"

long M = (128L*KB); // Maximum chunk size expected
long m = (4L*KB); // Minimum chunk size expected
long I = (64*KB); // Initial request size
long N = 1L; // number of request sizes tested
long O = (2*M/m); // Number of offsets

long getTimeDiff(struct timeval startTime, struct timeval endTime) {
    return (long)((endTime.tv_sec - startTime.tv_sec)*1000000 +
        (endTime.tv_usec - startTime.tv_usec));
}

float calculateStandardDeviation(int* latencies, float avgLatency, int trials) {
    long variance = 0;
    for (int i=0; i<trials; i++) {
        variance += pow(latencies[i] - avgLatency, 2);
    }
    return sqrt(variance/trials);
}

long getAlignedOffset() {
    long maxOffset = FILESIZE/(2*M);
    long randomNumber = (long)(rand()) % maxOffset;
    return (randomNumber * 2 * M);
}

void polluteSSDCache(int fd, void* buf, int numReads=30000) {
    for (int i=0; i<numReads; i++) {
        read(fd, (char*)buf, m);
    }
    fsync(fd);
}

int main() {
    int fd, fd_pollute;
    long n, o, i;
    struct timeval startTime, endTime;
    long elapsed;
    long requestSize, offset;
    long trials = 1000L;
    int latencies[trials];  // used to calculate the variance
    long timeTaken;
    long totalLatency;
    float avgLatency;
    long variance;
    float standardDeviation;
    long randOffset;
    void* buf;
    void* buf_pollute;

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

    requestSize = I;    // Initial request size
    // system("blktrace -d /dev/nvme0n1 -o - | blkparse -i - >> ./blktrace64k.output &");
    for (n=0; n<N; n++) {
        // polluteSSDCache(fd_pollute, buf_pollute, 100000);
        cout << "Starting for request size " << requestSize << endl;
        for (o=0; o<O; o++) {
            polluteSSDCache(fd_pollute, buf_pollute, 100000);
            offset = o*m;
            totalLatency = 0;
            // Do random reads of requestSize 100000 times
            for (i=0; i<trials; i++) {
                randOffset = getAlignedOffset() + offset;
                lseek(fd, (off_t)randOffset, SEEK_SET);
                // start time
                gettimeofday(&startTime, NULL);

                read(fd, (char*)buf, requestSize);
                fsync(fd);

                // end time
                gettimeofday(&endTime, NULL);
                timeTaken = getTimeDiff(startTime, endTime);
                totalLatency += timeTaken;
                latencies[i] = timeTaken;
            }
            avgLatency = totalLatency/(float)trials;
            standardDeviation = calculateStandardDeviation(latencies, avgLatency, trials);
            cout << "Offset " << o << ": " << avgLatency << ", " << standardDeviation << endl;
        }
        // Increase request size for next iteration
        requestSize = 2*requestSize;
    }
    // system("pkill blkparse");
    // system("pkill blktrace");
    close(fd);
    close(fd_pollute);
    return 0;
}
