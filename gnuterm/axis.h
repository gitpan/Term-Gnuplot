/* 
 * $Id: axis.h,v 1.23 2002/09/02 18:15:30 sfeam Exp $
 *
 */

/*[
 * Copyright 2000   Thomas Williams, Colin Kelley
 *
 * Permission to use, copy, and distribute this software and its
 * documentation for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 * Permission to modify the software is granted, but not the right to
 * distribute the complete modified source code.  Modifications are to
 * be distributed as patches to the released version.  Permission to
 * distribute binaries produced by compiling modified sources is granted,
 * provided you
 *   1. distribute the corresponding source modifications from the
 *    released version in the form of a patch file along with the binaries,
 *   2. add special version identification to distinguish your version
 *    in addition to the base release version number,
 *   3. provide your name and address as the primary contact for the
 *    support of your modified version, and
 *   4. retain our contact information in regard to use of the base
 *    software.
 * Permission to distribute the released version of the source code along
 * with corresponding source modifications in the form of a patch file is
 * granted with same provisions 2 through 4 for binary distributions.
 *
 * This software is provided "as is" without express or implied warranty
 * to the extent permitted by applicable law.
]*/

#ifndef GNUPLOT_AXIS_H
#define GNUPLOT_AXIS_H

#include "gp_types.h"		/* for TBOOLEAN */

#include "gadgets.h"
#include "parse.h"		/* for const_express() */
#include "tables.h"		/* for the axis name parse table */
#include "term_api.h"		/* for lp_style_type */
#include "util.h"		/* for int_error() */

/* typedefs / #defines */

/* give some names to some array elements used in command.c and grap*.c
 * maybe one day the relevant items in setshow will also be stored
 * in arrays. 
 *
 * Always keep the following conditions alive:
 * SECOND_X_AXIS = FIRST_X_AXIS + SECOND_AXES
 * FIRST_X_AXIS & SECOND_AXES == 0
 */
typedef enum AXIS_INDEX {
#define FIRST_AXES 0
    FIRST_Z_AXIS, 
    FIRST_Y_AXIS,
    FIRST_X_AXIS,
    T_AXIS,			/* fill gap */
#define SECOND_AXES 4
    SECOND_Z_AXIS,		/* not used, yet */
    SECOND_Y_AXIS,
    SECOND_X_AXIS,
    R_AXIS,			/* never used ? */
    U_AXIS,			/* dito */
    V_AXIS,			/* dito */
#ifdef PM3D
    COLOR_AXIS
#endif
} AXIS_INDEX;

#ifdef PM3D
# define AXIS_ARRAY_SIZE 11
#else
# define AXIS_ARRAY_SIZE 10
#endif


/* What kind of ticmarking is wanted? */
typedef enum en_ticseries_type {
    TIC_COMPUTED=1, 		/* default; gnuplot figures them */
    TIC_SERIES,			/* user-defined series */
    TIC_USER,			/* user-defined points */
    TIC_MONTH,   		/* print out month names ((mo-1)%12)+1 */
    TIC_DAY      		/* print out day of week */
} t_ticseries_type;

/* Defines one ticmark for TIC_USER style.
 * If label==NULL, the value is printed with the usual format string.
 * else, it is used as the format string (note that it may be a constant
 * string, like "high" or "low").
 */
typedef struct ticmark {
    double position;		/* where on axis is this */
    char *label;		/* optional (format) string label */
    struct ticmark *next;	/* linked list */
} ticmark;

/* Tic-mark labelling definition; see set xtics */
typedef struct ticdef {
    t_ticseries_type type;	
    union {
	   struct ticmark *user;	/* for TIC_USER */
	   struct {			/* for TIC_SERIES */
		  double start, incr;
		  double end;		/* ymax, if VERYLARGE */
	   } series;
    } def;
} t_ticdef;

/* we want two auto modes for minitics - default where minitics are
 * auto for log/time and off for linear, and auto where auto for all
 * graphs I've done them in this order so that logscale-mode can
 * simply test bit 0 to see if it must do the minitics automatically.
 * similarly, conventional plot can test bit 1 to see if minitics are
 * required */
enum en_minitics_status {
    MINI_OFF,
    MINI_DEFAULT,
    MINI_USER,
    MINI_AUTO
};

/* Function pointer type for callback functions doing operations for a
 * single ticmark */
typedef void (*tic_callback) __PROTO((AXIS_INDEX, double, char *, struct lp_style_type ));

/* Values to put in the axis_tics[] variables that decides where the
 * ticmarks should be drawn: not at all, on one or both plot borders,
 * or the zeroaxes. These look like a series of values, but TICS_MASK
 * shows that they're actually bit masks --> don't turn into an enum
 * */
#define NO_TICS        0
#define TICS_ON_BORDER 1
#define TICS_ON_AXIS   2
#define TICS_MASK      3
#define TICS_MIRROR    4


#if 0 /* HBB 20010806 --- move GRID flags into axis struct */
/* Need to allow user to choose grid at first and/or second axes tics.
 * Also want to let user choose circles at x or y tics for polar grid.
 * Also want to allow user rectangular grid for polar plot or polar
 * grid for parametric plot. So just go for full configurability.
 * These are bitmasks
 */
#define GRID_OFF    0
#define GRID_X      (1<<0)
#define GRID_Y      (1<<1)
#define GRID_Z      (1<<2)
#define GRID_X2     (1<<3)
#define GRID_Y2     (1<<4)
#define GRID_MX     (1<<5)
#define GRID_MY     (1<<6)
#define GRID_MZ     (1<<7)
#define GRID_MX2    (1<<8)
#define GRID_MY2    (1<<9)
/* GRID_{M}CB only for PM3D, but defined always for source code readability */
#define GRID_CB     (1<<10)
#define GRID_MCB    (1<<11)
#endif /* 0 */

/* HBB 20010610: new type for storing autoscale activity. Effectively
 * two booleans (bits) in a single variable, so I'm using an enum with
 * all 4 possible bit masks given readable names. */
typedef enum e_autoscale {
    AUTOSCALE_NONE = 0,
    AUTOSCALE_MIN = 1<<0,
    AUTOSCALE_MAX = 1<<1,
    AUTOSCALE_BOTH = (1<<0 | 1 << 1),
    AUTOSCALE_FIXMIN = 1<<2,
    AUTOSCALE_FIXMAX = 1<<3
} t_autoscale;


/* FIXME 20000725: collect some of those various TBOOLEAN fields into
 * a larger int (or -- shudder -- a bitfield?) */
typedef struct axis {
/* range of this axis */
    t_autoscale autoscale;	/* Which end(s) are autoscaled? */
    t_autoscale set_autoscale;	/* what does 'set' think autoscale to be? */
    int range_flags;		/* flag bits about autoscale/writeback: */
    /* write auto-ed ranges back to variables for autoscale */
#define RANGE_WRITEBACK 1	
    /* allow auto and reversed ranges */
#define RANGE_REVERSE   2	
    TBOOLEAN reverse_range;	/* range [high:low] silently reverted? */
    double min;			/* 'transient' axis extremal values */
    double max;
    double set_min;		/* set/show 'permanent' values */
    double set_max;
    double writeback_min;	/* ULIG's writeback implementation */
    double writeback_max;

/* output-related quantities */
    int term_lower;		/* low and high end of the axis on output, */
    int term_upper;		/* ... (in terminal coordinates)*/
    double term_scale;		/* scale factor: plot --> term coords */
    unsigned int term_zero;	/* position of zero axis */

/* log axis control */
    TBOOLEAN log;		/* log axis stuff: flag "islog?" */
    double base;		/* logarithm base value */
    double log_base;		/* ln(base), for easier computations */

/* time/date axis control */
    TBOOLEAN is_timedata;	/* is this a time/date axis? */
    TBOOLEAN format_is_numeric;	/* format string looks like numeric??? */
    char timefmt[MAX_ID_LEN+1];	/* format string for input */
    char formatstring[MAX_ID_LEN+1];
				/* the format string for output */

/* ticmark control variables */
    int ticmode;		/* tics on border/axis? mirrored? */
    struct ticdef ticdef;	/* tic series definition */
    int tic_rotate;		/* ticmarks rotated by this angle */
    TBOOLEAN gridmajor;		/* Grid lines wanted on major tics? */
    TBOOLEAN gridminor;		/* Grid lines for minor tics? */
    int minitics;		/* minor tic mode (none/auto/user)? */
    double mtic_freq;		/* minitic stepsize */

/* other miscellaneous fields */
    label_struct label;		/* label string and position offsets */
    lp_style_type zeroaxis;	/* drawing style for zeroaxis, if any */
} AXIS;

#define DEFAULT_AXIS_TICDEF {TIC_COMPUTED, {NULL} }
#ifdef PM3D
# define DEFAULT_AXIS_ZEROAXIS {0, -3, 0, 1.0, 1.0, 0}
#else
# define DEFAULT_AXIS_ZEROAXIS {0, -3, 0, 1.0, 1.0}
#endif

#define DEFAULT_AXIS_STRUCT {						    \
	AUTOSCALE_BOTH, AUTOSCALE_BOTH, /* auto, set_auto */		    \
	0, FALSE,		/* range_flags, rev_range */		    \
	-10.0, 10.0,		/* 3 pairs of min/max */		    \
	-10.0, 10.0,							    \
	-10.0, 10.0,							    \
	0, 0, 0, 0,		/* terminal dependents */		    \
	FALSE, 0.0, 0.0,	/* log, base, log(base) */		    \
	0, 1,			/* is_timedata, format_numeric */	    \
	DEF_FORMAT, TIMEFMT,	/* output format, timefmt */		    \
	NO_TICS,		/* tic output positions (border, mirror) */ \
	DEFAULT_AXIS_TICDEF,	/* tic series definition */		    \
	0, FALSE, FALSE, 	/* tic_rotate, grid{major,minor} */	    \
	MINI_DEFAULT, 10,	/* minitics, mtic_freq */		    \
	EMPTY_LABELSTRUCT,	/* axis label */			    \
	DEFAULT_AXIS_ZEROAXIS	/* zeroaxis line style */		    \
}

/* Table of default behaviours --- a subset of the struct above. Only
 * those fields are present that differ from axis to axis. */
typedef struct axis_defaults {
    double min;			/* default axis endpoints */
    double max;
    char name[4];		/* axis name, like in "x2" or "t" */
    int ticmode;		/* tics on border/axis? mirrored? */
} AXIS_DEFAULTS;



/* global variables in axis.c */

extern AXIS axis_array[AXIS_ARRAY_SIZE];
extern const AXIS_DEFAULTS axis_defaults[AXIS_ARRAY_SIZE];

/* A parsing table for mapping axis names into axis indices. For use
 * by the set/show machinery, mainly */
extern const struct gen_table axisname_tbl[AXIS_ARRAY_SIZE+1];


extern const struct ticdef default_axis_ticdef;

/* default format for tic mark labels */
#define DEF_FORMAT "% g"

/* default parse timedata string */
#define TIMEFMT "%d/%m/%y,%H:%M"

/* axis labels */
extern const label_struct default_axis_label;

/* zeroaxis linetype (flag type==-3 if none wanted) */
extern const lp_style_type default_axis_zeroaxis;

/* default grid linetype, to be used by 'unset grid' and 'reset' */
extern const struct lp_style_type default_grid_lp;

/* grid layer: -1 default, 0 back, 1 front */
extern int grid_layer;

/* global variables for communication with the tic callback functions */
/* FIXME HBB 20010806: had better be collected into a struct that's
 * passed to the callback */
extern int tic_start, tic_direction, tic_mirror;
/* These are for passing on to write_multiline(): */
extern int tic_text, rotate_tics, tic_hjust, tic_vjust;
/* Some of them are controlled by 'set' commands: */
extern double ticscale;		/* scale factor for tic marks (was (0..1])*/
extern double miniticscale;	/* and for minitics */
extern TBOOLEAN	tic_in;		/* tics to be drawn inward?  */
/* The remaining ones are for grid drawing; controlled by 'set grid': */
/* extern int grid_selection; --- comm'ed out, HBB 20010806 */
extern struct lp_style_type grid_lp; /* linestyle for major grid lines */
extern struct lp_style_type mgrid_lp; /* linestyle for minor grid lines */
extern double polar_grid_angle; /* angle step in polar grid in radians */

/* Length of the longest tics label, set by widest_tic_callback(): */
extern int widest_tic_strlen;

/* axes being used by the current plot */
extern AXIS_INDEX x_axis, y_axis, z_axis;
/* macros to reduce code clutter caused by the array notation, mainly
 * in graphics.c and fit.c */
#define X_AXIS axis_array[x_axis]
#define Y_AXIS axis_array[y_axis]
#define Z_AXIS axis_array[z_axis]
#ifdef PM3D
#define CB_AXIS axis_array[COLOR_AXIS]
#endif

/* -------- macros using these variables: */

/* Macros to map from user to terminal coordinates and back */
#define AXIS_MAP(axis, variable)		\
  (int) ((axis_array[axis].term_lower)		\
	 + ((variable) - axis_array[axis].min)	\
	 * axis_array[axis].term_scale + 0.5)
#define AXIS_MAPBACK(axis, pos)						   \
  (((double)(pos)-axis_array[axis].term_lower)/axis_array[axis].term_scale \
   + axis_array[axis].min)

/* these are the old names for these: */
#define map_x(x) AXIS_MAP(x_axis, x)
#define map_y(y) AXIS_MAP(y_axis, y)

#define AXIS_SETSCALE(axis, out_low, out_high)			\
    axis_array[axis].term_scale = ((out_high) - (out_low))	\
        / (axis_array[axis].max - axis_array[axis].min)

/* write current min/max_array contents into the set/show status
 * variables */
#define AXIS_WRITEBACK(axis)			\
do {						\
    AXIS *this = axis_array + axis;		\
						\
    if (this->range_flags & RANGE_WRITEBACK) {	\
	if (this->autoscale & AUTOSCALE_MIN)	\
	    this->set_min = this->min;		\
	if (this->autoscale & AUTOSCALE_MAX)	\
	    this->set_max = this->max;		\
    }						\
} while(0)

/* HBB 20000430: New macros, logarithmize a value into a stored
 * coordinate*/ 
#define AXIS_DO_LOG(axis,value) (log(value) / axis_array[axis].log_base)
#define AXIS_UNDO_LOG(axis,value) exp((value) * axis_array[axis].log_base)

/* HBB 20000430: same, but these test if the axis is log, first: */
#define AXIS_LOG_VALUE(axis,value)				\
    (axis_array[axis].log ? AXIS_DO_LOG(axis,value) : (value))
#define AXIS_DE_LOG_VALUE(axis,coordinate)				  \
    (axis_array[axis].log ? AXIS_UNDO_LOG(axis,coordinate): (coordinate))
 

/* copy scalar data to arrays. The difference between 3D and 2D
 * versions is: dont know we have to support ranges [10:-10] - lets
 * reverse it for now, then fix it at the end.  */
/* FIXME HBB 20000426: unknown if this distinction makes any sense... */
#define AXIS_INIT3D(axis, islog_override, infinite)			\
do {									\
    AXIS *this = axis_array + axis;					\
									\
    if ((this->autoscale = this->set_autoscale) == AUTOSCALE_NONE	\
	&& this->set_max < this->set_min) {				\
	this->min = this->set_max;					\
	this->max = this->set_min;					\
        /* we will fix later */						\
    } else {								\
	this->min = (infinite && (this->set_autoscale & AUTOSCALE_MIN))	\
	    ? VERYLARGE : this->set_min;				\
	this->max = (infinite && (this->set_autoscale & AUTOSCALE_MAX))	\
	    ? -VERYLARGE : this->set_max;				\
    }									\
    if (islog_override) {						\
	this->log = 0;							\
	this->base = 1;							\
	this->log_base = 0;						\
    } else {								\
	this->log_base = this->log ? log(this->base) : 0;		\
    }									\
} while(0)

#define AXIS_INIT2D(axis, infinite)					\
do {									\
    AXIS *this = axis_array + axis;					\
									\
    this->autoscale = this->set_autoscale;				\
    this->min = (infinite && (this->set_autoscale & AUTOSCALE_MIN))	\
	? VERYLARGE : this->set_min;					\
    this->max = (infinite && (this->set_autoscale & AUTOSCALE_MAX))	\
	? -VERYLARGE : this->set_max;					\
    this->log_base = this->log ? log(this->base) : 0;			\
} while(0)

/* handle reversed ranges */
#define CHECK_REVERSE(axis)						\
do {									\
    AXIS *this = axis_array + axis;					\
									\
    if ((this->autoscale == AUTOSCALE_NONE)				\
	&& (this->max < this->min)					\
	) {								\
	double temp = this->min;					\
	this->min = this->max;						\
	this->max = temp;						\
	this->reverse_range = 1;					\
    } else								\
	this->reverse_range = (this->range_flags & RANGE_REVERSE);	\
} while(0)

/* HBB 20000725: new macro, built upon ULIG's SAVE_WRITEBACK(axis),
 * but easier to use. Code like this occured twice, in plot2d and
 * plot3d: */
#define SAVE_WRITEBACK_ALL_AXES					\
do {								\
    AXIS_INDEX axis;						\
								\
    for (axis = 0; axis < AXIS_ARRAY_SIZE; axis++)		\
	if(axis_array[axis].range_flags & RANGE_WRITEBACK) {	\
	    set_writeback_min(axis);				\
	    set_writeback_max(axis);				\
	}							\
} while(0)
	    
/* get optional [min:max] */
#define PARSE_RANGE(axis)						   \
do {									   \
    if (equals(c_token, "[")) {						   \
	c_token++;							   \
	axis_array[axis].autoscale =					   \
	    load_range(axis, &axis_array[axis].min, &axis_array[axis].max, \
		       axis_array[axis].autoscale);			   \
	if (!equals(c_token, "]"))					   \
	    int_error(c_token, "']' expected");				   \
	c_token++;							   \
    }									   \
} while (0)

/* HBB 20000430: new macro, like PARSE_RANGE, but for named ranges as
 * in 'plot [phi=3.5:7] sin(phi)' */
#define PARSE_NAMED_RANGE(axis, dummy_token)				     \
do {									     \
    if (equals(c_token, "[")) {						     \
	c_token++;							     \
	if (isletter(c_token)) {					     \
	    if (equals(c_token + 1, "=")) {				     \
		dummy_token = c_token;					     \
		c_token += 2;						     \
	    }								     \
		/* oops; probably an expression with a variable: act	     \
		 * as if no variable name had been seen, by		     \
		 * fallthrough */					     \
	}								     \
	axis_array[axis].autoscale = load_range(axis, &axis_array[axis].min, \
				      &axis_array[axis].max,		     \
				      axis_array[axis].autoscale);	     \
	if (!equals(c_token, "]"))					     \
	    int_error(c_token, "']' expected");				     \
	c_token++;							     \
    }				/* first '[' */				     \
} while (0)

/* parse a position of the form
 *    [coords] x, [coords] y {,[coords] z}
 * where coords is one of first,second.graph,screen
 * if first or second, we need to take axis_is_timedata into account
 */
#define GET_NUMBER_OR_TIME(store,axes,axis)				\
do {									\
    if (((axes) >= 0) && (axis_array[(axes)+(axis)].is_timedata)	\
	&& isstring(c_token)) {						\
	char ss[80];							\
	struct tm tm;							\
	quote_str(ss,c_token, 80);					\
	++c_token;							\
	if (gstrptime(ss,axis_array[axis].timefmt,&tm))			\
	    (store) = (double) gtimegm(&tm);				\
    } else {								\
	struct value value;						\
	(store) = real(const_express(&value));				\
    }									\
} while(0)

/* This is one is very similar to GET_NUMBER_OR_TIME, but has slightly
 * different usage: it writes out '0' in case of inparsable time data,
 * and it's used when the target axis is fixed without a 'first' or
 * 'second' keyword in front of it. */
#define GET_NUM_OR_TIME(store,axis)			\
do {							\
    (store) = 0;					\
    GET_NUMBER_OR_TIME(store, FIRST_AXES, axis);	\
} while (0);

/* store VALUE or log(VALUE) in STORE, set TYPE as appropriate
 * Do OUT_ACTION or UNDEF_ACTION as appropriate
 * adjust range provided type is INRANGE (ie dont adjust y if x is outrange
 * VALUE must not be same as STORE
 * Note: see the particular implementation for COLOR AXIS below.
 */

#define STORE_WITH_LOG_AND_UPDATE_RANGE(STORE, VALUE, TYPE, AXIS,	  \
				       OUT_ACTION, UNDEF_ACTION)	  \
do {									  \
    /* HBB 20000726: new check, to avoid crashes with axis index -1 */	  \
    if (AXIS==-1)							  \
	break;								  \
    if (axis_array[AXIS].log) {						  \
	if (VALUE<0.0) {						  \
	    TYPE = UNDEFINED;						  \
	    UNDEF_ACTION;						  \
	    break;							  \
	} else if (VALUE == 0.0) {					  \
	    STORE = -VERYLARGE;						  \
	    TYPE = OUTRANGE;						  \
	    OUT_ACTION;							  \
	    break;							  \
	} else {							  \
	    STORE = AXIS_DO_LOG(AXIS,VALUE);				  \
	}								  \
    } else								  \
	STORE = VALUE;							  \
    if (TYPE != INRANGE)						  \
	break;  /* don't set y range if x is outrange, for example */	  \
    if ((int)AXIS < 0)							  \
	break;	/* HBB 20000507: don't check range if not a coordinate */ \
    if ( VALUE<axis_array[AXIS].min ) {					  \
	if (axis_array[AXIS].autoscale & AUTOSCALE_MIN)			  \
	    axis_array[AXIS].min = VALUE;				  \
	else {								  \
	    TYPE = OUTRANGE;						  \
	    OUT_ACTION;							  \
	    break;							  \
	}								  \
    }									  \
    if ( VALUE>axis_array[AXIS].max ) {					  \
	if (axis_array[AXIS].autoscale & AUTOSCALE_MAX)			  \
	    axis_array[AXIS].max = VALUE;				  \
	else {								  \
	    TYPE = OUTRANGE;						  \
	    OUT_ACTION;							  \
	}								  \
    }									  \
} while(0)

/* Implementation of the above for the color axis. It should not change
 * the type of the point (out-of-range color is plotted with the color
 * of the min or max color value).
 */
#ifdef PM3D
#define COLOR_STORE_WITH_LOG_AND_UPDATE_RANGE(STORE, VALUE, TYPE, AXIS,	  \
				       OUT_ACTION, UNDEF_ACTION)	  \
{									  \
    int c_type_tmp = TYPE;						  \
    STORE_WITH_LOG_AND_UPDATE_RANGE(STORE, VALUE, c_type_tmp, AXIS,	  \
				       OUT_ACTION, UNDEF_ACTION);	  \
}
#endif

/* use this instead empty macro arguments to work around NeXT cpp bug */
/* if this fails on any system, we might use ((void)0) */
#define NOOP			/* */

/* HBB 20000506: new macro, initializes one variable to the same
 * value, for all axes. */
#define INIT_AXIS_ARRAY(field, value)		\
do {						\
    int tmp;					\
    for (tmp=0; tmp<AXIS_ARRAY_SIZE; tmp++)	\
	axis_array[tmp].field=(value);		\
} while(0)

/* HBB 20000506: new macro to automatically build intializer lists
 * for arrays of AXIS_ARRAY_SIZE equal elements */
#ifdef PM3D
#define AXIS_ARRAY_INITIALIZER(value) {			\
    value, value, value, value, value,			\
	value, value, value, value, value, value }
#else
#define AXIS_ARRAY_INITIALIZER(value) {		\
    value, value, value, value, value,		\
	value, value, value, value, value }
#endif


/* used by set.c */
#define SET_DEFFORMAT(axis, flag_array)				\
	if (flag_array[axis]) {					\
	    (void) strcpy(axis_array[axis].formatstring,DEF_FORMAT);	\
	    axis_array[axis].format_is_numeric = 1;		\
	}


/* 'roundoff' check tolerance: less than one hundredth of a tic mark */
#define SIGNIF (0.01)
/* (DFK) Watch for cancellation error near zero on axes labels */
/* FIXME HBB 20000521: these seem not to be used much, anywhere... */
#define CheckZero(x,tic) (fabs(x) < ((tic) * SIGNIF) ? 0.0 : (x))
#define NearlyEqual(x,y,tic) (fabs((x)-(y)) < ((tic) * SIGNIF))


	
/* ------------ functions exported by axis.c */
t_autoscale load_range __PROTO((AXIS_INDEX, double *, double *, t_autoscale));
void axis_unlog_interval __PROTO((AXIS_INDEX, double *, double *, TBOOLEAN));
void axis_revert_and_unlog_range __PROTO((AXIS_INDEX));
double axis_log_value_checked __PROTO((AXIS_INDEX, double, const char *));
void axis_checked_extend_empty_range __PROTO((AXIS_INDEX, const char *mesg));
char * copy_or_invent_formatstring __PROTO((AXIS_INDEX));
double quantize_normal_tics __PROTO((double, int));
void setup_tics __PROTO((AXIS_INDEX, int));
void gen_tics __PROTO((AXIS_INDEX, /* int, */ tic_callback));
void axis_output_tics __PROTO((AXIS_INDEX, int *, AXIS_INDEX, tic_callback));
void axis_set_graphical_range __PROTO((AXIS_INDEX, unsigned int lower, unsigned int upper));
void axis_draw_2d_zeroaxis __PROTO((AXIS_INDEX, AXIS_INDEX));
TBOOLEAN some_grid_selected __PROTO((void));

double get_writeback_min __PROTO((AXIS_INDEX));
double get_writeback_max __PROTO((AXIS_INDEX));
void set_writeback_min __PROTO((AXIS_INDEX));
void set_writeback_max __PROTO((AXIS_INDEX));

/* set widest_tic_label: length of the longest tics label */
void widest_tic_callback __PROTO((AXIS_INDEX, double place, char *text, struct lp_style_type grid));

/* ------------ autoscaling of the color axis */
#ifdef PM3D

#define NEED_PALETTE(plot) \
   (PM3DSURFACE == (plot)->plot_style \
    || PM3D_IMPLICIT == pm3d.implicit \
    || 1 == (plot)->lp_properties.use_palette)
int set_pm3d_zminmax __PROTO((void));

#else

/* make this available regardless of the status of PM3D itself, for
 * ease of use */
#define NEED_PALETTE(plot) FALSE

#endif /* PM3D */

/* exported by set.c but used for axis tic marks */
void add_tic_user __PROTO((AXIS_INDEX, char * label, double position));

#endif /* GNUPLOT_AXIS_H */
