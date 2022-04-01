/*
 * Copyright 2014-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include <stdio.h>
#include "microjvm_main.h"
#include "LLMJVM.h"
#include "sni.h"

/*
 * Generic VM main function
 */
void microjvm_main(void)
{
	void* vm;
	int32_t err;
	int32_t exitcode;
	
	// create VM
	vm = SNI_createVM();

	if(vm == NULL)
	{
		printf("VM initialization error.\n");
	}
	else
	{
		printf("VM START\n");
		err = SNI_startVM(vm, 0, NULL);

		if(err < 0)
		{
			// Error occurred
			if(err == LLMJVM_E_EVAL_LIMIT)
			{
				printf("Evaluation limits reached.\n");
			}
			else
			{
				printf("VM execution error (err = %d).\n", err);
			}
		}
		else
		{
			// VM execution ends normally
			exitcode = SNI_getExitCode(vm);
			printf("VM END (exit code = %d)\n", exitcode);
		}

		// delete VM
		SNI_destroyVM(vm);
	}
}
