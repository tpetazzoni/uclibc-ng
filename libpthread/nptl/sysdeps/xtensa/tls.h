/* Definition for thread-local data handling.  NPTL/Xtensa version.
   Copyright (C) 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _TLS_H
#define _TLS_H

#ifndef __ASSEMBLER__

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

/* Type for the dtv.  */
typedef union dtv
{
  size_t counter;
  struct
  {
    void *val;
    bool is_static;
  } pointer;
} dtv_t;


#else /* __ASSEMBLER__ */
# include <tcb-offsets.h>
#endif /* __ASSEMBLER__ */

/* We require TLS support in the tools.  */
#define HAVE_TLS_SUPPORT                1
#define HAVE_TLS_MODEL_ATTRIBUTE        1
#define HAVE___THREAD                   1

/* Signal that TLS support is available.  */
#define USE_TLS	1

#ifndef __ASSEMBLER__

/* Get system call information.  */
# include <sysdep.h>

/* The TLS blocks start right after the TCB.  */
# define TLS_DTV_AT_TP	1

/* Get the thread descriptor definition.  */
# include <../../descr.h>

typedef struct
{
  dtv_t *dtv;
  void *private;
} tcbhead_t;

/* This is the size of the initial TCB.  */
# define TLS_INIT_TCB_SIZE	sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
# define TLS_INIT_TCB_ALIGN	16

/* This is the size of the TCB.  */
# define TLS_TCB_SIZE		sizeof (tcbhead_t)

/* This is the size we need before TCB.  */
# define TLS_PRE_TCB_SIZE	sizeof (struct pthread)

/* Alignment requirements for the TCB.  */
# define TLS_TCB_ALIGN		16


/* Install the dtv pointer.  The pointer passed is to the element with
   index -1 which contain the length.  */
# define INSTALL_DTV(tcbp, dtvp) \
  (((tcbhead_t *) (tcbp))->dtv = (dtvp) + 1)

/* Install new dtv for current thread.  */
# define INSTALL_NEW_DTV(dtv)						\
  ({ tcbhead_t *__tcbp;							\
     __asm__ __volatile__ ("rur %0, threadptr" : "=r" (__tcbp));	\
     __tcbp->dtv = (dtv); })						\

/* Return dtv of given thread descriptor.  */
# define GET_DTV(tcbp) \
  (((tcbhead_t *) (tcbp))->dtv)

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(tcbp, secondcall)					\
  ({ __asm__ __volatile__ ("wur %0, threadptr" : : "r" (tcbp)); 0; })

/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV()							\
  ({ tcbhead_t *__tcbp;							\
      __asm__ __volatile__ ("rur %0, threadptr" : "=r" (__tcbp));	\
      __tcbp->dtv; })

/* Return the thread descriptor for the current thread.  */
# define THREAD_SELF							\
 ({ struct pthread *__self;						\
    __asm__ ("rur %0, threadptr" : "=r" (__self));			\
    __self - 1; })

/* Magic for libthread_db to know how to do THREAD_SELF.  */
# define DB_THREAD_SELF \
  CONST_THREAD_AREA (32, sizeof (struct pthread))

/* Access to data in the thread descriptor is easy.  */
#define THREAD_GETMEM(descr, member) \
  descr->member
#define THREAD_GETMEM_NC(descr, member, idx) \
  descr->member[idx]
#define THREAD_SETMEM(descr, member, value) \
  descr->member = (value)
#define THREAD_SETMEM_NC(descr, member, idx, value) \
  descr->member[idx] = (value)

/* Get and set the global scope generation counter in struct pthread.  */
#define THREAD_GSCOPE_FLAG_UNUSED 0
#define THREAD_GSCOPE_FLAG_USED   1
#define THREAD_GSCOPE_FLAG_WAIT   2
#define THREAD_GSCOPE_RESET_FLAG() \
  do									     \
    { int __res								     \
	= atomic_exchange_rel (&THREAD_SELF->header.gscope_flag,	     \
			       THREAD_GSCOPE_FLAG_UNUSED);		     \
      if (__res == THREAD_GSCOPE_FLAG_WAIT)				     \
	lll_futex_wake (&THREAD_SELF->header.gscope_flag, 1, LLL_PRIVATE);   \
    }									     \
  while (0)
#define THREAD_GSCOPE_SET_FLAG() \
  do									     \
    {									     \
      THREAD_SELF->header.gscope_flag = THREAD_GSCOPE_FLAG_USED;	     \
      atomic_write_barrier ();						     \
    }									     \
  while (0)

#define THREAD_GSCOPE_WAIT() \
  GL(dl_wait_lookup_done) ()

#endif /* __ASSEMBLER__ */

#endif	/* tls.h */
