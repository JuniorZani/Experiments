#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
	int a, b, r;

	if(argc < 2){
		printf("Not enough variables");
		return 1;
	}

	a = atoi(argv[1]);
	b = atoi(argv[2]);

   	while (b != 0){
       r = a % b;
       a = b;
       b = r;
	}
	
	printf("MDC(%d,%d) = %d", atoi(argv[1]), atoi(argv[2]), a);
   	return 0;
}