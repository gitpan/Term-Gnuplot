#include <stdio.h>

static void
croak(char *str)
{
   fprintf(stderr, "%s\n", str);
}

#define SET_OPTIONS_FROM_STRING
#define GNUPLOT_OUTLINE_STDOUT
#define DONT_POLLUTE_INIT
#include "Gnuplot.h"

static int dummy;
