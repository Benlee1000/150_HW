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

	if (argc > 3) {
		perror("Error, too many arguments");
		exit(1);
	}

	if(!fileExist(argv[1]) || !fileExist(argv[2]))
	{
		perror("Error, file 1 or file 2 does not exist.");
		exit(1);
	}
	// struct stat file1Info = lstat(file1);
	// struct stat file2Info = lstat(file2);

	struct stat file1HardInfo;
	struct stat file2HardInfo;

	struct stat john;
	struct stat james;

	stat(argv[1], &file1HardInfo);
	stat(argv[2], &file2HardInfo);
	lstat(argv[1], &john);
	lstat(argv[2], &james);

	if (file1HardInfo.st_ino == file2HardInfo.st_ino) {
		printf("These files are linked");
	}

		
	if(john.st_mode == james.st_mode)
	{
		printf("These files are soft linked");
	}

}

int fileExist (const char* filename)
{
	struct stat sb;
	int exist = stat(filename, &sb);
	if(!exist) return 1;
	else return 0;
}
