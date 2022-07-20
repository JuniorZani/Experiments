#include <stdio.h>
int main(void) {
    int n;        /* The number of fibonacci numbers we will print */
    int i;        /* The index of fibonacci number to be printed next*/
    int current;  /* The value of the (i)th fibonacci number */
    int next;     /* The value of the (i+1)th fibonacci number */
    int twoaway;  /* The value of the (i+2)th fibonacci number */
    int exit;
 
    printf("How many Fibonacci numbers do you want to compute? ");
    scanf("%d", &n);
    printf ("\n\n");
    if (n<=0)
       printf("The number should be positive.n");
    else {
      next = current = 1;
      for (i=1; i<=n; i++) {
          printf("%d    ",current);
          twoaway = current+next;
          current = next;
          next    = twoaway;
      }
    }
 printf ("\n\n");
}
  
