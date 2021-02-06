/******************************************************************************\
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

/**
 *   @file trace.c
 *
 *	This file defines the implementation for the tracing facility of the
 *	embedded lib.
 *
 *****************************************************************************/


#include "trace.h"

#include <stdarg.h>

#include "dct_assert.h"

#ifndef NDEBUG

static int glb_level = MAX_LEVEL;

static Tracer*	tracerListHead = NULL;

// use macro instead of variable, or build error as "variable length array folded to constant array as an extension"
#define BUFFSIZE  1024

/*	For some stupid reason beyond my imagination, gccs stdio.h doesnt	*/
/*	support vsnprintf() in strict c99 mode. Only happens in cygwin.		*/
#if defined(__GNUC__) && defined(__CYGWIN__) && !defined(PICO)
    int vsnprintf(char *, size_t, const char *, __VALIST);
#endif


int getTraceLevel(void)
{
	return glb_level;
}
void setTraceLevel(int new_level)
{
    if (TRACE_OFF != new_level)
    {
        new_level = (~(((uint32_t)(new_level))-1u)) & MAX_LEVEL;
    }
    glb_level = new_level;
}

void enableTracer(Tracer *t)
{
    DCT_ASSERT(t);
    t->enabled = 1;
}


void disableTracer(Tracer *t)
{
    DCT_ASSERT(t);
    t->enabled = 0;
}


void setTracerFile(Tracer* t, FILE* f)
{
    DCT_ASSERT(t);
    t->fp = f;
}

void flushTracer(const Tracer *t)
{
    if (t->fp)
    {
        (void) fflush(t->fp);
    }
}

static void addToList(Tracer* tracer)
{
    if (tracerListHead)
    {
        tracer->next = tracerListHead;
    }
    tracer->linked = 1;
    tracerListHead = tracer;
}

Tracer* getTracerList(void)
{
    return tracerListHead;
}

void trace( Tracer* tracer, const CHAR* sFormat, ...)
{
    char buffer[BUFFSIZE];
    int length;
    va_list args;

    DCT_ASSERT(tracer);

    if (!tracer->linked)
    {
        addToList(tracer);
    }
    if ((tracer->level & glb_level) && (tracer->enabled != 0))
    {
        va_start(args, sFormat);
        length = vsnprintf(buffer, BUFFSIZE, sFormat, args);
        if (!((length > 0) && (length < BUFFSIZE)))
        {
            /* message was truncated */
            fprintf(stderr, "Warning: Trace output truncated !");
        }
        va_end(args);

        if (tracer->fp == 0)
        {
            tracer->fp = stdout;
        }

        fprintf(tracer->fp, "%s%s", tracer->prefix, buffer);
        (void) fflush(tracer->fp);
    }
}

#endif	/* NDEBUG */
