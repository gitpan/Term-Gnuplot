#ifndef USE_JUNK

char *
alloc(size,name)
     unsigned long size;
     char *name;
{
  return malloc((size_t)size);
}

void *
const_express() {return NULL;}

extern int term;
int term;
float xsize=1.0, ysize=1.0;		/* During test! */
FILE *outfile = stdout;
char term_options[4] = "";
jmp_buf env;
char outstr[] = "'Perl'";

char *input_line;
int inline_num;          /* from command.c */
int interactive;    /* from plot.c */
char *infile_name;       /* from plot.c */

/* Not used: */

char *token;
long c_token, num_tokens;

#endif /* !defined(USE_JUNK) */


/* Cannot pull the whole plot.h, too many contradictions. */

#ifdef __ZTC__
typedef int (*FUNC_PTR)(...);
#else
typedef int (*FUNC_PTR)();
#endif

struct TERMENTRY {
        char *name;
#if defined(_Windows) && !defined(WIN32)
        char GPFAR description[80];     /* to make text go in FAR segment */
#else
        char *description;
#endif
        unsigned int xmax,ymax,v_char,h_char,v_tic,h_tic;
        FUNC_PTR options,init,reset,text,scale,graphics,move,vector,linetype,
                put_text,text_angle,justify_text,point,arrow;
};

#ifdef _Windows
#define termentry TERMENTRY far
#else
#define termentry TERMENTRY
#endif

extern struct termentry term_tbl[];
 
#define RETVOID 
#define RETINT , 1

#define F_0 void(*)()
#define F_1 void(*)(int)
#define F_1I int(*)(int)
#define F_2 void(*)(unsigned int,unsigned int)
#define F_2D int(*)(double,double)
#define F_3 void(*)(unsigned int,unsigned int,int)
#define F_3T void(*)(int,int,char*)
#define F_4 void(*)(int,int,int,int)
#define F_5 void(*)(int,int,int,int,int)

#define CALL_G_METH0(method) CALL_G_METH(method,0,(),RETVOID)
#define CALL_G_METH1(method,arg1) CALL_G_METH(method,1,(arg1),RETVOID)
#define CALL_G_METH1I(method,arg1) CALL_G_METH(method,1I,(arg1),RETINT)
#define CALL_G_METH2(method,arg1,arg2) \
		CALL_G_METH(method,2,(arg1,arg2),RETVOID)
#define CALL_G_METH2D(method,arg1,arg2) \
		CALL_G_METH(method,2D,(arg1,arg2),RETINT)
#define CALL_G_METH3(method,arg1,arg2,arg3) \
		CALL_G_METH(method,3,(arg1,arg2,arg3),RETVOID)
#define CALL_G_METH3T(method,arg1,arg2,arg3) \
		CALL_G_METH(method,3T,(arg1,arg2,arg3),RETVOID)
#define CALL_G_METH4(method,arg1,arg2,arg3,arg4) \
		CALL_G_METH(method,4,(arg1,arg2,arg3,arg4,arg5),RETVOID)
#define CALL_G_METH5(method,arg1,arg2,arg3,arg4,arg5) \
		CALL_G_METH(method,5,(arg1,arg2,arg3,arg4,arg5),RETVOID)

#define CALL_G_METH(method,mult,args,returnval)    (		\
       (term<0) ? (						\
	 croak("No terminal specified") returnval		\
       ) :							\
       ((CAT2(F_,mult))term_tbl[term].method)args		\
     )

#define init()	CALL_G_METH0(init)
#define reset()	CALL_G_METH0(reset)
#define text()	CALL_G_METH0(text)
#define graphics()	CALL_G_METH0(graphics)
#define linetype(lt)	CALL_G_METH1(linetype,lt)
#define justify_text(mode)	CALL_G_METH1I(justify_text,mode)
#define text_angle(ang)	CALL_G_METH1I(text_angle,ang)
#define scale(xs,ys)	CALL_G_METH2D(scale,xs,ys)
#define move(x,y)	CALL_G_METH2(move,x,y)
#define vector(x,y)	CALL_G_METH2(vector,x,y)
#define put_text(x,y,str)	CALL_G_METH3T(put_text,x,y,str)
#define point(x,y,point)	CALL_G_METH3(point,x,y,point)
#define arrow(sx,sy,ex,ey,head)	CALL_G_METH5(arrow,sx,sy,ex,ey,head)
