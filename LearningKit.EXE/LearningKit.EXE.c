#include <stdio.h>
#include "LearningKitInterface.h"

int main()
{

	for (unsigned char i = 0; i <= 9; i++)
	{
		_Bool result = DisplaySymbol(i);
		if (result)
			printf("Congratulations! You just displayed symbol %c", i);
		else
			printf("Something went wrong!");

		Sleep(1000);
	}
	return 0;
}

