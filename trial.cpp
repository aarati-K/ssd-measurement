#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <math.h>
#include <ctime>

using namespace std;
#define FILENAME "./temp.txt"

int main() {
	int sz;
	int fd = open(FILENAME, O_RDWR|O_DIRECT);
	if (fd < 0) {
		cout << "Failed" << endl;
		exit(1);
	}
	// Try doing a random write
	lseek(fd, (off_t)10, SEEK_SET);
	sz = write(fd, "somerandomstr", 13);
	if (sz != 13) {
		cout << "Failed writing randomly" << endl;
	}

	// Align a buffer of page size
	void *buf;
	if (posix_memalign(&buf, 4*1024, 4*1024) != 0) {
		cout << "posix_memalign failed" << endl;
		exit(1);
	}
	lseek(fd, (off_t)10, SEEK_SET);
	sz = write(fd, (char*)buf, 13);
	if (sz != 13) {
		cout << "Failed again" << endl;
	}

	// Write on page boundary
	lseek(fd, (off_t)4096, SEEK_SET);
	sz = write(fd, (char*)buf, 4096);
	if (sz == 4096) {
		cout << "Worked!" << endl;
	}

	close(fd);
    // ifstream i("/mnt/hdd/record/offsets_written/1.txt");
    // int offset;
    // i >> offset;
    // cout << offset << endl;
    // i>> offset;
    // cout << offset << endl;
    // i >> offset;
    // cout << offset << endl;
    return 0;
}
