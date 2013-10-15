/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "core/loader/FormSubmission.h"

#include "HTMLNames.h"
#include "RuntimeEnabledFeatures.h"
#include "core/dom/Document.h"
#include "core/events/Event.h"
#include "core/html/DOMFormData.h"
#include "core/html/HTMLFormControlElement.h"
#include "core/html/HTMLFormElement.h"
#include "core/html/HTMLInputElement.h"
#include "core/html/parser/HTMLParserIdioms.h"
#include "core/loader/FormState.h"
#include "core/loader/FrameLoadRequest.h"
#include "core/loader/FrameLoader.h"
#include "core/platform/network/FormData.h"
#include "core/platform/network/FormDataBuilder.h"
#include "wtf/CurrentTime.h"
#include "wtf/text/TextEncoding.h"

namespace WebCore {

using namespace HTMLNames;

static int64_t generateFormDataIdentifier()
{
    // Initialize to the current time to reduce the likelihood of generating
    // identifiers that overlap with those from past/future browser sessions.
    static int64_t nextIdentifier = static_cast<int64_t>(currentTime() * 1000000.0);
    return ++nextIdentifier;
}

static void appendMailtoPostFormDataToURL(KURL& url, const FormData& data, const String& encodingType)
{
    String body = data.flattenToString();

    if (equalIgnoringCase(encodingType, "text/plain")) {
        // Convention seems to be to decode, and s/&/\r\n/. Also, spaces are encoded as %20.
        body = decodeURLEscapeSequences(body.replaceWithLiteral('&', "\r\n").replace('+', ' ') + "\r\n");
    }

    Vector<char> bodyData;
    bodyData.append("body=", 5);
    FormDataBuilder::encodeStringAsFormData(bodyData, body.utf8());
    body = String(bodyData.data(), bodyData.size()).replaceWithLiteral('+', "%20");

    String query = url.query();
    if (!query.isEmpty())
        query.append('&');
    query.append(body);
    url.setQuery(query);
}

void FormSubmission::Attributes::parseAction(const String& action)
{
    // FIXME: Can we parse into a KURL?
    m_action = stripLeadingAndTrailingHTMLSpaces(action);
}

String FormSubmission::Attributes::parseEncodingType(const String& type)
{
    if (equalIgnoringCase(type, "multipart/form-data"))
        return "multipart/form-data";
    if (equalIgnoringCase(type, "text/plain"))
        return "text/plain";
    return "application/x-www-form-urlencoded";
}

void FormSubmission::Attributes::updateEncodingType(const String& type)
{
    m_encodingType = parseEncodingType(type);
    m_isMultiPartForm = (m_encodingType == "multipart/form-data");
}

FormSubmission::Method FormSubmission::Attributes::parseMethodType(const String& type)
{
    if (equalIgnoringCase(type, "post"))
        return FormSubmission::PostMethod;
    if (RuntimeEnabledFeatures::dialogElementEnabled() && equalIgnoringCase(type, "dialog"))
        return FormSubmission::DialogMethod;
    return FormSubmission::GetMethod;
}

void FormSubmission::Attributes::updateMethodType(const String& type)
{
    m_method = parseMethodType(type);
}

String FormSubmission::Attributes::methodString(Method method)
{
    switch (method) {
    case GetMethod:
        return "get";
    case PostMethod:
        return "post";
    case DialogMethod:
        return "dialog";
    }
    ASSERT_NOT_REACHED();
    return emptyString();
}

void FormSubmission::Attributes::copyFrom(const Attributes& other)
{
    m_method = other.m_method;
    m_isMultiPartForm = other.m_isMultiPartForm;

    m_action = other.m_action;
    m_target = other.m_target;
    m_encodingType = other.m_encodingType;
    m_acceptCharset = other.m_acceptCharset;
}

inline FormSubmission::FormSubmission(Method method, const KURL& action, const String& target, const String& contentType, PassRefPtr<FormState> state, PassRefPtr<FormData> data, const String& boundary, PassRefPtr<Event> event)
    : m_method(method)
    , m_action(action)
    , m_target(target)
    , m_contentType(contentType)
    , m_formState(state)
    , m_formData(data)
    , m_boundary(boundary)
    , m_event(event)
{
}

inline FormSubmission::FormSubmission(const String& result)
    : m_method(DialogMethod)
    , m_result(result)
{
}

PassRefPtr<FormSubmission> FormSubmission::create(HTMLFormElement* form, const Attributes& attributes, PassRefPtr<Event> event, FormSubmissionTrigger trigger)
{
    ASSERT(form);

    HTMLFormControlElement* submitButton = 0;
    if (event && event->target()) {
        for (Node* node = event->target()->toNode(); node; node = node->parentOrShadowHostNode()) {
            if (node->isElementNode() && toElement(node)->isFormControlElement()) {
                submitButton = toHTMLFormControlElement(node);
                break;
            }
        }
    }

    FormSubmission::Attributes copiedAttributes;
    copiedAttributes.copyFrom(attributes);
    if (submitButton) {
        String attributeValue;
        if (!(attributeValue = submitButton->fastGetAttribute(formactionAttr)).isNull())
            copiedAttributes.parseAction(attributeValue);
        if (!(attributeValue = submitButton->fastGetAttribute(formenctypeAttr)).isNull())
            copiedAttributes.updateEncodingType(attributeValue);
        if (!(attributeValue = submitButton->fastGetAttribute(formmethodAttr)).isNull())
            copiedAttributes.updateMethodType(attributeValue);
        if (!(attributeValue = submitButton->fastGetAttribute(formtargetAttr)).isNull())
            copiedAttributes.setTarget(attributeValue);
    }

    if (copiedAttributes.method() == DialogMethod)
        return adoptRef(new FormSubmission(submitButton->fastGetAttribute(valueAttr)));

    Document& document = form->document();
    KURL actionURL = document.completeURL(copiedAttributes.action().isEmpty() ? document.url().string() : copiedAttributes.action());
    bool isMailtoForm = actionURL.protocolIs("mailto");
    bool isMultiPartForm = false;
    String encodingType = copiedAttributes.encodingType();

    if (copiedAttributes.method() == PostMethod) {
        isMultiPartForm = copiedAttributes.isMultiPartForm();
        if (isMultiPartForm && isMailtoForm) {
            encodingType = "application/x-www-form-urlencoded";
            isMultiPartForm = false;
        }
    }
    WTF::TextEncoding dataEncoding = isMailtoForm ? UTF8Encoding() : FormDataBuilder::encodingFromAcceptCharset(copiedAttributes.acceptCharset(), document.inputEncoding(), document.defaultCharset());
    RefPtr<DOMFormData> domFormData = DOMFormData::create(dataEncoding.encodingForFormSubmission());
    Vector<pair<String, String> > formValues;

    bool containsPasswordData = false;
    for (unsigned i = 0; i < form->associatedElements().size(); ++i) {
        FormAssociatedElement* control = form->associatedElements()[i];
        HTMLElement* element = toHTMLElement(control);
        if (!element->isDisabledFormControl())
            control->appendFormData(*domFormData, isMultiPartForm);
        if (element->hasTagName(inputTag)) {
            HTMLInputElement* input = toHTMLInputElement(element);
            if (input->isTextField())
                formValues.append(pair<String, String>(input->name().string(), input->value()));
            if (input->isPasswordField() && !input->value().isEmpty())
                containsPasswordData = true;
        }
    }

    RefPtr<FormData> formData;
    String boundary;

    if (isMultiPartForm) {
        formData = FormData::createMultiPart(*(static_cast<FormDataList*>(domFormData.get())), domFormData->encoding());
        boundary = formData->boundary().data();
    } else {
        formData = FormData::create(*(static_cast<FormDataList*>(domFormData.get())), domFormData->encoding(), attributes.method() == GetMethod ? FormData::FormURLEncoded : FormData::parseEncodingType(encodingType));
        if (copiedAttributes.method() == PostMethod && isMailtoForm) {
            // Convert the form data into a string that we put into the URL.
            appendMailtoPostFormDataToURL(actionURL, *formData, encodingType);
            formData = FormData::create();
        }
    }

    formData->setIdentifier(generateFormDataIdentifier());
    formData->setContainsPasswordData(containsPasswordData);
    String targetOrBaseTarget = copiedAttributes.target().isEmpty() ? document.baseTarget() : copiedAttributes.target();
    RefPtr<FormState> formState = FormState::create(form, formValues, &document, trigger);
    return adoptRef(new FormSubmission(copiedAttributes.method(), actionURL, targetOrBaseTarget, encodingType, formState.release(), formData.release(), boundary, event));
}

KURL FormSubmission::requestURL() const
{
    if (m_method == FormSubmission::PostMethod)
        return m_action;

    KURL requestURL(m_action);
    requestURL.setQuery(m_formData->flattenToString());
    return requestURL;
}

void FormSubmission::populateFrameLoadRequest(FrameLoadRequest& frameRequest)
{
    if (!m_target.isEmpty())
        frameRequest.setFrameName(m_target);

    if (!m_referrer.isEmpty())
        frameRequest.resourceRequest().setHTTPReferrer(m_referrer);

    if (m_method == FormSubmission::PostMethod) {
        frameRequest.resourceRequest().setHTTPMethod("POST");
        frameRequest.resourceRequest().setHTTPBody(m_formData);

        // construct some user headers if necessary
        if (m_boundary.isEmpty())
            frameRequest.resourceRequest().setHTTPContentType(m_contentType);
        else
            frameRequest.resourceRequest().setHTTPContentType(m_contentType + "; boundary=" + m_boundary);
    }

    frameRequest.resourceRequest().setURL(requestURL());
    FrameLoader::addHTTPOriginIfNeeded(frameRequest.resourceRequest(), m_origin);
}

}
