/*
 * instance-test.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "instance-test.h"
#include <CUnit/Basic.h>
#include "storage/availability.h"
#include "storage/entry-table.h"

char *key = "key";
char *value = "value";
char *mounting_path = "/home/utnso/entries/";
char *file_name = "/home/utnso/entries/key";

int instance_run_test() {
	CU_initialize_registry();

	CU_pSuite prueba = CU_add_suite("Instance suit", NULL, NULL);
	CU_add_test(prueba, "create availability all zero", test_availability_create_all_zero);
	CU_add_test(prueba, "file system store key", test_file_system_store_key);
	CU_add_test(prueba, "file system load key", test_file_system_load_key);
	CU_add_test(prueba, "test entry table has atomic", test_entry_table_has_atomic);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}


void test_availability_create_all_zero() {
	int entry_count = 8;
	t_availability * availability = availability_create(entry_count);

	CU_ASSERT_EQUAL(availability->max_entries, entry_count);

	CU_ASSERT_EQUAL(availability_get_free_entries_count(availability), entry_count);
}

void test_file_system_store_key() {
	int max_entries = 10;
	size_t entry_size = 40;
	t_entry_table *table = entry_table_create(max_entries, entry_size);
	entry_table_put(table, key, "value");
	entry_table_store(table, mounting_path, key);
}

void test_file_system_load_key() {
	FILE * file = fopen(file_name, "w");
	fwrite(value, 1, strlen(value), file);
	fclose(file);

	int max_entries = 10;
	size_t entry_size = 40;
	t_entry_table *table = entry_table_create(max_entries, entry_size);

	entry_table_load(table, mounting_path, key);

	char *value_loaded = entry_table_get(table, key);
	CU_ASSERT_STRING_EQUAL(value_loaded, value);
}

void test_entry_table_has_atomic() {
	int max_entries = 10;
	size_t entry_size = 10;
	t_entry_table *table = entry_table_create(max_entries, entry_size);
	bool has_atomic = entry_table_has_atomic_entries(table);
	CU_ASSERT_EQUAL(has_atomic, false);
	entry_table_put(table, "key1", "valuenotatomicvalue1");
	has_atomic = entry_table_has_atomic_entries(table);
	CU_ASSERT_EQUAL(has_atomic, false);
	entry_table_put(table, "key2", "value2");
	has_atomic = entry_table_has_atomic_entries(table);
	CU_ASSERT_EQUAL(has_atomic, true);
	entry_table_remove(table, "key2");
	has_atomic = entry_table_has_atomic_entries(table);
	CU_ASSERT_EQUAL(has_atomic, false);
}

void test_entry_table_replace_atomic() {
	int max_entries = 5;
	size_t entry_size = 5;
	t_entry_table *table = entry_table_create(max_entries, entry_size);

	entry_table_put(table, "key1", "123456");
	CU_ASSERT_EQUAL(entry_table_has_atomic_entries(table), false);
	entry_table_put(table, "key3", "654321");
	CU_ASSERT_EQUAL(entry_table_has_atomic_entries(table), false);
	entry_table_put(table, "key2", "12345");
	CU_ASSERT_EQUAL(entry_table_has_atomic_entries(table), true);

	entry_table_put(table, "key1", "valuenotatomicvalue1");
	has_atomic = entry_table_has_atomic_entries(table);
	CU_ASSERT_EQUAL(has_atomic, false);
	entry_table_put(table, "key2", "value2");
	has_atomic = entry_table_has_atomic_entries(table);
	CU_ASSERT_EQUAL(has_atomic, true);
	entry_table_remove(table, "key2");
	has_atomic = entry_table_has_atomic_entries(table);
	CU_ASSERT_EQUAL(has_atomic, false);
}
