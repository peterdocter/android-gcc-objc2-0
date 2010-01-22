/* Definitions for non-Linux based ARM systems using ELF
   Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004
   Free Software Foundation, Inc.
   Contributed by Catherine Moore <clm@cygnus.com>

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* elfos.h should have already been included.  Now just override
   any conflicting definitions and add any extras.  */

#undef TARGET_CPU_CPP_BUILTINS
#define TARGET_CPU_CPP_BUILTINS()		\
do						\
{						\
builtin_define ("__ANDROID__"); \
/* Define __arm__ even when in thumb mode, for	\
consistency with armcc.  */			\
builtin_define ("__arm__");			\
builtin_define ("__APCS_32__");			\
if (TARGET_THUMB)				\
builtin_define ("__thumb__");			\
\
if (TARGET_BIG_END)				\
{						\
builtin_define ("__ARMEB__");		\
if (TARGET_THUMB)				\
builtin_define ("__THUMBEB__");		\
if (TARGET_LITTLE_WORDS)			\
builtin_define ("__ARMWEL__");		\
}						\
else						\
{						\
builtin_define ("__ARMEL__");		\
if (TARGET_THUMB)				\
builtin_define ("__THUMBEL__");		\
}						\
\
if (TARGET_SOFT_FLOAT)				\
builtin_define ("__SOFTFP__");		\
\
if (TARGET_VFP)					\
builtin_define ("__VFP_FP__");		\
\
/* Add a define for interworking.		\
Needed when building libgcc.a.  */		\
if (arm_cpp_interwork)				\
builtin_define ("__THUMB_INTERWORK__");	\
\
builtin_assert ("cpu=arm");			\
builtin_assert ("machine=arm");			\
\
builtin_define (arm_arch_name);			\
if (arm_arch_cirrus)				\
builtin_define ("__MAVERICK__");		\
if (arm_arch_xscale)				\
builtin_define ("__XSCALE__");		\
if (arm_arch_iwmmxt)				\
builtin_define ("__IWMMXT__");		\
if (TARGET_AAPCS_BASED)				\
builtin_define ("__ARM_EABI__");		\
builtin_define ("__arm"); \
builtin_define ("__ARM_ARCH_5__");  \
builtin_define ("__ARM_ARCH_5T__"); \
builtin_define ("__ARM_ARCH_5E__"); \
builtin_define ("__ARM_ARCH_5TE__"); \
builtin_define ("__LITTLE_ENDIAN__"); \
if (flag_objc_gc || flag_objc_gc_only) { \
builtin_define ("__strong=__attribute__((objc_gc(strong)))"); \
builtin_define ("__weak=__attribute__((objc_gc(weak)))"); \
builtin_define ("__OBJC_GC__"); \
} else { \
builtin_define ("__strong="); \
builtin_define ("__weak=__attribute__((objc_gc(weak)))"); \
} \
if (flag_blocks) { \
builtin_define ("__block=__attribute__((__blocks__(byref)))"); \
} \
if (flag_objc_abi == 2) \
builtin_define ("__OBJC2__"); \
if (flag_objc_zerocost_exceptions) \
builtin_define ("OBJC_ZEROCOST_EXCEPTIONS"); \
builtin_define ("OBJC_NEW_PROPERTIES"); \
}						\
while (false)
