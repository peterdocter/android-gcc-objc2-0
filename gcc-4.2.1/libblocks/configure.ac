# Process this file with autoconf to produce a configure script.
#   Copyright 2010 Versv LLC
#   Originally contributed by Philippe Hausler (philippe@versv.com).
#
#This file is part of Android NDK GCC ObjC 2.0 fork.
#
#GCC is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2, or (at your option)
#any later version.
#
#GCC is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with GCC; see the file COPYING.  If not, write to
#the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
#02110-1301, USA.

AC_PREREQ(2.59)
AC_INIT(package-unused, version-unused,, libblocks)
AC_CONFIG_HEADER(config.h)

# We need the following definitions because AC_PROG_LIBTOOL relies on them
PACKAGE=libblocks
# Version is pulled out to make it a bit easier to change using sed.
VERSION=1:0:0
AC_SUBST(VERSION)

# --------
# Programs
# --------

GCC_NO_EXECUTABLES

# We must force CC to /not/ be a precious variable; otherwise
# the wrong, non-multilib-adjusted value will be used in multilibs.
# As a side effect, we have to subst CFLAGS ourselves.
m4_rename([_AC_ARG_VAR_PRECIOUS],[real_PRECIOUS])
m4_define([_AC_ARG_VAR_PRECIOUS],[])
AC_PROG_CC
m4_rename([real_PRECIOUS],[_AC_ARG_VAR_PRECIOUS])

AC_SUBST(CFLAGS)

AC_CHECK_TOOL(AS, as)
AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(RANLIB, ranlib, :)
AC_PROG_INSTALL

AC_PROG_LIBTOOL

AM_PROG_CC_C_O

AC_PROG_MAKE_SET

AC_HEADER_STDC

AC_CHECK_HEADERS(AvailabilityMacros.h)
AC_CHECK_HEADERS(TargetConditionals.h)
AC_CHECK_HEADERS(libkern/OSAtomic.h)
AC_CHECK_FUNCS(__sync_bool_compare_and_swap)
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
