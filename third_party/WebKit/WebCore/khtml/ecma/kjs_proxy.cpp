/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"
#include "kjs_proxy.h"

#include "kjs_window.h"
#include "kjs_events.h"
#include <khtml_part.h>
#include <kjs/collector.h>

using namespace KJS;

using DOM::EventListener;

#ifndef NDEBUG
int KJSProxyImpl::s_count = 0;
#endif

KJSProxyImpl::KJSProxyImpl(KHTMLPart *part)
{
  m_script = 0;
  m_part = part;
  m_handlerLineno = 0;
#ifndef NDEBUG
  s_count++;
#endif
}

KJSProxyImpl::~KJSProxyImpl()
{
  JSLock lock;
  delete m_script;

#ifndef NDEBUG
  s_count--;
  // If it was the last interpreter, we should have nothing left
#ifdef KJS_DEBUG_MEM
  if (s_count == 0)
    Interpreter::finalCheck();
#endif
#endif
}

QVariant KJSProxyImpl::evaluate(QString filename, int baseLine,
                                const QString&str, DOM::NodeImpl *n) 
{
  // evaluate code. Returns the JS return value or an invalid QVariant
  // if there was none, an error occured or the type couldn't be converted.

  initScript();
  // inlineCode is true for <a href="javascript:doSomething()">
  // and false for <script>doSomething()</script>. Check if it has the
  // expected value in all cases.
  // See smart window.open policy for where this is used.
  bool inlineCode = filename.isNull();

  m_script->setInlineCode(inlineCode);

  JSLock lock;

  KJS::JSValue *thisNode = n ? Window::retrieve(m_part) : getDOMNode(m_script->globalExec(), n);
  UString code(str);
  Completion comp = m_script->evaluate(filename, baseLine, code, thisNode);

  bool success = ( comp.complType() == Normal ) || ( comp.complType() == ReturnValue );  

  // let's try to convert the return value
  if (success && comp.value())
    return ValueToVariant(m_script->globalExec(), comp.value());

  if ( comp.complType() == Throw ) {
    UString errorMessage = comp.value()->toString(m_script->globalExec());
    int lineNumber =  comp.value()->toObject(m_script->globalExec())->get(m_script->globalExec(), "line")->toInt32(m_script->globalExec());
    UString sourceURL = comp.value()->toObject(m_script->globalExec())->get(m_script->globalExec(), "sourceURL")->toString(m_script->globalExec());

    KWQ(m_part)->addMessageToConsole(errorMessage.qstring(), lineNumber, sourceURL.qstring());
  }
  return QVariant();
}

void KJSProxyImpl::clear() {
  // clear resources allocated by the interpreter, and make it ready to be used by another page
  // We have to keep it, so that the Window object for the part remains the same.
  // (we used to delete and re-create it, previously)
  if (m_script) {
    Window *win = Window::retrieveWindow(m_part);
    if (win)
        win->clear( m_script->globalExec() );
  }
}

DOM::EventListener *KJSProxyImpl::createHTMLEventHandler(QString code, DOM::NodeImpl *node)
{
  initScript();
  JSLock lock;
  return KJS::Window::retrieveWindow(m_part)->getJSLazyEventListener(code, node, m_handlerLineno);
}

void KJSProxyImpl::finishedWithEvent(DOM::EventImpl *event)
{
  // This is called when the DOM implementation has finished with a particular event. This
  // is the case in sitations where an event has been created just for temporary usage,
  // e.g. an image load or mouse move. Once the event has been dispatched, it is forgotten
  // by the DOM implementation and so does not need to be cached still by the interpreter
  m_script->forgetDOMObject(event);
}

KJS::ScriptInterpreter *KJSProxyImpl::interpreter()
{
  if (!m_script)
    initScript();
  m_part->keepAlive();
  return m_script;
}

// Implementation of the debug() function
class TestFunctionImp : public JSObject {
public:
  TestFunctionImp() : JSObject() {}
  virtual bool implementsCall() const { return true; }
  virtual JSValue *callAsFunction(ExecState *exec, JSObject *thisObj, const List &args);
};

JSValue *TestFunctionImp::callAsFunction(ExecState *exec, JSObject */*thisObj*/, const List &args)
{
  fprintf(stderr,"--> %s\n",args[0]->toString(exec).ascii());
  return jsUndefined();
}

void KJSProxyImpl::initScript()
{
  if (m_script)
    return;

  // Build the global object - which is a Window instance
  KJS::JSLock lock;
  JSObject *globalObject( new Window(m_part) );

  // Create a KJS interpreter for this part
  m_script = new KJS::ScriptInterpreter(globalObject, m_part);
  globalObject->put(m_script->globalExec(), "debug", new TestFunctionImp(), Internal);

  QString userAgent = KWQ(m_part)->userAgent();
  if (userAgent.find(QString::fromLatin1("Microsoft")) >= 0 ||
      userAgent.find(QString::fromLatin1("MSIE")) >= 0)
    m_script->setCompatMode(Interpreter::IECompat);
  else
    // If we find "Mozilla" but not "(compatible, ...)" we are a real Netscape
    if (userAgent.find(QString::fromLatin1("Mozilla")) >= 0 &&
        userAgent.find(QString::fromLatin1("compatible")) == -1)
      m_script->setCompatMode(Interpreter::NetscapeCompat);
}
