#include <cstdlib>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

#define KB 1024L

int main() {
        int fd, numReads;
        void* buf;
        fd = open("/home/aarati/workspace/CS736/ssd-measurements/pollute", O_RDWR | O_DIRECT);
        if (fd < 0) {
                cout << "could not open file" << endl;
                exit(1);
        }

        if (posix_memalign(&buf, 4*KB, 4*KB)) {
                cout << "posix_memalign failed" << endl;
                exit(2);
        }

        numReads = 1000000;
        for (int i=0; i<numReads; i++) {
                read(fd, (char*)buf, 4*KB);
        }
        fsync(fd);
        close(fd);
        return 0;
}
