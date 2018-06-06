/*
 * file-system.c
 *
 *  Created on: 30 may. 2018
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>

int file_system_save(char* name, char* value) {
	int file = open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

	int length = strlen(value);

	ftruncate(file, length);

	void *map_file = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, file, 0);

	memcpy(map_file, value, length);

	munmap(map_file, length);
	close(file);
	return 0;
}

void file_system_delete(char* name) {

}

char* file_system_read(char* name) {
	int file = open(name, O_RDWR);

	struct stat st;
	fstat(file, &st);
	int length = st.st_size;

	void *map_file = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, file, 0);

	char *value = malloc(length);
	memcpy(value, map_file, length);

	munmap(map_file, length);
	close(file);

	return value;
}
