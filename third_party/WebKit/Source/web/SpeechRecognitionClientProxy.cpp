/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "web/SpeechRecognitionClientProxy.h"

#include "core/dom/ExecutionContext.h"
#include "modules/speech/SpeechGrammarList.h"
#include "modules/speech/SpeechRecognition.h"
#include "modules/speech/SpeechRecognitionError.h"
#include "modules/speech/SpeechRecognitionResult.h"
#include "modules/speech/SpeechRecognitionResultList.h"
#include "platform/weborigin/SecurityOrigin.h"
#include "public/web/WebSecurityOrigin.h"
#include "public/web/WebSpeechGrammar.h"
#include "public/web/WebSpeechRecognitionHandle.h"
#include "public/web/WebSpeechRecognitionParams.h"
#include "public/web/WebSpeechRecognitionResult.h"
#include "public/web/WebSpeechRecognizer.h"
#include "wtf/PassRefPtr.h"

using namespace WebCore;

namespace blink {

SpeechRecognitionClientProxy::~SpeechRecognitionClientProxy()
{
}

PassOwnPtr<SpeechRecognitionClientProxy> SpeechRecognitionClientProxy::create(WebSpeechRecognizer* recognizer)
{
    return adoptPtr(new SpeechRecognitionClientProxy(recognizer));
}

void SpeechRecognitionClientProxy::start(SpeechRecognition* recognition, const SpeechGrammarList* grammarList, const String& lang, bool continuous, bool interimResults, unsigned long maxAlternatives)
{
    WebVector<WebSpeechGrammar> webSpeechGrammars(static_cast<size_t>(grammarList->length()));
    for (unsigned long i = 0; i < grammarList->length(); ++i)
        webSpeechGrammars[i] = grammarList->item(i);

    WebSpeechRecognitionParams params(webSpeechGrammars, lang, continuous, interimResults, maxAlternatives, WebSecurityOrigin(recognition->executionContext()->securityOrigin()));
    m_recognizer->start(WebSpeechRecognitionHandle(recognition), params, this);
}

void SpeechRecognitionClientProxy::stop(SpeechRecognition* recognition)
{
    m_recognizer->stop(WebSpeechRecognitionHandle(recognition), this);
}

void SpeechRecognitionClientProxy::abort(SpeechRecognition* recognition)
{
    m_recognizer->abort(WebSpeechRecognitionHandle(recognition), this);
}

void SpeechRecognitionClientProxy::didStartAudio(const WebSpeechRecognitionHandle& handle)
{
    RefPtrWillBeRawPtr<SpeechRecognition> recognition = PassRefPtrWillBeRawPtr<SpeechRecognition>(handle);
    recognition->didStartAudio();
}

void SpeechRecognitionClientProxy::didStartSound(const WebSpeechRecognitionHandle& handle)
{
    RefPtrWillBeRawPtr<SpeechRecognition> recognition = PassRefPtrWillBeRawPtr<SpeechRecognition>(handle);
    recognition->didStartSound();
    recognition->didStartSpeech();
}

void SpeechRecognitionClientProxy::didEndSound(const WebSpeechRecognitionHandle& handle)
{
    RefPtrWillBeRawPtr<SpeechRecognition> recognition = PassRefPtrWillBeRawPtr<SpeechRecognition>(handle);
    recognition->didEndSpeech();
    recognition->didEndSound();
}

void SpeechRecognitionClientProxy::didEndAudio(const WebSpeechRecognitionHandle& handle)
{
    RefPtrWillBeRawPtr<SpeechRecognition> recognition = PassRefPtrWillBeRawPtr<SpeechRecognition>(handle);
    recognition->didEndAudio();
}

void SpeechRecognitionClientProxy::didReceiveResults(const WebSpeechRecognitionHandle& handle, const WebVector<WebSpeechRecognitionResult>& newFinalResults, const WebVector<WebSpeechRecognitionResult>& currentInterimResults)
{
    RefPtrWillBeRawPtr<SpeechRecognition> recognition = PassRefPtrWillBeRawPtr<SpeechRecognition>(handle);

    WillBeHeapVector<RefPtrWillBeMember<SpeechRecognitionResult> > finalResultsVector(newFinalResults.size());
    for (size_t i = 0; i < newFinalResults.size(); ++i)
        finalResultsVector[i] = PassRefPtrWillBeRawPtr<SpeechRecognitionResult>(newFinalResults[i]);

    WillBeHeapVector<RefPtrWillBeMember<SpeechRecognitionResult> > interimResultsVector(currentInterimResults.size());
    for (size_t i = 0; i < currentInterimResults.size(); ++i)
        interimResultsVector[i] = PassRefPtrWillBeRawPtr<SpeechRecognitionResult>(currentInterimResults[i]);

    recognition->didReceiveResults(finalResultsVector, interimResultsVector);
}

void SpeechRecognitionClientProxy::didReceiveNoMatch(const WebSpeechRecognitionHandle& handle, const WebSpeechRecognitionResult& result)
{
    RefPtrWillBeRawPtr<SpeechRecognition> recognition = PassRefPtrWillBeRawPtr<SpeechRecognition>(handle);
    recognition->didReceiveNoMatch(result);
}

void SpeechRecognitionClientProxy::didReceiveError(const WebSpeechRecognitionHandle& handle, const WebString& message, WebSpeechRecognizerClient::ErrorCode code)
{
    RefPtrWillBeRawPtr<SpeechRecognition> recognition = PassRefPtrWillBeRawPtr<SpeechRecognition>(handle);
    SpeechRecognitionError::ErrorCode errorCode = static_cast<SpeechRecognitionError::ErrorCode>(code);
    recognition->didReceiveError(SpeechRecognitionError::create(errorCode, message));
}

void SpeechRecognitionClientProxy::didStart(const WebSpeechRecognitionHandle& handle)
{
    RefPtrWillBeRawPtr<SpeechRecognition> recognition = PassRefPtrWillBeRawPtr<SpeechRecognition>(handle);
    recognition->didStart();
}

void SpeechRecognitionClientProxy::didEnd(const WebSpeechRecognitionHandle& handle)
{
    RefPtrWillBeRawPtr<SpeechRecognition> recognition = PassRefPtrWillBeRawPtr<SpeechRecognition>(handle);
    recognition->didEnd();
}

SpeechRecognitionClientProxy::SpeechRecognitionClientProxy(WebSpeechRecognizer* recognizer)
    : m_recognizer(recognizer)
{
}

} // namespace blink
