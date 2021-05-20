// Project - Code Optimization
// System Programming, DGIST, Prof. Yeseong Kim
// 
// You will not turn in this file, so do not need to modify it.
// Read the provided instruction carefully.
//
// - Compute the number of cycles for the function f with given parameters.
// - The original souce comes with the computer system programming book.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>

#include "perfenv.h"

/* 
 * Routines for using the cycle counter 
 */

/* Initialize the cycle counter */
static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;

/* Set *hi and *lo to the high and low order bits  of the cycle counter.  
   Implementation requires assembly code to use the rdtsc instruction. */
void access_counter(unsigned *hi, unsigned *lo)
{
    asm("rdtsc; movl %%edx,%0; movl %%eax,%1"   /* Read cycle counter */
	: "=r" (*hi), "=r" (*lo)                /* and move results to */
	: /* No input */                        /* the two outputs */
	: "%edx", "%eax");
}

/* Record the current value of the cycle counter. */
void start_counter()
{
    access_counter(&cyc_hi, &cyc_lo);
}

/* Return the number of cycles since the last call to start_counter. */
double get_counter()
{
    unsigned ncyc_hi, ncyc_lo;
    unsigned hi, lo, borrow;
    double result;

    /* Get cycle counter */
    access_counter(&ncyc_hi, &ncyc_lo);

    /* Do double precision subtraction */
    lo = ncyc_lo - cyc_lo;
    borrow = lo > ncyc_lo;
    hi = ncyc_hi - cyc_hi - borrow;
    result = (double) hi * (1 << 30) * 4 + lo;
    if (result < 0) {
	fprintf(stderr, "Error: counter returns neg value: %.0f\n", result);
    }
    return result;
}
/* $end x86cyclecounter */

double ovhd()
{
    /* Do it twice to eliminate cache effects */
    int i;
    double result;

    for (i = 0; i < 2; i++) {
	start_counter();
	result = get_counter();
    }
    return result;
}

/* $begin mhz */
/* Estimate the clock rate by measuring the cycles that elapse */ 
/* while sleeping for sleeptime seconds */
double mhz_full(int verbose, int sleeptime)
{
    double rate;

    start_counter();
    sleep(sleeptime);
    rate = get_counter() / (1e6*sleeptime);
    if (verbose) 
	printf("Processor clock rate ~= %.1f MHz\n", rate);
    return rate;
}
/* $end mhz */

/* Version using a default sleeptime */
double mhz(int verbose)
{
    return mhz_full(verbose, 2);
}

/** Special counters that compensate for timer interrupt overhead */

static double cyc_per_tick = 0.0;

#define NEVENT 100
#define THRESHOLD 1000
#define RECORDTHRESH 3000

/* Attempt to see how much time is used by timer interrupt */
static void callibrate(int verbose)
{
    double oldt;
    struct tms t;
    clock_t oldc;
    int e = 0;

    times(&t);
    oldc = t.tms_utime;
    start_counter();
    oldt = get_counter();
    while (e <NEVENT) {
	double newt = get_counter();

	if (newt-oldt >= THRESHOLD) {
	    clock_t newc;
	    times(&t);
	    newc = t.tms_utime;
	    if (newc > oldc) {
		double cpt = (newt-oldt)/(newc-oldc);
		if ((cyc_per_tick == 0.0 || cyc_per_tick > cpt) && cpt > RECORDTHRESH)
		    cyc_per_tick = cpt;
		/*
		  if (verbose)
		  printf("Saw event lasting %.0f cycles and %d ticks.  Ratio = %f\n",
		  newt-oldt, (int) (newc-oldc), cpt);
		*/
		e++;
		oldc = newc;
	    }
	    oldt = newt;
	}
    }
      /* ifdef added by Sanjit - 10/2001 */
#ifdef DEBUG
    if (verbose)
	printf("Setting cyc_per_tick to %f\n", cyc_per_tick);
#endif
}

static clock_t start_tick = 0;

void start_comp_counter() 
{
    struct tms t;

    if (cyc_per_tick == 0.0)
	callibrate(1);
    times(&t);
    start_tick = t.tms_utime;
    start_counter();
}

double get_comp_counter() 
{
    double time = get_counter();
    double ctime;
    struct tms t;
    clock_t ticks;

    times(&t);
    ticks = t.tms_utime - start_tick;
    ctime = time - ticks*cyc_per_tick;
    /*
      printf("Measured %.0f cycles.  Ticks = %d.  Corrected %.0f cycles\n",
      time, (int) ticks, ctime);
    */
    return ctime;
}


#define K 3
#define MAXSAMPLES 20
#define EPSILON 0.01 
#define COMPENSATE 1
#define CLEAR_CACHE 1
#define CACHE_BYTES (1<<19) // 512KB
#define CACHE_BLOCK 32

static int kbest = K;
static int compensate = COMPENSATE;
static int clear_cache = CLEAR_CACHE;
static int maxsamples = MAXSAMPLES;
static double epsilon = EPSILON;
static int cache_bytes = CACHE_BYTES;
static int cache_block = CACHE_BLOCK;

static int *cache_buf = NULL;

static double *values = NULL;
static int samplecount = 0;

#define KEEP_VALS 0
#define KEEP_SAMPLES 0

#if KEEP_SAMPLES
static double *samples = NULL;
#endif

/* Start new sampling process */
static void init_sampler()
{
  if (values)
    free(values);
  values = calloc(kbest, sizeof(double));
#if KEEP_SAMPLES
  if (samples)
    free(samples);
  /* Allocate extra for wraparound analysis */
  samples = calloc(maxsamples+kbest, sizeof(double));
#endif
  samplecount = 0;
}

/* Add new sample.  */
static void add_sample(double val)
{
  int pos = 0;
  if (samplecount < kbest) {
    pos = samplecount;
    values[pos] = val;
  } else if (val < values[kbest-1]) {
    pos = kbest-1;
    values[pos] = val;
  }
#if KEEP_SAMPLES
  samples[samplecount] = val;
#endif
  samplecount++;
  /* Insertion sort */
  while (pos > 0 && values[pos-1] > values[pos]) {
    double temp = values[pos-1];
    values[pos-1] = values[pos];
    values[pos] = temp;
    pos--;
  }
}

/* Have kbest minimum measurements converged within epsilon? */
static int has_converged()
{
  return
    (samplecount >= kbest) &&
    ((1 + epsilon)*values[0] >= values[kbest-1]);
}

/* Code to clear cache */


static volatile int sink = 0;

static void clear()
{
  int x = sink;
  int *cptr, *cend;
  int incr = cache_block/sizeof(int);
  if (!cache_buf) {
    cache_buf = malloc(cache_bytes);
    if (!cache_buf) {
      fprintf(stderr, "Fatal error.  Malloc returned null when trying to clear cache\n");
      exit(1);
    }
  }
  cptr = (int *) cache_buf;
  cend = cptr + cache_bytes/sizeof(int);
  while (cptr < cend) {
    x += *cptr;
    cptr += incr;
  }
  sink = x;
}

double measure(func_t f, void *params[])
{
  double result;
  init_sampler();
  if (compensate) {
    do {
      double cyc;
      if (clear_cache)
          clear();
      start_comp_counter();
      f(params);
      cyc = get_comp_counter();
      add_sample(cyc);
      printf("Trial: %d\n", samplecount);
    } while (!has_converged() && samplecount < maxsamples);
  } else {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_counter();
      f(params);
      cyc = get_counter();
      add_sample(cyc);
    } while (!has_converged() && samplecount < maxsamples);
  }
#ifdef DEBUG
  {
    int i;
    printf(" %d smallest values: [", kbest);
    for (i = 0; i < kbest; i++)
      printf("%.0f%s", values[i], i==kbest-1 ? "]\n" : ", ");
  }
#endif
  result = values[0];
#if !KEEP_VALS
  free(values); 
  values = NULL;
#endif
  return result;  
}




/***********************************************************/
/* Set the various parameters used by measurement routines */


/* When set, will run code to clear cache before each measurement 
   Default = 0
*/
void set_fcyc_clear_cache(int clear)
{
  clear_cache = clear;
}

/* Set size of cache to use when clearing cache 
   Default = 1<<19 (512KB)
*/
void set_fcyc_cache_size(int bytes)
{
  if (bytes != cache_bytes) {
    cache_bytes = bytes;
    if (cache_buf) {
      free(cache_buf);
      cache_buf = NULL;
    }
  }
}

/* Set size of cache block 
   Default = 32
*/
void set_fcyc_cache_block(int bytes) {
  cache_block = bytes;
}


/* When set, will attempt to compensate for timer interrupt overhead 
   Default = 0
*/
void set_fcyc_compensate(int compensate_arg)
{
  compensate = compensate_arg;
}

/* Value of K in K-best
   Default = 3
*/
void set_fcyc_k(int k)
{
  kbest = k;
}

/* Maximum number of samples attempting to find K-best within some tolerance.
   When exceeded, just return best sample found.
   Default = 20
*/
void set_fcyc_maxsamples(int maxsamples_arg)
{
  maxsamples = maxsamples_arg;
}

/* Tolerance required for K-best
   Default = 0.01
*/
void set_fcyc_epsilon(double epsilon_arg)
{
  epsilon = epsilon_arg;
}





