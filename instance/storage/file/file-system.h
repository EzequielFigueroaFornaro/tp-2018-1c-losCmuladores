/*
 * file-system.h
 *
 *  Created on: 30 may. 2018
 *      Author: utnso
 */

#ifndef STORAGE_FILE_FILE_SYSTEM_H_
#define STORAGE_FILE_FILE_SYSTEM_H_

int file_system_save(char* name, char* value);

void file_system_delete(char* name);

char* file_system_read(char* name);

#endif /* STORAGE_FILE_FILE_SYSTEM_H_ */
