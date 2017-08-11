/*
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 */

#ifndef UNIT_TEST_H_
#define UNIT_TEST_H_

#include <CUnit/CUError.h>
#include <CUnit/TestDB.h>

#define MAX		1024

struct TestTable {
	const char *name;
	CU_TestFunc function;
};

CU_ErrorCode add_tests(CU_pSuite pSuite, struct TestTable *testTable);
CU_ErrorCode create_uri_suit(void);
CU_ErrorCode create_client_suit(void);
CU_ErrorCode create_object_read_suit(void);

#endif /* UNIT_TEST_H_ */
