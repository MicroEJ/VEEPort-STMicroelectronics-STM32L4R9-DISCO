/*
 * C
 *
 * Copyright 2014-2019 MicroEJ Corp. All rights reserved.
 * For demonstration purpose only.
 * MicroEJ Corp. PROPRIETARY. Use is subject to license terms.
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
