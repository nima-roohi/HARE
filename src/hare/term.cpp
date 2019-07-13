/**************************************************************************************************
 *                           HARE: Hybrid Abstraction Refinement Engine                           *
 *                                                                                                *
 * Copyright (C) 2019                                                                             *
 * Authors:                                                                                       *
 *   Nima Roohi <nroohi@ucsd.edu> (University of California San Diego)                            *
 *                                                                                                *
 * This program is free software: you can redistribute it and/or modify it under the terms        *
 * of the GNU General Public License as published by the Free Software Foundation, either         *
 * version 3 of the License, or (at your option) any later version.                               *
 *                                                                                                *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      *
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 * See the GNU General Public License for more details.                                           *
 *                                                                                                *
 * You should have received a copy of the GNU General Public License along with this program.     *
 * If not, see <https://www.gnu.org/licenses/>.                                                   *
 **************************************************************************************************/

/** Unfortunately I keep getting "undefined reference error" for the following methods in Ubuntu. So I added them only for this OS. */
#ifdef __linux__

#include "cmn/dbc.hpp"

#include <gmp.h>
#include <gmpxx.h>

#include <algorithm>
#include <ostream>
#include <iostream>

__GMP_DECLSPEC extern void * (*__gmp_allocate_func) (size_t);
__GMP_DECLSPEC extern void * (*__gmp_reallocate_func) (void *, size_t, size_t);
__GMP_DECLSPEC extern void   (*__gmp_free_func) (void *, size_t);
extern const struct doprnt_funs_t  __gmp_asprintf_funs_noformat;

#define DOPRNT_CONV_FIXED        1
#define DOPRNT_CONV_SCIENTIFIC   2
#define DOPRNT_CONV_GENERAL      3

#define DOPRNT_JUSTIFY_NONE      0
#define DOPRNT_JUSTIFY_LEFT      1
#define DOPRNT_JUSTIFY_RIGHT     2
#define DOPRNT_JUSTIFY_INTERNAL  3

#define DOPRNT_SHOWBASE_YES      1
#define DOPRNT_SHOWBASE_NO       2
#define DOPRNT_SHOWBASE_NONZERO  3

#undef MAX
#define MAX(h,i) ((h) > (i) ? (h) : (i))

#define GMP_ASPRINTF_T_INIT(d, output)					            \
  do {									                                    \
    (d).result = (output);						                      \
    (d).alloc = 256;							                          \
    (d).buf = (char *) (*__gmp_allocate_func) ((d).alloc);	\
    (d).size = 0;							                              \
  } while (0)

#define __GMP_REALLOCATE_FUNC_MAYBE_TYPE(ptr, oldsize, newsize, type)	\
  do {									                                              \
    if ((oldsize) != (newsize))						                            \
      (ptr) = (type *) (*__gmp_reallocate_func)				                \
	(ptr, (oldsize) * sizeof (type), (newsize) * sizeof (type));	      \
  } while (0)

#define DOPRNT_ACCUMULATE(call)						        \
  do {									                          \
    int  __ret;								                    \
    __ret = call;							                    \
    if (__ret == -1)							                \
      goto error;							                    \
    retval += __ret;							                \
  } while (0)

#define DOPRNT_ACCUMULATE_FUN(fun, params)				\
  do {									                          \
    asserts ((fun) != NULL);						          \
    DOPRNT_ACCUMULATE ((*(fun)) params);				  \
  } while (0)

#define DOPRNT_FORMAT(fmt, ap)						\
  DOPRNT_ACCUMULATE_FUN (funs->format, (data, fmt, ap))

#define DOPRNT_MEMORY(ptr, len)						\
  DOPRNT_ACCUMULATE_FUN (funs->memory, (data, ptr, len))

#define DOPRNT_REPS(c, n)						      \
  DOPRNT_ACCUMULATE_FUN (funs->reps, (data, c, n))

#define DOPRNT_STRING(str) DOPRNT_MEMORY (str, strlen (str))

#define DOPRNT_REPS_MAYBE(c, n)						\
  do {									                  \
    if ((n) != 0)							            \
      DOPRNT_REPS (c, n);						      \
  } while (0)

#define DOPRNT_MEMORY_MAYBE(ptr, len)			\
  do {									                  \
    if ((len) != 0)							          \
      DOPRNT_MEMORY (ptr, len);						\
  } while (0)

typedef int (*doprnt_format_t) (void *, const char *, va_list);
typedef int (*doprnt_memory_t) (void *, const char *, size_t);
typedef int (*doprnt_reps_t)   (void *, int, int);
typedef int (*doprnt_final_t)  (void *);

struct doprnt_funs_t {
  doprnt_format_t  format;
  doprnt_memory_t  memory;
  doprnt_reps_t    reps;
  doprnt_final_t   final;   /* NULL if not required */
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

class gmp_allocated_string {
 public:
  char *str;
  size_t len;
  gmp_allocated_string(char *arg) {
    str = arg;
    len = std::strlen (str); }
  ~gmp_allocated_string() { (*__gmp_free_func) (str, len+1); }
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct gmp_asprintf_t {
  char    **result;
  char    *buf;
  size_t  size;
  size_t  alloc;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

struct doprnt_params_t {
  int         base;          /* negative for upper case */
  int         conv;          /* choices above */
  const char  *expfmt;       /* exponent format */
  int         exptimes4;     /* exponent multiply by 4 */
  char        fill;          /* character */
  int         justify;       /* choices above */
  int         prec;          /* prec field, or -1 for all digits */
  int         showbase;      /* choices above */
  int         showpoint;     /* if radix point always shown */
  int         showtrailing;  /* if trailing zeros wanted */
  char        sign;          /* '+', ' ', or '\0' */
  int         width;         /* width field */
};

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void __gmp_doprnt_params_from_ios (struct doprnt_params_t *p, std::ios &o) {
  if ((o.flags() & std::ios::basefield) == std::ios::hex) {
      p->expfmt = "@%c%02d";
      p->base = (o.flags() & std::ios::uppercase ? -16 : 16); 
  } else {
      p->expfmt = (o.flags() & std::ios::uppercase ? "E%c%02d" : "e%c%02d");
      if ((o.flags() & std::ios::basefield) == std::ios::oct)
        p->base = 8;
      else
        p->base = 10;
    }

  /* "general" if none or more than one bit set */
  if ((o.flags() & std::ios::floatfield) == std::ios::fixed)
    p->conv = DOPRNT_CONV_FIXED;
  else if ((o.flags() & std::ios::floatfield) == std::ios::scientific)
    p->conv = DOPRNT_CONV_SCIENTIFIC;
  else
    p->conv = DOPRNT_CONV_GENERAL;

  p->exptimes4 = 0;
  p->fill = o.fill();

  /* "right" if more than one bit set */
  if ((o.flags() & std::ios::adjustfield) == std::ios::left)
    p->justify = DOPRNT_JUSTIFY_LEFT;
  else if ((o.flags() & std::ios::adjustfield) == std::ios::internal)
    p->justify = DOPRNT_JUSTIFY_INTERNAL;
  else
    p->justify = DOPRNT_JUSTIFY_RIGHT;

  /* std::ios::fixed allows prec==0, others take 0 as the default 6.
     Don't allow negatives (they do bad things to __gmp_doprnt_float_cxx).  */
  p->prec = MAX(0, o.precision());
  if (p->prec == 0 && p->conv != DOPRNT_CONV_FIXED)
    p->prec = 6;

  /* for hex showbase is always, for octal only non-zero */
  if (o.flags() & std::ios::showbase)
    p->showbase = ((o.flags() & std::ios::basefield) == std::ios::hex
                   ? DOPRNT_SHOWBASE_YES : DOPRNT_SHOWBASE_NONZERO);
  else
    p->showbase = DOPRNT_SHOWBASE_NO;

  p->showpoint = ((o.flags() & std::ios::showpoint) != 0);

  /* in fixed and scientific always show trailing zeros, in general format
     show them if showpoint is set (or so it seems) */
  if ((o.flags() & std::ios::floatfield) == std::ios::fixed
      || (o.flags() & std::ios::floatfield) == std::ios::scientific)
    p->showtrailing = 1;
  else
    p->showtrailing = p->showpoint;

  p->sign = (o.flags() & std::ios::showpos ? '+' : '\0');

  p->width = o.width();

  /* reset on each output */
  o.width (0);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

int __gmp_asprintf_final (struct gmp_asprintf_t *d) {
  char  *buf = d->buf;
  asserts(d->alloc >= d->size + 1)
  buf[d->size] = '\0';
  __GMP_REALLOCATE_FUNC_MAYBE_TYPE (buf, d->alloc, d->size+1, char);
  *d->result = buf;
  return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

int __gmp_doprnt_integer (const struct doprnt_funs_t *funs,
          void *data,
          const struct doprnt_params_t *p,
          const char *s)
{
  int         retval = 0;
  int         slen, justlen, showbaselen, sign, signlen, slashlen, zeros;
  int         justify, den_showbaselen;
  const char  *slash, *showbase;

  /* '+' or ' ' if wanted, and don't already have '-' */
  sign = p->sign;
  if (s[0] == '-')
    {
      sign = s[0];
      s++;
    }
  signlen = (sign != '\0');

  /* if the precision was explicitly 0, print nothing for a 0 value */
  if (*s == '0' && p->prec == 0)
    s++;

  slen = strlen (s);
  slash = strchr (s, '/');

  showbase = NULL;
  showbaselen = 0;

  if (p->showbase != DOPRNT_SHOWBASE_NO)
    {
      switch (p->base) {
      case 16:  showbase = "0x"; showbaselen = 2; break;
      case -16: showbase = "0X"; showbaselen = 2; break;
      case 8:   showbase = "0";  showbaselen = 1; break;
      }
    }

  den_showbaselen = showbaselen;
  if (slash == NULL
      || (p->showbase == DOPRNT_SHOWBASE_NONZERO && slash[1] == '0'))
    den_showbaselen = 0;

  if (p->showbase == DOPRNT_SHOWBASE_NONZERO && s[0] == '0')
    showbaselen = 0;

  /* the influence of p->prec on mpq is currently undefined */
  zeros = MAX (0, p->prec - slen);

  /* space left over after actual output length */
  justlen = p->width
    - (strlen(s) + signlen + showbaselen + den_showbaselen + zeros);

  justify = p->justify;
  if (justlen <= 0) /* no justifying if exceed width */
    justify = DOPRNT_JUSTIFY_NONE;

  if (justify == DOPRNT_JUSTIFY_RIGHT)             /* pad right */
    DOPRNT_REPS (p->fill, justlen);

  DOPRNT_REPS_MAYBE (sign, signlen);               /* sign */

  DOPRNT_MEMORY_MAYBE (showbase, showbaselen);     /* base */

  DOPRNT_REPS_MAYBE ('0', zeros);                  /* zeros */

  if (justify == DOPRNT_JUSTIFY_INTERNAL)          /* pad internal */
    DOPRNT_REPS (p->fill, justlen);

  /* if there's a showbase on the denominator, then print the numerator
     separately so it can be inserted */
  if (den_showbaselen != 0)
    {
      asserts (slash != NULL);
      slashlen = slash+1 - s;
      DOPRNT_MEMORY (s, slashlen);                 /* numerator and slash */
      slen -= slashlen;
      s += slashlen;
      DOPRNT_MEMORY (showbase, den_showbaselen);
    }

  DOPRNT_MEMORY (s, slen);                         /* number, or denominator */

  if (justify == DOPRNT_JUSTIFY_LEFT)              /* pad left */
    DOPRNT_REPS (p->fill, justlen);

 done:
  return retval;

 error:
  retval = -1;
  goto done;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

std::ostream& __gmp_doprnt_integer_ostream(std::ostream &o, struct doprnt_params_t *p, char *s) {
  struct gmp_asprintf_t   d;
  char  *result;
  int   ret;

  /* don't show leading zeros the way printf does */
  p->prec = -1;

  GMP_ASPRINTF_T_INIT (d, &result);
  ret = __gmp_doprnt_integer (&__gmp_asprintf_funs_noformat, &d, p, s);
  asserts(ret != -1)
  __gmp_asprintf_final (&d);
  (*__gmp_free_func) (s, strlen(s)+1);

  gmp_allocated_string  t (result);
  return o.write (t.str, t.len);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, mpz_srcptr num) { 
 struct doprnt_params_t  param;
 __gmp_doprnt_params_from_ios (&param, os);
 return __gmp_doprnt_integer_ostream (os, &param, mpz_get_str (NULL, param.base, num)); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os,  mpq_srcptr num) { 
  struct doprnt_params_t  param;
  __gmp_doprnt_params_from_ios (&param, os);
  return __gmp_doprnt_integer_ostream (os, &param, mpq_get_str (NULL, param.base, num)); }

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef MAX

#endif // __linux__

