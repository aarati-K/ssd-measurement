#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

#define KB 1024L
#define MB (KB*KB)
#define GB (KB*KB*KB)

int main() {
        int fd;
        void* buf;
        long fileSize, numChunks;
        fd = open("/home/aarati/workspace/CS736/ssd-measurements/pollute", O_RDWR | O_DIRECT);
        if (fd <0 ) {
                cout << "Failed opening file" << endl;
                exit(1);
        }

        // Write to the file sequentially in 1MB chunks
        if (posix_memalign(&buf, 4*KB, 1*MB) != 0) {
                cout << "posix_memalign failed" << endl;
                exit(2);
        }
        fileSize = 1*GB;
        numChunks = fileSize/(1*MB);
        for (int i=0; i<numChunks; i++) {
                write(fd, (char*)buf, 1*MB);
                fsync(fd);
        }
        close(fd);
        return 0;
}
