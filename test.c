#include <stdio.h>
int notmain()
{
	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;
	int e = 0;
	int f = 0;
	int g = 0;
	printf("Hello, World.\n");
	if (a == 0) {
		return 1;
	} else {
		return 0;
	}
}

int main()
{
	int a = 0;
	printf("Hello, World.\n");
	if (a == 0) {
		return notmain();
	} else {
		return 0;
	}
}
