#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int timediff(struct timeval *res, struct timeval *x, struct timeval *y) {
   /* Perform the carry for the later subtraction by updating y. */
   if (x->tv_usec < y->tv_usec) {
      int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
      y->tv_usec -= 1000000 * nsec;
      y->tv_sec += nsec;
   }
   if (x->tv_usec - y->tv_usec > 1000000) {
      int nsec = (x->tv_usec - y->tv_usec) / 1000000;
      y->tv_usec += 1000000 * nsec;
      y->tv_sec -= nsec;
   }
     
   /* Compute the time remaining to wait.
      tv_usec is certainly positive. */
   res->tv_sec = x->tv_sec - y->tv_sec;
   res->tv_usec = x->tv_usec - y->tv_usec;
   
   /* Return 1 if result is negative. */
   return x->tv_sec < y->tv_sec;
} // end timediff //


void multByM(double v[], double w[]);

extern int n;
extern unsigned long nz;

int main(int argc, char **argv) {
   int iters;
   int i,j;
   double *v, *w;
   struct timeval tp1, tp2, res;
   
   if (nz < 5000) {
     iters = 500000;
   } else if (nz < 10000) {
     iters = 200000;
   } else if (nz < 50000) {
     iters = 100000;
   } else if (nz < 100000) {
     iters = 50000;
   } else if (nz < 200000) {
     iters = 10000;
   } else if (nz < 1000000) {
     iters = 5000;
   } else if (nz < 2000000) {
     iters = 1000;
   } else if (nz < 3000000) {
     iters = 500;
   } else {
     iters = 200;
   }

  v = malloc(n*sizeof(double));
  w = malloc(n*sizeof(double));
  
  if (w==NULL  || v==NULL)  exit (1);
  
  for (i=0; i<n;i++) {
     v [i] = i;
  } // end for //


  gettimeofday(&tp1, NULL);
  for (i=0; i<iters; i++) {
    for (j=0; j<n; j++) w[j] = 0.0;
    multByM(v,w);
  } // end for //  
  gettimeofday(&tp2, NULL);

   timediff(&res, &tp2, &tp1);
   double timePerIter = ((res.tv_sec*1000000)+res.tv_usec) / (double)iters;
   printf("%lf ", timePerIter);

 /*  
  for (i=0; i<n;i++) {
     printf("%g\n",w [i]);
  } // end for //
*/
   
  return 0;
} // end mian //
