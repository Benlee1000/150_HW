#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int fileExist(const char *fileName);

int main(int argc, char *argv[]) {
	if (argc < 3) {
		perror("Error, not enough arguments");
		exit(1);
	}
	else if (argc > 3) {
		perror("Error, too many arguments");
		exit(1);
	}
	else if(!fileExist(argv[1]) || !fileExist(argv[2]))
	{
		perror("Error, file 1 or file 2 does not exist.");
		exit(1);
	}
	
	struct stat file1Info;
	struct stat file2Info;

	struct stat file1HardInfo;
	struct stat file2HardInfo;

	stat(argv[1], &file1HardInfo);
	stat(argv[2], &file2HardInfo);
	lstat(argv[1], &file1Info);
	lstat(argv[2], &file2Info);

	if (file1HardInfo.st_ino == file2HardInfo.st_ino) {
		printf("These files are linked");
	}
}

int fileExist (const char* filename)
{
	struct stat sb;
	int exist = stat(filename, &sb);
	if(!exist) return 1;
	else return 0;
}
