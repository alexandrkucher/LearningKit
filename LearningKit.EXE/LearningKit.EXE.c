#include <stdio.h>
#include "LearningKitInterface.h"

int main()
{
	while (1) 
	{
		int x = 5;
		printf("{%d} - Sometgin is happening right now...\r\n", x++);
		Sleep(1000);
	}

    return 0;
}

