/*
 * Copyright (C) 2001 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef QAPPLICATION_H_
#define QAPPLICATION_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobject.h>
#include <qwidget.h>
#include <qpalette.h>
#include <qsize.h>

#if (defined(__APPLE__) && defined(__OBJC__) && defined(__cplusplus))
#import <Cocoa/Cocoa.h>
#endif


// class QApplication ==========================================================

class QApplication : public QObject {
public:

    // typedefs ----------------------------------------------------------------
    // enums -------------------------------------------------------------------
    // constants ---------------------------------------------------------------
    
    // static member functions -------------------------------------------------

    static QPalette palette(const QWidget *p=0);
    static QWidget *desktop();
    static int startDragDistance();
    static QSize globalStrut();
    static void	setOverrideCursor(const QCursor &);
    static void restoreOverrideCursor();
    static bool sendEvent(QObject *, QEvent *);
    static void sendPostedEvents(QObject *receiver, int event_type);

    // constructors, copy constructors, and destructors ------------------------

// add no-arg constructor
#ifdef _KWQ_PEDANTIC_
    QApplication() {}
#endif

    QApplication( int &argc, char **argv);
    virtual ~QApplication();

    // These two functions (exec and setMainWidget) are only used by our
    // test apps.
    int		     exec();
    virtual void     setMainWidget( QWidget * );

    // member functions --------------------------------------------------------
    // operators ---------------------------------------------------------------

// protected -------------------------------------------------------------------

// private ---------------------------------------------------------------------

private:
    // no copying or assignment
    // note that these are "standard" (no pendantic stuff needed)
    QApplication(const QApplication &);
    QApplication &operator=(const QApplication &);

    void _initialize();
    
#if (defined(__APPLE__) && defined(__OBJC__) && defined(__cplusplus))
    NSApplication *application;
    NSAutoreleasePool *globalPool;
#else
    void *application;
    void *globalPool;
#endif

}; // class QApplication =======================================================

#endif
