/*
 * Copyright (C) 2001, 2002 Apple Computer, Inc.  All rights reserved.
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

#import <KWQKHTMLPartImpl.h>

#import <htmltokenizer.h>
#import <html_documentimpl.h>
#import <render_root.h>
#import <render_frames.h>
#import <render_text.h>
#import <khtmlpart_p.h>
#import <khtmlview.h>

#import <WebCoreBridge.h>
#import <WebCoreViewFactory.h>

#import <kwqdebug.h>

#undef _KWQ_TIMING

using khtml::Decoder;
using khtml::RenderObject;
using khtml::RenderPart;
using khtml::RenderWidget;

void KHTMLPart::onURL(const QString &)
{
}

void KHTMLPart::nodeActivated(const DOM::Node &aNode)
{
}

void KHTMLPart::setStatusBarText(const QString &status)
{
    impl->setStatusBarText(status);
}

KWQKHTMLPartImpl::KWQKHTMLPartImpl(KHTMLPart *p)
    : part(p)
    , d(part->d)
    , m_redirectionTimer(0)
{
}

KWQKHTMLPartImpl::~KWQKHTMLPartImpl()
{
    killTimer(m_redirectionTimer);
}

bool KWQKHTMLPartImpl::openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs )
{
    WebCoreBridge *frame;

    if (!urlArgs.frameName.isEmpty()) {
        frame = [bridge frameNamed:urlArgs.frameName.getNSString()];
        if (frame == nil) {
            frame = [bridge mainFrame];
        }
    } else {
        frame = bridge;
    }

    [frame loadURL:url.getNSURL()];

    return true;
}

void KWQKHTMLPartImpl::openURL(const KURL &url)
{
    d->m_workingURL = url;
    part->m_url = url;
}

void KWQKHTMLPartImpl::slotData(NSString *encoding, const char *bytes, int length, bool complete)
{
// NOTE: This code emulates the interface used by the original khtml part  
    QString enc;

    if (!d->m_workingURL.isEmpty()) {
        //begin(d->m_workingURL, d->m_extension->urlArgs().xOffset, d->m_extension->urlArgs().yOffset);
        part->begin(d->m_workingURL, 0, 0);

	//d->m_doc->docLoader()->setReloading(d->m_bReloading);
        d->m_workingURL = KURL();
    }

    // This flag is used to tell when a load has completed so we can be sure
    // to process the data even if we have not yet determined the proper
    // encoding.
    if (complete) {
        d->m_bComplete = true;    
    }

    if (encoding != NULL) {
        enc = QString::fromCFString((CFStringRef) encoding);
        part->setEncoding(enc, true);
    }
    
    KWQ_ASSERT(d->m_doc != NULL);

    part->write(bytes, length);
}

// FIXME: Need to remerge this with code in khtml_part.cpp?
void KWQKHTMLPartImpl::begin( const KURL &url, int xOffset, int yOffset )
{
  KParts::URLArgs args;
  args.xOffset = xOffset;
  args.yOffset = yOffset;
  d->m_extension->setURLArgs( args );

  d->m_bComplete = false;
  // d->m_referrer = url.url();
  part->m_url = url;

  // ### not sure if XHTML documents served as text/xml should use DocumentImpl or HTMLDocumentImpl
  if (args.serviceType == "text/xml")
    d->m_doc = DOM::DOMImplementationImpl::instance()->createDocument( d->m_view );
  else
    d->m_doc = DOM::DOMImplementationImpl::instance()->createHTMLDocument( d->m_view );

    //DomShared::instanceToCheck = (void *)((DomShared *)d->m_doc);
    d->m_doc->ref();

    d->m_workingURL = url;

    /* FIXME: this is a pretty gross way to make sure the decoder gets reinitialized for each page. */
    if (d->m_decoder != NULL) {
	delete d->m_decoder;
	d->m_decoder = NULL;
    }

    //FIXME: do we need this? 
    if (!d->m_doc->attached())
	d->m_doc->attach();
    d->m_doc->setURL( url.url() );
    
    // do not set base URL if it has already been set
    if (!d->m_workingURL.isEmpty())
    {
	// We're not planning to support the KDE chained URL feature, AFAIK
#if KDE_CHAINED_URIS
        KURL::List lst = KURL::split( d->m_workingURL );
        KURL baseurl;
        if ( !lst.isEmpty() )
            baseurl = *lst.begin();
        // Use this for relative links.
        setBaseURL(baseurl);
#else
	if (d->m_doc != NULL) {
	    d->m_doc->setBaseURL(d->m_workingURL.url());
	}
#endif
    }

    /* FIXME: we'll need to make this work....
    QString userStyleSheet = KHTMLFactory::defaultHTMLSettings()->userStyleSheet();
    if ( !userStyleSheet.isEmpty() ) {
        setUserStyleSheet( KURL( userStyleSheet ) );
    }
    */
    
    d->m_doc->open();    

    d->m_doc->setParsing(true);

#ifdef _KWQ_TIMING        
    d->totalWriteTime = 0;
#endif
}

// FIXME: Need to remerge this with code in khtml_part.cpp?
void KWQKHTMLPartImpl::end()
{
    KWQ_ASSERT(d->m_doc != NULL);

    d->m_doc->setParsing(false);

    d->m_doc->close();
    KURL::clearCaches();
    
    if (d->m_view)
        d->m_view->complete();
}
 
bool KWQKHTMLPartImpl::gotoBaseAnchor()
{
    if ( !part->m_url.ref().isEmpty() )
        return part->gotoAnchor( part->m_url.ref() );
    return false;
}

// FIXME: Need to remerge this with code in khtml_part.cpp?
// Specifically, it seems that if we implement QTimer, including
// some sort of special case to make connect work, we could use
// KHTMLPart::scheduleRedirection as-is.
void KWQKHTMLPartImpl::scheduleRedirection(int delay, const QString &url)
{
    if( d->m_redirectURL.isEmpty() || delay < d->m_delayRedirect )
    {
        d->m_delayRedirect = delay;
        d->m_redirectURL = url;
        killTimer(m_redirectionTimer);
        m_redirectionTimer = startTimer(1000 * d->m_delayRedirect);
    }
}

void KWQKHTMLPartImpl::timerEvent(QTimerEvent *e)
{
    part->slotRedirect();
}

void KWQKHTMLPartImpl::urlSelected( const QString &url, int button, int state, const QString &_target, KParts::URLArgs )
{
    KURL clickedURL(part->completeURL( url));
    KURL refLess(clickedURL);
    WebCoreBridge *frame;
	
    if ( url.find( "javascript:", 0, false ) == 0 )
    {
        part->executeScript( url.right( url.length() - 11) );
        return;
    }

    // Open new window on command-click
    if (state & MetaButton) {
        [bridge openNewWindowWithURL:clickedURL.getNSURL()];
        return;
    }

    part->m_url.setRef ("");
    refLess.setRef ("");
    if (refLess.url() == part->m_url.url()){
        part->m_url = clickedURL;
        part->gotoAnchor (clickedURL.ref());
        // This URL needs to be added to the back/forward list.
        [bridge addBackForwardItemWithURL: clickedURL.getNSURL() anchor:clickedURL.ref().getNSString()];
        return;
    }
    
    if (_target.isEmpty()) {
        // If we're the only frame in a frameset then pop the frame.
        if ([[[bridge parent] childFrames] count] == 1) {
            frame = [bridge parent];
        } else {
            frame = bridge;
        }
    } else {
        frame = [bridge descendantFrameNamed:_target.getNSString()];
        if (frame == nil) {
            NSLog (@"WARNING: unable to find frame named %@, creating new window with \"_blank\" name.  New window will not be named until 2959902 is fixed.\n", _target.getNSString());
                frame = [bridge descendantFrameNamed:@"_blank"];
        }
    }
    
    [frame loadURL:clickedURL.getNSURL()];
}

bool KWQKHTMLPartImpl::requestFrame( RenderPart *frame, const QString &url, const QString &frameName,
                                     const QStringList &params, bool isIFrame )
{
    KWQ_ASSERT(!frameExists(frameName));

    NSURL *childURL = part->completeURL(url).getNSURL();
    if (childURL == nil) {
        NSLog (@"ERROR (probably need to fix CFURL): unable to create URL with path (base URL %s, relative URL %s)", d->m_doc->baseURL().ascii(), url.ascii());
        return false;
    }
    
    KWQDEBUGLEVEL(KWQ_LOG_FRAMES, "name %s\n", frameName.ascii());
    
    HTMLIFrameElementImpl *o = static_cast<HTMLIFrameElementImpl *>(frame->element());
    if (![bridge createChildFrameNamed:frameName.getNSString() withURL:childURL
            renderPart:frame allowsScrolling:o->scrollingMode() != QScrollView::AlwaysOff
            marginWidth:o->getMarginWidth() marginHeight:o->getMarginHeight()]) {
        return false;
    }

#ifdef _SUPPORT_JAVASCRIPT_URL_    
    if ( url.find( QString::fromLatin1( "javascript:" ), 0, false ) == 0 && !isIFrame )
    {
        // static cast is safe as of isIFrame being false.
        // but: shouldn't we support this javascript hack for iframes aswell?
        RenderFrame* rf = static_cast<RenderFrame*>(frame);
        assert(rf);
        QVariant res = executeScript( DOM::Node(rf->frameImpl()), url.right( url.length() - 11) );
        if ( res.type() == QVariant::String ) {
            KURL myurl;
            myurl.setProtocol("javascript");
            myurl.setPath(res.asString());
            return processObjectRequest(&(*it), myurl, QString("text/html") );
        }
        return false;
    }
#endif

    return true;
}

bool KWQKHTMLPartImpl::requestObject(RenderPart *frame, const QString &url, const QString &serviceType, const QStringList &args)
{
    if (url.isEmpty()) {
        return false;
    }
    if (frame->widget()) {
        return true;
    }
    
    NSMutableArray *argsArray = [NSMutableArray arrayWithCapacity:args.count()];
    for (uint i = 0; i < args.count(); i++) {
        [argsArray addObject:args[i].getNSString()];
    }
    
    QWidget *widget = new QWidget();
    widget->setView([[WebCoreViewFactory sharedFactory]
        viewForPluginWithURL:part->completeURL(url).getNSURL()
                 serviceType:serviceType.getNSString()
                   arguments:argsArray
                     baseURL:KURL(d->m_doc->baseURL()).getNSURL()]);
    frame->setWidget(widget);
    
    return true;
}

void KWQKHTMLPartImpl::submitForm( const char *action, const QString &url, const QByteArray &formData, const QString &_target, const QString& contentType, const QString& boundary )
{
  QString target = _target;
  
  //if ( target.isEmpty() )
  //  target = d->m_baseTarget;

  KURL u = part->completeURL( url );

  if ( u.isMalformed() )
  {
    // ### ERROR HANDLING!
    return;
  }

  QString urlstring = u.url();

  if ( urlstring.find( "javascript:", 0, false ) == 0 ) {
    urlstring = KURL::decode_string(urlstring);
    part->executeScript( urlstring.right( urlstring.length() - 11) );
    return;
  }

#ifdef NEED_THIS
  if (!checkLinkSecurity(u,
			 i18n( "<qt>The form will be submitted to <BR><B>%1</B><BR>on your local filesystem.<BR>Do you want to submit the form?" ),
			 i18n( "Submit" )))
    return;
#endif

#ifdef NEED_THIS
  KParts::URLArgs args;

  if (!d->m_referrer.isEmpty())
     args.metaData()["referrer"] = d->m_referrer;

  args.metaData().insert("main_frame_request",
                         parentPart() == 0 ? "TRUE":"FALSE");
  args.metaData().insert("ssl_was_in_use", d->m_ssl_in_use ? "TRUE":"FALSE");
  args.metaData().insert("ssl_activate_warnings", "TRUE");
  args.frameName = _target.isEmpty() ? d->m_doc->baseTarget() : _target ;
#endif

  if ( strcmp( action, "get" ) == 0 )
  {
    u.setQuery( QString( formData.data(), formData.size() ) );
    [bridge loadURL:u.getNSURL()];

#ifdef NEED_THIS
    args.frameName = target;
    args.setDoPost( false );
#endif
  }
  else
  {
#ifdef NEED_THIS
    args.postData = formData;
    args.frameName = target;
    args.setDoPost( true );

    // construct some user headers if necessary
    if (contentType.isNull() || contentType == "application/x-www-form-urlencoded")
      args.setContentType( "Content-Type: application/x-www-form-urlencoded" );
    else // contentType must be "multipart/form-data"
      args.setContentType( "Content-Type: " + contentType + "; boundary=" + boundary );
#endif
    NSData *postData = [NSData dataWithBytes:formData.data() length:formData.size()];
    [bridge postWithURL:u.getNSURL() data:postData];
  }

#ifdef NEED_THIS
  if ( d->m_bParsing || d->m_runningScripts > 0 ) {
    if( d->m_submitForm ) {
        return;
    }
    d->m_submitForm = new KHTMLPartPrivate::SubmitForm;
    d->m_submitForm->submitAction = action;
    d->m_submitForm->submitUrl = url;
    d->m_submitForm->submitFormData = formData;
    d->m_submitForm->target = _target;
    d->m_submitForm->submitContentType = contentType;
    d->m_submitForm->submitBoundary = boundary;
    connect(this, SIGNAL(completed()), this, SLOT(submitFormAgain()));
  }
  else
    emit d->m_extension->openURLRequest( u, args );
#endif
}

bool KWQKHTMLPartImpl::frameExists( const QString &frameName )
{
    return [bridge frameNamed:frameName.getNSString()] != nil;
}

KHTMLPart *KWQKHTMLPartImpl::findFrame(const QString &frameName)
{
    return [[bridge frameNamed:frameName.getNSString()] part];
}

QPtrList<KParts::ReadOnlyPart> KWQKHTMLPartImpl::frames() const
{
    QPtrList<KParts::ReadOnlyPart> parts;
    NSEnumerator *e = [[bridge childFrames] objectEnumerator];
    WebCoreBridge *childFrame;
    while ((childFrame = [e nextObject])) {
        KHTMLPart *childPart = [childFrame part];
        if (childPart)
            parts.append(childPart);
    }
    return parts;
}

void KWQKHTMLPartImpl::setView(KHTMLView *view)
{
    d->m_view = view;
    part->setWidget(view);
}

KHTMLView *KWQKHTMLPartImpl::getView() const
{
    return d->m_view;
}

void KWQKHTMLPartImpl::setTitle(const DOMString &title)
{
    [bridge setTitle:title.string().getNSString()];
}

void KWQKHTMLPartImpl::setStatusBarText(const QString &status)
{
    [bridge setStatusText:status.getNSString()];
}


KHTMLPart *KWQKHTMLPartImpl::parentPart()
{
    return [[bridge parent] part];
}

bool KWQKHTMLPartImpl::openedByJS()
{
    return [bridge openedByScript];
}

void KWQKHTMLPartImpl::setOpenedByJS(bool _openedByJS)
{
    [bridge setOpenedByScript:_openedByJS];
}

void KWQKHTMLPartImpl::scheduleClose()
{
    [[bridge window] performSelector:@selector(close) withObject:nil afterDelay:0.0];
}

void KWQKHTMLPartImpl::unfocusWindow()
{
    [bridge unfocusWindow];
}

void KWQKHTMLPartImpl::overURL( const QString &url, const QString &target, int modifierState)
{
    if (url.isEmpty()) {
        setStatusBarText(QString());
        return;
    }

    NSString *message;
    
    // FIXME: This would do strange things with a link that said "xjavascript:".
    int position = url.find("javascript:", 0, false);
    if (position != -1) {
        // FIXME: Is it worthwhile to special-case scripts that do a window.open and nothing else?
        const QString scriptName = url.mid(position + strlen("javascript:"));
        message = [NSString stringWithFormat:@"Run script \"%@\"", scriptName.getNSString()];
        setStatusBarText(QString::fromNSString(message));
        return;
    }
    
    KURL u = part->completeURL(url);
    
    if (u.protocol() == QString("mailto")) {
        // FIXME: addressbook integration? probably not worth it...
        
        setStatusBarText(QString::fromNSString([NSString stringWithFormat:@"Send email to %@", KURL::decode_string(u.path()).getNSString()]));
        return;
    }
    
    NSString *format;
    
    if (target == QString("_blank")) {
        // FIXME: should use curly quotes
        format = @"Open \"%@\" in a new window";
        
    } else if (!target.isEmpty() &&
                (target != QString("_top")) &&
                (target != QString("_self")) &&
                (target != QString("_parent"))) {
        if (frameExists(target)) {
            // FIXME: distinguish existing frame in same window from
            // existing frame name for other window
            format = @"Go to \"%@\" in another frame";
        } else {
            format = @"Open \"%@\" in a new window";
        }
    } else {
        format = @"Go to \"%@\"";
    }
    
    if ([bridge modifierTrackingEnabled]) {
        if (modifierState & MetaButton) {
            if (modifierState & ShiftButton) {
                format = @"Open \"%@\" in a new window, behind the current window";
            } else {
                format = @"Open \"%@\" in a new window";
            }
        } else if (modifierState & AltButton) {
            format = @"Download \"%@\"";
        }
    }
    
    setStatusBarText(QString::fromNSString([NSString stringWithFormat:format, url.getNSString()]));
}


void KWQKHTMLPartImpl::jumpToSelection()
{
    // Assumes that selection will only ever be text nodes.  This is currently
    // true, but will it always be so?
    if (d->m_selectionStart != 0){
        khtml::RenderObject *ro = static_cast<khtml::RenderObject *>(d->m_selectionStart.handle()->renderer());

        if (strcmp(ro->renderName(), "RenderText") == 0){
            int x = 0, y = 0;
            khtml::RenderText *rt = static_cast<khtml::RenderText *>(d->m_selectionStart.handle()->renderer());
            rt->posOfChar(d->m_startOffset, x, y);
            // The -50 offset is copied from KHTMLPart::findTextNext, which sets the contents position
            // after finding a matched text string.
            d->m_view->setContentsPos(x-50, y-50);
        }
    }
}

