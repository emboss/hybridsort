#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#ifndef S_ISDIR
#   define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#include <ruby.h>

#define STRING_P(s) (RB_TYPE_P((s), T_STRING) && CLASS_OF(s) == rb_cString)

static ID id_cmp;

/* mm.c */

#define mmtype long
#define mmcount (16 / SIZEOF_LONG)
#define A ((mmtype*)a)
#define B ((mmtype*)b)
#define C ((mmtype*)c)
#define D ((mmtype*)d)

#define mmstep (sizeof(mmtype) * mmcount)
#define mmprepare(base, size) do {\
 if (((VALUE)(base) % sizeof(mmtype)) == 0 && ((size) % sizeof(mmtype)) == 0) \
   if ((size) >= mmstep) mmkind = 1;\
   else              mmkind = 0;\
 else                mmkind = -1;\
 high = ((size) / mmstep) * mmstep;\
 low  = ((size) % mmstep);\
} while (0)\

#define mmarg mmkind, size, high, low
#define mmargdecl int mmkind, size_t size, size_t high, size_t low

static void mmswap_(register char *a, register char *b, mmargdecl)
{
 if (a == b) return;
 if (mmkind >= 0) {
   register mmtype s;
#if mmcount > 1
   if (mmkind > 0) {
     register char *t = a + high;
     do {
       s = A[0]; A[0] = B[0]; B[0] = s;
       s = A[1]; A[1] = B[1]; B[1] = s;
#if mmcount > 2
       s = A[2]; A[2] = B[2]; B[2] = s;
#if mmcount > 3
       s = A[3]; A[3] = B[3]; B[3] = s;
#endif
#endif
       a += mmstep; b += mmstep;
     } while (a < t);
   }
#endif
   if (low != 0) { s = A[0]; A[0] = B[0]; B[0] = s;
#if mmcount > 2
     if (low >= 2 * sizeof(mmtype)) { s = A[1]; A[1] = B[1]; B[1] = s;
#if mmcount > 3
       if (low >= 3 * sizeof(mmtype)) {s = A[2]; A[2] = B[2]; B[2] = s;}
#endif
     }
#endif
   }
 }
 else {
   register char *t = a + size, s;
   do {s = *a; *a++ = *b; *b++ = s;} while (a < t);
 }
}
#define mmswap(a,b) mmswap_((a),(b),mmarg)

/* a, b, c = b, c, a */
static void mmrot3_(register char *a, register char *b, register char *c, mmargdecl)
{
 if (mmkind >= 0) {
   register mmtype s;
#if mmcount > 1
   if (mmkind > 0) {
     register char *t = a + high;
     do {
       s = A[0]; A[0] = B[0]; B[0] = C[0]; C[0] = s;
       s = A[1]; A[1] = B[1]; B[1] = C[1]; C[1] = s;
#if mmcount > 2
       s = A[2]; A[2] = B[2]; B[2] = C[2]; C[2] = s;
#if mmcount > 3
       s = A[3]; A[3] = B[3]; B[3] = C[3]; C[3] = s;
#endif
#endif
       a += mmstep; b += mmstep; c += mmstep;
     } while (a < t);
   }
#endif
   if (low != 0) { s = A[0]; A[0] = B[0]; B[0] = C[0]; C[0] = s;
#if mmcount > 2
     if (low >= 2 * sizeof(mmtype)) { s = A[1]; A[1] = B[1]; B[1] = C[1]; C[1] = s;
#if mmcount > 3
       if (low == 3 * sizeof(mmtype)) {s = A[2]; A[2] = B[2]; B[2] = C[2]; C[2] = s;}
#endif
     }
#endif
   }
 }
 else {
   register char *t = a + size, s;
   do {s = *a; *a++ = *b; *b++ = *c; *c++ = s;} while (a < t);
 }
}
#define mmrot3(a,b,c) mmrot3_((a),(b),(c),mmarg)

/* a = b */
static void mmassign_(register char *a, register char *b, mmargdecl)
{
 if (a == b) return;
 if (mmkind >= 0) {
#if mmcount > 1
   if (mmkind > 0) {
     register char *t = a + high;
     do {
       A[0] = B[0];
       A[1] = B[1];
#if mmcount > 2
       A[2] = B[2];
#if mmcount > 3
       A[3] = B[3];
#endif
#endif
       a += mmstep; b += mmstep;
     } while (a < t);
   }
#endif
   if (low != 0) { A[0] = B[0];
#if mmcount > 2
     if (low >= 2 * sizeof(mmtype)) { A[1] = B[1];
#if mmcount > 3
       if (low >= 3 * sizeof(mmtype)) { A[2] = B[2]; }
#endif
     }
#endif
   }
 }
 else {
   register char *t = a + size;
   do { *a++ = *b++;} while (a < t);
 }
}
#define mmassign(a, b) mmassign_((a),(b),mmarg)

/* qs6.c */
/*****************************************************/
/*                                                   */
/*          qs6   (Quick sort function)              */
/*                                                   */
/* by  Tomoyuki Kawamura              1995.4.21      */
/* kawamura@tokuyama.ac.jp                           */
/*****************************************************/

typedef struct { char *LL, *RR; } stack_node; /* Stack structure for L,l,R,r */
#define PUSH(ll,rr) do { top->LL = (ll); top->RR = (rr); ++top; } while (0)  /* Push L,l,R,r */
#define POP(ll,rr)  do { --top; (ll) = top->LL; (rr) = top->RR; } while (0)      /* Pop L,l,R,r */

#define med3(a,b,c) ((*cmp)((a),(b),d)<0 ?                                   \
                       ((*cmp)((b),(c),d)<0 ? (b) : ((*cmp)((a),(c),d)<0 ? (c) : (a))) : \
                       ((*cmp)((b),(c),d)>0 ? (b) : ((*cmp)((a),(c),d)<0 ? (a) : (c))))

typedef int (cmpfunc_t)(const void*, const void*, void*);

static void
insertion_sort(char *l, char *r, cmpfunc_t *cmp, void *d, mmargdecl)
{
    char *p, *q, *v;
    v = ALLOCA_N(char, size);

    for(p = l + size; p <= r; p += size) {
	mmassign(v, p);
        for (q = p - size; q >= l && cmp(q, v, d) > 0; q -= size) {
	    mmassign(q+size, q);
        }
	if (q+size != p)
	    mmassign(q+size, v);
    }
}

void
rqsort(void* base, const size_t nel, const size_t size, cmpfunc_t *cmp, void *d, size_t cutoff)
{
  register char *l, *r, *m;          	  /* l,r:left,right group   m:median point */
  register int t, eq_l, eq_r;       	  /* eq_l: all items in left group are equal to S */
  char *end = (char*)base + size*(nel-1); /* right end of current region */
  char *L = base;                    	  /* left end of current region */
  char *R = end; 			  /* right end of current region */
  size_t chklim = 63;                     /* threshold of ordering element check */
  stack_node stack[32], *top = stack;     /* 32 is enough for 32bit CPU */
  int mmkind;
  size_t high, low, n;

  if (nel <= 1) return;        /* need not to sort */
  mmprepare(base, size);
  goto start;

  nxt:
  if (stack == top) {
      /* stack is empty - run insertion sort over nearly-sorted array to finalize */
      insertion_sort(base, end, cmp, d, mmarg);
      return;
  }
  POP(L,R);

  for (;;) {
    start:

    if (L + size == R) {       /* 2 elements */
      if ((*cmp)(L,R,d) > 0) mmswap(L,R); goto nxt;
    }

    l = L; r = R;
    n = (r - l + size) / size;  /* number of elements */
    m = l + size * (n >> 1);    /* calculate median value */

    if (n <= cutoff) /* Leave chunks smaller than 'cutoff' unsorted */
	goto nxt;

    if (n >= 60) {
      register char *m1;
      register char *m3;
      if (n >= 200) {
	n = size*(n>>3); /* number of bytes in splitting 8 */
	{
	  register char *p1 = l  + n;
	  register char *p2 = p1 + n;
	  register char *p3 = p2 + n;
	  m1 = med3(p1, p2, p3);
	  p1 = m  + n;
	  p2 = p1 + n;
	  p3 = p2 + n;
	  m3 = med3(p1, p2, p3);
	}
      }
      else {
	n = size*(n>>2); /* number of bytes in splitting 4 */
	m1 = l + n;
	m3 = m + n;
      }
      m = med3(m1, m, m3);
    }

    if ((t = (*cmp)(l,m,d)) < 0) {                           /*3-5-?*/
      if ((t = (*cmp)(m,r,d)) < 0) {                         /*3-5-7*/
	if (chklim && nel >= chklim) {   /* check if already ascending order */
	  char *p;
	  chklim = 0;
	  for (p=l; p<r; p+=size) if ((*cmp)(p,p+size,d) > 0) goto fail;
	  goto nxt;
	}
	fail: goto loopA;                                    /*3-5-7*/
      }
      if (t > 0) {
	if ((*cmp)(l,r,d) <= 0) {mmswap(m,r); goto loopA;}     /*3-5-4*/
	mmrot3(r,m,l); goto loopA;                           /*3-5-2*/
      }
      goto loopB;                                            /*3-5-5*/
    }

    if (t > 0) {                                             /*7-5-?*/
      if ((t = (*cmp)(m,r,d)) > 0) {                         /*7-5-3*/
	if (chklim && nel >= chklim) {   /* check if already ascending order */ /*wrong: must be descending!*/
	  char *p;
	  chklim = 0;
	  for (p=l; p<r; p+=size) if ((*cmp)(p,p+size,d) < 0) goto fail2;
	  while (l<r) {mmswap(l,r); l+=size; r-=size;}  /* reverse region */
	  goto nxt;
	}
	fail2: mmswap(l,r); goto loopA;                      /*7-5-3*/
      }
      if (t < 0) {
	if ((*cmp)(l,r,d) <= 0) {mmswap(l,m); goto loopB;}   /*7-5-8*/
	mmrot3(l,m,r); goto loopA;                           /*7-5-6*/
      }
      mmswap(l,r); goto loopA;                               /*7-5-5*/
    }

    if ((t = (*cmp)(m,r,d)) < 0)  {goto loopA;}              /*5-5-7*/
    if (t > 0) {mmswap(l,r); goto loopB;}                    /*5-5-3*/

    /* determining splitting type in case 5-5-5 */           /*5-5-5*/
    for (;;) {
      if ((l += size) == r)      goto nxt;                   /*5-5-5*/
      if (l == m) continue;
      if ((t = (*cmp)(l,m,d)) > 0) {mmswap(l,r); l = L; goto loopA;}/*575-5*/
      if (t < 0)                 {mmswap(L,l); l = L; goto loopB;}  /*535-5*/
    }

    loopA: eq_l = 1; eq_r = 1;  /* splitting type A */ /* left <= median < right */
    for (;;) {
      for (;;) {
	if ((l += size) == r)
	  {l -= size; if (l != m) mmswap(m,l); l -= size; goto fin;}
	if (l == m) continue;
	if ((t = (*cmp)(l,m,d)) > 0) {eq_r = 0; break;}
	if (t < 0) eq_l = 0;
      }
      for (;;) {
	if (l == (r -= size))
	  {l -= size; if (l != m) mmswap(m,l); l -= size; goto fin;}
	if (r == m) {m = l; break;}
	if ((t = (*cmp)(r,m,d)) < 0) {eq_l = 0; break;}
	if (t == 0) break;
      }
      mmswap(l,r);    /* swap left and right */
    }

    loopB: eq_l = 1; eq_r = 1;  /* splitting type B */ /* left < median <= right */
    for (;;) {
      for (;;) {
	if (l == (r -= size))
	  {r += size; if (r != m) mmswap(r,m); r += size; goto fin;}
	if (r == m) continue;
	if ((t = (*cmp)(r,m,d)) < 0) {eq_l = 0; break;}
	if (t > 0) eq_r = 0;
      }
      for (;;) {
	if ((l += size) == r)
	  {r += size; if (r != m) mmswap(r,m); r += size; goto fin;}
	if (l == m) {m = r; break;}
	if ((t = (*cmp)(l,m,d)) > 0) {eq_r = 0; break;}
	if (t == 0) break;
      }
      mmswap(l,r);    /* swap left and right */
    }

    fin:
    if (eq_l == 0)                         /* need to sort left side */
      if (eq_r == 0)                       /* need to sort right side */
	if (l-L < R-r) {PUSH(r,R); R = l;} /* sort left side first */
	else           {PUSH(L,l); L = r;} /* sort right side first */
      else R = l;                          /* need to sort left side only */
    else if (eq_r == 0) L = r;             /* need to sort right side only */
    else goto nxt;                         /* need not to sort both sides */
  }
}

static int
sort_1(const void *ap, const void *bp, void *dummy)
{
    VALUE retval;
    VALUE a = *(const VALUE *)ap, b = *(const VALUE *)bp;

    retval = rb_yield_values(2, a, b);
    return rb_cmpint(retval, a, b);
}

static int
sort_2(const void *ap, const void *bp, void *dummy)
{
    VALUE retval;
    VALUE a = *(const VALUE *)ap, b = *(const VALUE *)bp;

    if (FIXNUM_P(a) && FIXNUM_P(b)) {
	if ((long)a > (long)b) return 1;
	if ((long)a < (long)b) return -1;
	return 0;
    }
    if (STRING_P(a) && STRING_P(b)) {
	return rb_str_cmp(a, b);
    }

    retval = rb_funcall(a, id_cmp, 1, b);
    return rb_cmpint(retval, a, b);
}

static VALUE
hybrid_sort(VALUE self, VALUE ary, VALUE cutoff)
{
    ary = rb_ary_dup(ary);
    rqsort(RARRAY_PTR(ary), RARRAY_LEN(ary), sizeof(VALUE),
		   rb_block_given_p()?sort_1:sort_2, NULL, (size_t)NUM2LONG(cutoff));
    return ary;
}

void
Init_hybridsort(void)
{
    VALUE mHybrid = rb_define_module("Hybrid");
    id_cmp = rb_intern("<=>");
    rb_define_module_function(mHybrid, "sort", hybrid_sort, 2);
}

