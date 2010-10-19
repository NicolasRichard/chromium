/*
 * Copyright (C) 2009, 2010 Apple Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

// This all-in-one cpp file cuts down on template bloat to allow us to build our Windows release build.

#include "DOMObjectHashTableMap.cpp"
#include "DOMWrapperWorld.cpp"
#include "GCController.cpp"
#include "JSAttrCustom.cpp"
#include "JSAudioConstructor.cpp"
#include "JSBindingState.cpp"
#include "JSCDATASectionCustom.cpp"
#include "JSCSSRuleCustom.cpp"
#include "JSCSSRuleListCustom.cpp"
#include "JSCSSStyleDeclarationCustom.cpp"
#include "JSCSSValueCustom.cpp"
#include "JSCallbackData.cpp"
#include "JSCanvasRenderingContext2DCustom.cpp"
#include "JSCanvasRenderingContextCustom.cpp"
#include "JSClipboardCustom.cpp"
#include "JSConsoleCustom.cpp"
#include "JSCoordinatesCustom.cpp"
#include "JSCustomPositionCallback.cpp"
#include "JSCustomPositionErrorCallback.cpp"
#include "JSCustomSQLStatementErrorCallback.cpp"
#include "JSCustomVoidCallback.cpp"
#include "JSCustomXPathNSResolver.cpp"
#include "JSDOMApplicationCacheCustom.cpp"
#include "JSDOMBinding.cpp"
#include "JSDOMFormDataCustom.cpp"
#include "JSDOMGlobalObject.cpp"
#include "JSDOMStringMapCustom.cpp"
#include "JSDOMWindowBase.cpp"
#include "JSDOMWindowCustom.cpp"
#include "JSDOMWindowShell.cpp"
#include "JSDOMWrapper.cpp"
#include "JSDataGridColumnListCustom.cpp"
#include "JSDataGridDataSource.cpp"
#include "JSDebugWrapperSet.cpp"
#include "JSDedicatedWorkerContextCustom.cpp"
#include "JSDesktopNotificationsCustom.cpp"
#include "JSDeviceOrientationEventCustom.cpp"
#include "JSDocumentCustom.cpp"
#include "JSElementCustom.cpp"
#include "JSEventCustom.cpp"
#include "JSEventListener.cpp"
#include "JSEventSourceCustom.cpp"
#include "JSEventTarget.cpp"
#include "JSExceptionBase.cpp"
#include "JSGeolocationCustom.cpp"
#include "JSHTMLAllCollectionCustom.cpp"
#include "JSHTMLAppletElementCustom.cpp"
#include "JSHTMLCanvasElementCustom.cpp"
#include "JSHTMLCollectionCustom.cpp"
#include "JSHTMLDataGridElementCustom.cpp"
#include "JSHTMLDocumentCustom.cpp"
#include "JSHTMLElementCustom.cpp"
#include "JSHTMLEmbedElementCustom.cpp"
#include "JSHTMLFormElementCustom.cpp"
#include "JSHTMLFrameElementCustom.cpp"
#include "JSHTMLFrameSetElementCustom.cpp"
#include "JSHTMLInputElementCustom.cpp"
#include "JSHTMLObjectElementCustom.cpp"
#include "JSHTMLOptionsCollectionCustom.cpp"
#include "JSHTMLSelectElementCustom.cpp"
#include "JSHistoryCustom.cpp"
#include "JSImageConstructor.cpp"
#include "JSImageDataCustom.cpp"
#include "JSInjectedScriptHostCustom.cpp"
#include "JSInspectorFrontendHostCustom.cpp"
#include "JSJavaScriptCallFrameCustom.cpp"
#include "JSLazyEventListener.cpp"
#include "JSLocationCustom.cpp"
#include "JSMainThreadExecState.cpp"
#include "JSMessageChannelCustom.cpp"
#include "JSMessageEventCustom.cpp"
#include "JSMessagePortCustom.cpp"
#include "JSDOMMimeTypeArrayCustom.cpp"
#include "JSNamedNodeMapCustom.cpp"
#include "JSNavigatorCustom.cpp"
#include "JSNodeCustom.cpp"
#include "JSNodeFilterCondition.cpp"
#include "JSNodeFilterCustom.cpp"
#include "JSNodeIteratorCustom.cpp"
#include "JSNodeListCustom.cpp"
#include "JSOptionConstructor.cpp"
#include "JSDOMPluginArrayCustom.cpp"
#include "JSDOMPluginCustom.cpp"
#include "JSPluginElementFunctions.cpp"
#include "JSPopStateEventCustom.cpp"
#include "JSSQLResultSetRowListCustom.cpp"
#include "JSSQLTransactionCustom.cpp"
#include "JSSQLTransactionSyncCustom.cpp"
#include "JSSVGElementInstanceCustom.cpp"
#include "JSSVGLengthCustom.cpp"
#include "JSSVGMatrixCustom.cpp"
#include "JSSVGPathSegCustom.cpp"
#include "JSSVGPathSegListCustom.cpp"
#include "JSScriptProfileNodeCustom.cpp"
#include "JSSharedWorkerCustom.cpp"
#include "JSStorageCustom.cpp"
#include "JSStyleSheetCustom.cpp"
#include "JSStyleSheetListCustom.cpp"
#include "JSTextCustom.cpp"
#include "JSTouchCustom.cpp"
#include "JSTouchListCustom.cpp"
#include "JSTreeWalkerCustom.cpp"
#include "JSWebKitCSSMatrixCustom.cpp"
#include "JSWebKitPointCustom.cpp"
#include "JSWebSocketCustom.cpp"
#include "JSWorkerContextBase.cpp"
#include "JSWorkerContextCustom.cpp"
#include "JSWorkerContextErrorHandler.cpp"
#include "JSWorkerCustom.cpp"
#include "JSXMLHttpRequestCustom.cpp"
#include "JSXMLHttpRequestUploadCustom.cpp"
#include "JSXSLTProcessorCustom.cpp"
#include "JavaScriptCallFrame.cpp"
#include "MemoryInfo.cpp"
#include "ScheduledAction.cpp"
#include "ScriptCachedFrameData.cpp"
#include "ScriptCallFrame.cpp"
#include "ScriptCallStack.cpp"
#include "ScriptController.cpp"
#include "ScriptControllerWin.cpp"
#include "ScriptDebugServer.cpp"
#include "ScriptEventListener.cpp"
#include "ScriptFunctionCall.cpp"
#include "ScriptGCEvent.cpp"
#include "ScriptProfiler.cpp"
#include "ScriptState.cpp"
#include "SerializedScriptValue.cpp"
#include "WorkerScriptController.cpp"
