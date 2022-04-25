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
	else if(!fileExist(argv[1]))
	{
		perror(argv[1]);
		exit(1);
	}
	else if(!fileExist(argv[2]))
	{
		perror(argv[2]);
		exit(1);
	}
	
	struct stat file1Info;
	struct stat file2Info;

	struct stat file1HardInfo;
	struct stat file2HardInfo;

	char *symfile1;
	char *symfile2;

	readlink(argv[1], symfile1, 100);
	readlink(argv[2], symfile2, 100);

	stat(argv[1], &file1HardInfo);
	stat(argv[2], &file2HardInfo);
	lstat(argv[1], &file1Info);
	lstat(argv[2], &file2Info);

	printf("Number of links for file 1, file 2: %ld %ld \n", file1HardInfo.st_nlink, file2HardInfo.st_nlink);
	printf("File 1 info: %ld %ld \n", file1Info.st_ino, file1HardInfo.st_ino );
	printf("File 2 info: %ld %ld \n", file2Info.st_ino, file2HardInfo.st_ino );
	printf("File 1, File2: %s %s\n", symfile1, symfile2);

	// Make sure both aren't soft links or hard links
	if(!(file1Info.st_ino != file1HardInfo.st_ino && file2Info.st_ino != file2HardInfo.st_ino) && 
	   !(file1Info.st_ino == file1HardInfo.st_ino && file2Info.st_ino == file2HardInfo.st_ino)) {
		// File 2 is a hard link to a file, and file 1 is a symbolic link to File 2	
		if (file1HardInfo.st_ino == file2HardInfo.st_ino && file1Info.st_ino != file2HardInfo.st_ino) {
			printf("%s is a symbolic link to %s\n", argv[1], argv[2]);
		}
		// Or vice versa
		else if (file1HardInfo.st_ino == file2HardInfo.st_ino && file1HardInfo.st_ino != file2Info.st_ino) {
			printf("%s is a symbolic link to %s\n", argv[2], argv[1]);
		}
		else {
			printf("These files are not linked (1).\n");
		}
	}
	// Both are hard links or soft links to the same file
	else if (file1HardInfo.st_ino == file2HardInfo.st_ino) {
		printf("These files are linked.\n");
	}
	else {
		printf("These files are not linked.\n");
	}
}

int fileExist (const char* filename)
{
	struct stat sb;
	int exist = stat(filename, &sb);
	if(!exist) return 1;
	else return 0;
}
