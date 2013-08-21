/*
 * Copyright (C) 2011, 2012 Google Inc. All rights reserved.
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
#include "WebView.h"

#include <gtest/gtest.h>
#include "FrameTestHelpers.h"
#include "URLTestHelpers.h"
#include "WebAutofillClient.h"
#include "WebContentDetectionResult.h"
#include "WebDocument.h"
#include "WebElement.h"
#include "WebFrame.h"
#include "WebFrameClient.h"
#include "WebFrameImpl.h"
#include "WebHelperPluginImpl.h"
#include "WebHitTestResult.h"
#include "WebInputEvent.h"
#include "WebSettings.h"
#include "WebViewClient.h"
#include "WebViewImpl.h"
#include "WebWidget.h"
#include "core/dom/Document.h"
#include "core/dom/Element.h"
#include "core/html/HTMLDocument.h"
#include "core/page/FrameView.h"
#include "public/platform/Platform.h"
#include "public/platform/WebSize.h"
#include "public/platform/WebThread.h"
#include "public/platform/WebUnitTestSupport.h"
#include "public/web/WebWidgetClient.h"

using namespace WebKit;
using WebKit::FrameTestHelpers::runPendingTasks;
using WebKit::URLTestHelpers::toKURL;

namespace {

enum HorizontalScrollbarState {
    NoHorizontalScrollbar,
    VisibleHorizontalScrollbar,
};

enum VerticalScrollbarState {
    NoVerticalScrollbar,
    VisibleVerticalScrollbar,
};

class TestData {
public:
    void setWebView(WebView* webView) { m_webView = static_cast<WebViewImpl*>(webView); }
    void setSize(const WebSize& newSize) { m_size = newSize; }
    HorizontalScrollbarState horizontalScrollbarState() const
    {
        return m_webView->hasHorizontalScrollbar() ? VisibleHorizontalScrollbar: NoHorizontalScrollbar;
    }
    VerticalScrollbarState verticalScrollbarState() const
    {
        return m_webView->hasVerticalScrollbar() ? VisibleVerticalScrollbar : NoVerticalScrollbar;
    }
    int width() const { return m_size.width; }
    int height() const { return m_size.height; }

private:
    WebSize m_size;
    WebViewImpl* m_webView;
};

class AutoResizeWebViewClient : public WebViewClient {
public:
    // WebViewClient methods
    virtual void didAutoResize(const WebSize& newSize) { m_testData.setSize(newSize); }

    // Local methods
    TestData& testData() { return m_testData; }

private:
    TestData m_testData;
};

class FormChangeWebViewClient : public WebViewClient {
public:
    // WebViewClient methods
    virtual void didChangeFormState(const WebNode& node)
    {
        m_focused = node.focused();
        m_called = true;
    }

    // Local methods
    void reset()
    {
        m_called = false;
        m_focused = false;
    }
    bool called() { return m_called; }
    bool focused() { return m_focused; }

private:
    bool m_called;
    bool m_focused;
};

class TapHandlingWebViewClient : public WebViewClient {
public:
    // WebViewClient methods
    virtual void didHandleGestureEvent(const WebGestureEvent& event, bool eventCancelled)
    {
        if (event.type == WebInputEvent::GestureTap) {
            m_tapX = event.x;
            m_tapY = event.y;
        } else if (event.type == WebInputEvent::GestureLongPress) {
            m_longpressX = event.x;
            m_longpressY = event.y;
        }
    }

    // Local methods
    void reset()
    {
        m_tapX = -1;
        m_tapY = -1;
        m_longpressX = -1;
        m_longpressY = -1;
    }
    int tapX() { return m_tapX; }
    int tapY() { return m_tapY; }
    int longpressX() { return m_longpressX; }
    int longpressY() { return m_longpressY; }

private:
    int m_tapX;
    int m_tapY;
    int m_longpressX;
    int m_longpressY;
};

class HelperPluginCreatingWebViewClient : public WebViewClient {
public:
    // WebViewClient methods
    virtual WebKit::WebWidget* createPopupMenu(WebKit::WebPopupType popupType) OVERRIDE
    {
        EXPECT_EQ(WebPopupTypeHelperPlugin, popupType);
        m_helperPluginWebWidget = WebKit::WebHelperPlugin::create(this);
        // The caller owns the object, but we retain a pointer for use in closeWidgetSoon().
        return m_helperPluginWebWidget;
    }

    virtual void initializeHelperPluginWebFrame(WebKit::WebHelperPlugin* plugin) OVERRIDE
    {
        ASSERT_TRUE(m_webFrameClient);
        plugin->initializeFrame(m_webFrameClient);
    }

    // WebWidgetClient methods
    virtual void closeWidgetSoon() OVERRIDE
    {
        ASSERT_TRUE(m_helperPluginWebWidget);
        m_helperPluginWebWidget->close();
        m_helperPluginWebWidget = 0;
    }

    // Local methods
    HelperPluginCreatingWebViewClient()
        :   m_helperPluginWebWidget(0)
        ,   m_webFrameClient(0)
    {
    }

    void setWebFrameClient(WebFrameClient* client) { m_webFrameClient = client; }

private:
    WebWidget* m_helperPluginWebWidget;
    WebFrameClient* m_webFrameClient;
};

class WebViewTest : public testing::Test {
public:
    WebViewTest()
        : m_baseURL("http://www.test.com/")
    {
    }

    virtual void TearDown()
    {
        Platform::current()->unitTestSupport()->unregisterAllMockedURLs();
    }

protected:
    void testAutoResize(const WebSize& minAutoResize, const WebSize& maxAutoResize,
                        const std::string& pageWidth, const std::string& pageHeight,
                        int expectedWidth, int expectedHeight,
                        HorizontalScrollbarState expectedHorizontalState, VerticalScrollbarState expectedVerticalState);

    void testTextInputType(WebTextInputType expectedType, const std::string& htmlFile);
    void testInputMode(const WebString& expectedInputMode, const std::string& htmlFile);

    std::string m_baseURL;
};

TEST_F(WebViewTest, SetBaseBackgroundColor)
{
    const WebColor kWhite    = 0xFFFFFFFF;
    const WebColor kBlue     = 0xFF0000FF;
    const WebColor kDarkCyan = 0xFF227788;
    const WebColor kTranslucentPutty = 0x80BFB196;

    WebView* webView = FrameTestHelpers::createWebView();
    EXPECT_EQ(kWhite, webView->backgroundColor());

    webView->setBaseBackgroundColor(kBlue);
    EXPECT_EQ(kBlue, webView->backgroundColor());

    WebURL baseURL = URLTestHelpers::toKURL("http://example.com/");
    webView->mainFrame()->loadHTMLString(
        "<html><head><style>body {background-color:#227788}</style></head></html>", baseURL);
    Platform::current()->unitTestSupport()->serveAsynchronousMockedRequests();
    EXPECT_EQ(kDarkCyan, webView->backgroundColor());

    webView->mainFrame()->loadHTMLString(
        "<html><head><style>body {background-color:rgba(255,0,0,0.5)}</style></head></html>", baseURL);
    Platform::current()->unitTestSupport()->serveAsynchronousMockedRequests();
    // Expected: red (50% alpha) blended atop base of kBlue.
    EXPECT_EQ(0xFF7F0080, webView->backgroundColor());

    webView->setBaseBackgroundColor(kTranslucentPutty);
    // Expected: red (50% alpha) blended atop kTranslucentPutty. Note the alpha.
    EXPECT_EQ(0xBFE93B32, webView->backgroundColor());

    webView->close();
}

TEST_F(WebViewTest, FocusIsInactive)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), "visible_iframe.html");
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "visible_iframe.html");

    webView->setFocus(true);
    webView->setIsActive(true);
    WebFrameImpl* frame = static_cast<WebFrameImpl*>(webView->mainFrame());
    EXPECT_TRUE(frame->frame()->document()->isHTMLDocument());

    WebCore::HTMLDocument* document = static_cast<WebCore::HTMLDocument*>(frame->frame()->document());
    EXPECT_TRUE(document->hasFocus());
    webView->setFocus(false);
    webView->setIsActive(false);
    EXPECT_FALSE(document->hasFocus());
    webView->setFocus(true);
    webView->setIsActive(true);
    EXPECT_TRUE(document->hasFocus());
    webView->setFocus(true);
    webView->setIsActive(false);
    EXPECT_FALSE(document->hasFocus());
    webView->setFocus(false);
    webView->setIsActive(true);
    EXPECT_FALSE(document->hasFocus());

    webView->close();
}

TEST_F(WebViewTest, ActiveState)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), "visible_iframe.html");
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "visible_iframe.html");

    ASSERT_TRUE(webView);

    webView->setIsActive(true);
    EXPECT_TRUE(webView->isActive());

    webView->setIsActive(false);
    EXPECT_FALSE(webView->isActive());

    webView->setIsActive(true);
    EXPECT_TRUE(webView->isActive());

    webView->close();
}

TEST_F(WebViewTest, HitTestResultAtWithPageScale)
{
    std::string url = m_baseURL + "specify_size.html?" + "50px" + ":" + "50px";
    URLTestHelpers::registerMockedURLLoad(toKURL(url), "specify_size.html");
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(url, true, 0);
    webView->resize(WebSize(100, 100));
    WebPoint hitPoint(75, 75);

    // Image is at top left quandrant, so should not hit it.
    WebHitTestResult negativeResult = webView->hitTestResultAt(hitPoint);
    ASSERT_EQ(WebNode::ElementNode, negativeResult.node().nodeType());
    EXPECT_FALSE(negativeResult.node().to<WebElement>().hasTagName("img"));
    negativeResult.reset();

    // Scale page up 2x so image should occupy the whole viewport.
    webView->setPageScaleFactor(2.0f, WebPoint(0, 0));
    WebHitTestResult positiveResult = webView->hitTestResultAt(hitPoint);
    ASSERT_EQ(WebNode::ElementNode, positiveResult.node().nodeType());
    EXPECT_TRUE(positiveResult.node().to<WebElement>().hasTagName("img"));
    positiveResult.reset();

    webView->close();
}

void WebViewTest::testAutoResize(const WebSize& minAutoResize, const WebSize& maxAutoResize,
                                 const std::string& pageWidth, const std::string& pageHeight,
                                 int expectedWidth, int expectedHeight,
                                 HorizontalScrollbarState expectedHorizontalState, VerticalScrollbarState expectedVerticalState)
{
    AutoResizeWebViewClient client;
    std::string url = m_baseURL + "specify_size.html?" + pageWidth + ":" + pageHeight;
    URLTestHelpers::registerMockedURLLoad(toKURL(url), "specify_size.html");
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(url, true, 0, &client);
    client.testData().setWebView(webView);

    WebFrameImpl* frame = static_cast<WebFrameImpl*>(webView->mainFrame());
    WebCore::FrameView* frameView = frame->frame()->view();
    frameView->layout();
    EXPECT_FALSE(frameView->layoutPending());
    EXPECT_FALSE(frameView->needsLayout());

    webView->enableAutoResizeMode(minAutoResize, maxAutoResize);
    EXPECT_TRUE(frameView->layoutPending());
    EXPECT_TRUE(frameView->needsLayout());
    frameView->layout();

    EXPECT_TRUE(frame->frame()->document()->isHTMLDocument());

    EXPECT_EQ(expectedWidth, client.testData().width());
    EXPECT_EQ(expectedHeight, client.testData().height());
    EXPECT_EQ(expectedHorizontalState, client.testData().horizontalScrollbarState());
    EXPECT_EQ(expectedVerticalState, client.testData().verticalScrollbarState());

    webView->close();
}

TEST_F(WebViewTest, DISABLED_AutoResizeMinimumSize)
{
    WebSize minAutoResize(91, 56);
    WebSize maxAutoResize(403, 302);
    std::string pageWidth = "91px";
    std::string pageHeight = "56px";
    int expectedWidth = 91;
    int expectedHeight = 56;
    testAutoResize(minAutoResize, maxAutoResize, pageWidth, pageHeight,
                   expectedWidth, expectedHeight, NoHorizontalScrollbar, NoVerticalScrollbar);
}

TEST_F(WebViewTest, AutoResizeHeightOverflowAndFixedWidth)
{
    WebSize minAutoResize(90, 95);
    WebSize maxAutoResize(90, 100);
    std::string pageWidth = "60px";
    std::string pageHeight = "200px";
    int expectedWidth = 90;
    int expectedHeight = 100;
    testAutoResize(minAutoResize, maxAutoResize, pageWidth, pageHeight,
                   expectedWidth, expectedHeight, NoHorizontalScrollbar, VisibleVerticalScrollbar);
}

TEST_F(WebViewTest, AutoResizeFixedHeightAndWidthOverflow)
{
    WebSize minAutoResize(90, 100);
    WebSize maxAutoResize(200, 100);
    std::string pageWidth = "300px";
    std::string pageHeight = "80px";
    int expectedWidth = 200;
    int expectedHeight = 100;
    testAutoResize(minAutoResize, maxAutoResize, pageWidth, pageHeight,
                   expectedWidth, expectedHeight, VisibleHorizontalScrollbar, NoVerticalScrollbar);
}

// Next three tests disabled for https://bugs.webkit.org/show_bug.cgi?id=92318 .
// It seems we can run three AutoResize tests, then the next one breaks.
TEST_F(WebViewTest, DISABLED_AutoResizeInBetweenSizes)
{
    WebSize minAutoResize(90, 95);
    WebSize maxAutoResize(200, 300);
    std::string pageWidth = "100px";
    std::string pageHeight = "200px";
    int expectedWidth = 100;
    int expectedHeight = 200;
    testAutoResize(minAutoResize, maxAutoResize, pageWidth, pageHeight,
                   expectedWidth, expectedHeight, NoHorizontalScrollbar, NoVerticalScrollbar);
}

TEST_F(WebViewTest, DISABLED_AutoResizeOverflowSizes)
{
    WebSize minAutoResize(90, 95);
    WebSize maxAutoResize(200, 300);
    std::string pageWidth = "300px";
    std::string pageHeight = "400px";
    int expectedWidth = 200;
    int expectedHeight = 300;
    testAutoResize(minAutoResize, maxAutoResize, pageWidth, pageHeight,
                   expectedWidth, expectedHeight, VisibleHorizontalScrollbar, VisibleVerticalScrollbar);
}

TEST_F(WebViewTest, DISABLED_AutoResizeMaxSize)
{
    WebSize minAutoResize(90, 95);
    WebSize maxAutoResize(200, 300);
    std::string pageWidth = "200px";
    std::string pageHeight = "300px";
    int expectedWidth = 200;
    int expectedHeight = 300;
    testAutoResize(minAutoResize, maxAutoResize, pageWidth, pageHeight,
                   expectedWidth, expectedHeight, NoHorizontalScrollbar, NoVerticalScrollbar);
}

void WebViewTest::testTextInputType(WebTextInputType expectedType, const std::string& htmlFile)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8(htmlFile.c_str()));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + htmlFile);
    webView->setInitialFocus(false);
    EXPECT_EQ(expectedType, webView->textInputInfo().type);
    webView->close();
}

TEST_F(WebViewTest, TextInputType)
{
    testTextInputType(WebTextInputTypeText, "input_field_default.html");
    testTextInputType(WebTextInputTypePassword, "input_field_password.html");
    testTextInputType(WebTextInputTypeEmail, "input_field_email.html");
    testTextInputType(WebTextInputTypeSearch, "input_field_search.html");
    testTextInputType(WebTextInputTypeNumber, "input_field_number.html");
    testTextInputType(WebTextInputTypeTelephone, "input_field_tel.html");
    testTextInputType(WebTextInputTypeURL, "input_field_url.html");
}

void WebViewTest::testInputMode(const WebString& expectedInputMode, const std::string& htmlFile)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8(htmlFile.c_str()));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + htmlFile);
    webView->setInitialFocus(false);
    EXPECT_EQ(expectedInputMode, webView->textInputInfo().inputMode);
    webView->close();
}

TEST_F(WebViewTest, InputMode)
{
    testInputMode(WebString(), "input_mode_default.html");
    testInputMode(WebString("unknown"), "input_mode_default_unknown.html");
    testInputMode(WebString("verbatim"), "input_mode_default_verbatim.html");
    testInputMode(WebString("verbatim"), "input_mode_type_text_verbatim.html");
    testInputMode(WebString("verbatim"), "input_mode_type_search_verbatim.html");
    testInputMode(WebString(), "input_mode_type_url_verbatim.html");
    testInputMode(WebString("verbatim"), "input_mode_textarea_verbatim.html");
}

TEST_F(WebViewTest, SetEditableSelectionOffsetsAndTextInputInfo)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("input_field_populated.html"));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "input_field_populated.html");
    webView->setInitialFocus(false);
    webView->setEditableSelectionOffsets(5, 13);
    WebFrameImpl* frame = static_cast<WebFrameImpl*>(webView->mainFrame());
    EXPECT_EQ("56789abc", frame->selectionAsText());
    WebTextInputInfo info = webView->textInputInfo();
    EXPECT_EQ("0123456789abcdefghijklmnopqrstuvwxyz", info.value);
    EXPECT_EQ(5, info.selectionStart);
    EXPECT_EQ(13, info.selectionEnd);
    EXPECT_EQ(-1, info.compositionStart);
    EXPECT_EQ(-1, info.compositionEnd);
    webView->close();

    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("content_editable_populated.html"));
    webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "content_editable_populated.html");
    webView->setInitialFocus(false);
    webView->setEditableSelectionOffsets(8, 19);
    frame = static_cast<WebFrameImpl*>(webView->mainFrame());
    EXPECT_EQ("89abcdefghi", frame->selectionAsText());
    info = webView->textInputInfo();
    EXPECT_EQ("0123456789abcdefghijklmnopqrstuvwxyz", info.value);
    EXPECT_EQ(8, info.selectionStart);
    EXPECT_EQ(19, info.selectionEnd);
    EXPECT_EQ(-1, info.compositionStart);
    EXPECT_EQ(-1, info.compositionEnd);
    webView->close();
}

TEST_F(WebViewTest, ConfirmCompositionCursorPositionChange)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("input_field_populated.html"));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "input_field_populated.html");
    webView->setInitialFocus(false);

    // Set up a composition that needs to be committed.
    std::string compositionText("hello");

    WebVector<WebCompositionUnderline> emptyUnderlines;
    webView->setComposition(WebString::fromUTF8(compositionText.c_str()), emptyUnderlines, 3, 3);

    WebTextInputInfo info = webView->textInputInfo();
    EXPECT_EQ("hello", std::string(info.value.utf8().data()));
    EXPECT_EQ(3, info.selectionStart);
    EXPECT_EQ(3, info.selectionEnd);
    EXPECT_EQ(0, info.compositionStart);
    EXPECT_EQ(5, info.compositionEnd);

    webView->confirmComposition(WebWidget::KeepSelection);
    info = webView->textInputInfo();
    EXPECT_EQ(3, info.selectionStart);
    EXPECT_EQ(3, info.selectionEnd);
    EXPECT_EQ(-1, info.compositionStart);
    EXPECT_EQ(-1, info.compositionEnd);

    webView->setComposition(WebString::fromUTF8(compositionText.c_str()), emptyUnderlines, 3, 3);
    info = webView->textInputInfo();
    EXPECT_EQ("helhellolo", std::string(info.value.utf8().data()));
    EXPECT_EQ(6, info.selectionStart);
    EXPECT_EQ(6, info.selectionEnd);
    EXPECT_EQ(3, info.compositionStart);
    EXPECT_EQ(8, info.compositionEnd);

    webView->confirmComposition(WebWidget::DoNotKeepSelection);
    info = webView->textInputInfo();
    EXPECT_EQ(8, info.selectionStart);
    EXPECT_EQ(8, info.selectionEnd);
    EXPECT_EQ(-1, info.compositionStart);
    EXPECT_EQ(-1, info.compositionEnd);

    webView->close();
}

TEST_F(WebViewTest, InsertNewLinePlacementAfterConfirmComposition)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("text_area_populated.html"));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "text_area_populated.html");
    webView->setInitialFocus(false);

    WebVector<WebCompositionUnderline> emptyUnderlines;

    webView->setEditableSelectionOffsets(4, 4);
    webView->setCompositionFromExistingText(8, 12, emptyUnderlines);

    WebTextInputInfo info = webView->textInputInfo();
    EXPECT_EQ("0123456789abcdefghijklmnopqrstuvwxyz", std::string(info.value.utf8().data()));
    EXPECT_EQ(4, info.selectionStart);
    EXPECT_EQ(4, info.selectionEnd);
    EXPECT_EQ(8, info.compositionStart);
    EXPECT_EQ(12, info.compositionEnd);

    webView->confirmComposition(WebWidget::KeepSelection);
    info = webView->textInputInfo();
    EXPECT_EQ(4, info.selectionStart);
    EXPECT_EQ(4, info.selectionEnd);
    EXPECT_EQ(-1, info.compositionStart);
    EXPECT_EQ(-1, info.compositionEnd);

    std::string compositionText("\n");
    webView->confirmComposition(WebString::fromUTF8(compositionText.c_str()));
    info = webView->textInputInfo();
    EXPECT_EQ(5, info.selectionStart);
    EXPECT_EQ(5, info.selectionEnd);
    EXPECT_EQ(-1, info.compositionStart);
    EXPECT_EQ(-1, info.compositionEnd);
    EXPECT_EQ("0123\n456789abcdefghijklmnopqrstuvwxyz", std::string(info.value.utf8().data()));

    webView->close();
}

TEST_F(WebViewTest, FormChange)
{
    FormChangeWebViewClient client;
    client.reset();
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("input_field_set_value_while_focused.html"));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "input_field_set_value_while_focused.html", true, 0, &client);
    EXPECT_TRUE(client.called());
    EXPECT_TRUE(client.focused());
    client.reset();
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("input_field_set_value_while_not_focused.html"));
    webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "input_field_set_value_while_not_focused.html", true, 0, &client);
    EXPECT_TRUE(client.called());
    EXPECT_FALSE(client.focused());
    webView->close();
}

TEST_F(WebViewTest, ExtendSelectionAndDelete)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("input_field_populated.html"));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "input_field_populated.html");
    webView->setInitialFocus(false);
    webView->setEditableSelectionOffsets(10, 10);
    webView->extendSelectionAndDelete(5, 8);
    WebTextInputInfo info = webView->textInputInfo();
    EXPECT_EQ("01234ijklmnopqrstuvwxyz", std::string(info.value.utf8().data()));
    EXPECT_EQ(5, info.selectionStart);
    EXPECT_EQ(5, info.selectionEnd);
    webView->extendSelectionAndDelete(10, 0);
    info = webView->textInputInfo();
    EXPECT_EQ("ijklmnopqrstuvwxyz", std::string(info.value.utf8().data()));
    webView->close();
}

TEST_F(WebViewTest, SetCompositionFromExistingText)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("input_field_populated.html"));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "input_field_populated.html");
    webView->setInitialFocus(false);
    WebVector<WebCompositionUnderline> underlines(static_cast<size_t>(1));
    underlines[0] = WebKit::WebCompositionUnderline(0, 4, 0, false);
    webView->setEditableSelectionOffsets(4, 10);
    webView->setCompositionFromExistingText(8, 12, underlines);
    WebVector<WebCompositionUnderline> underlineResults = static_cast<WebViewImpl*>(webView)->compositionUnderlines();
    EXPECT_EQ(8u, underlineResults[0].startOffset);
    EXPECT_EQ(12u, underlineResults[0].endOffset);
    WebTextInputInfo info = webView->textInputInfo();
    EXPECT_EQ(4, info.selectionStart);
    EXPECT_EQ(10, info.selectionEnd);
    EXPECT_EQ(8, info.compositionStart);
    EXPECT_EQ(12, info.compositionEnd);
    WebVector<WebCompositionUnderline> emptyUnderlines;
    webView->setCompositionFromExistingText(0, 0, emptyUnderlines);
    info = webView->textInputInfo();
    EXPECT_EQ(4, info.selectionStart);
    EXPECT_EQ(10, info.selectionEnd);
    EXPECT_EQ(-1, info.compositionStart);
    EXPECT_EQ(-1, info.compositionEnd);
    webView->close();
}

TEST_F(WebViewTest, SetCompositionFromExistingTextInTextArea)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("text_area_populated.html"));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "text_area_populated.html");
    webView->setInitialFocus(false);
    WebVector<WebCompositionUnderline> underlines(static_cast<size_t>(1));
    underlines[0] = WebKit::WebCompositionUnderline(0, 4, 0, false);
    webView->setEditableSelectionOffsets(27, 27);
    std::string newLineText("\n");
    webView->confirmComposition(WebString::fromUTF8(newLineText.c_str()));
    WebTextInputInfo info = webView->textInputInfo();
    EXPECT_EQ("0123456789abcdefghijklmnopq\nrstuvwxyz", std::string(info.value.utf8().data()));

    webView->setEditableSelectionOffsets(31, 31);
    webView->setCompositionFromExistingText(30, 34, underlines);
    WebVector<WebCompositionUnderline> underlineResults = static_cast<WebViewImpl*>(webView)->compositionUnderlines();
    EXPECT_EQ(30u, underlineResults[0].startOffset);
    EXPECT_EQ(34u, underlineResults[0].endOffset);
    info = webView->textInputInfo();
    EXPECT_EQ("0123456789abcdefghijklmnopq\nrstuvwxyz", std::string(info.value.utf8().data()));
    EXPECT_EQ(31, info.selectionStart);
    EXPECT_EQ(31, info.selectionEnd);
    EXPECT_EQ(30, info.compositionStart);
    EXPECT_EQ(34, info.compositionEnd);

    std::string compositionText("yolo");
    webView->confirmComposition(WebString::fromUTF8(compositionText.c_str()));
    info = webView->textInputInfo();
    EXPECT_EQ("0123456789abcdefghijklmnopq\nrsyoloxyz", std::string(info.value.utf8().data()));
    EXPECT_EQ(34, info.selectionStart);
    EXPECT_EQ(34, info.selectionEnd);
    EXPECT_EQ(-1, info.compositionStart);
    EXPECT_EQ(-1, info.compositionEnd);
    webView->close();
}

TEST_F(WebViewTest, IsSelectionAnchorFirst)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("input_field_populated.html"));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "input_field_populated.html");
    WebFrame* frame = webView->mainFrame();

    webView->setInitialFocus(false);
    webView->setEditableSelectionOffsets(4, 10);
    EXPECT_TRUE(webView->isSelectionAnchorFirst());
    WebRect anchor;
    WebRect focus;
    webView->selectionBounds(anchor, focus);
    frame->selectRange(WebPoint(focus.x, focus.y), WebPoint(anchor.x, anchor.y));
    EXPECT_FALSE(webView->isSelectionAnchorFirst());
    webView->close();
}

TEST_F(WebViewTest, HistoryResetScrollAndScaleState)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("hello_world.html"));
    WebViewImpl* webViewImpl = static_cast<WebViewImpl*>(FrameTestHelpers::createWebViewAndLoad(m_baseURL + "hello_world.html"));
    webViewImpl->resize(WebSize(640, 480));
    webViewImpl->layout();
    EXPECT_EQ(0, webViewImpl->mainFrame()->scrollOffset().width);
    EXPECT_EQ(0, webViewImpl->mainFrame()->scrollOffset().height);

    // Make the page scale and scroll with the given paremeters.
    webViewImpl->setPageScaleFactor(2.0f, WebPoint(116, 84));
    EXPECT_EQ(2.0f, webViewImpl->pageScaleFactor());
    EXPECT_EQ(116, webViewImpl->mainFrame()->scrollOffset().width);
    EXPECT_EQ(84, webViewImpl->mainFrame()->scrollOffset().height);
    webViewImpl->page()->mainFrame()->loader()->history()->saveDocumentAndScrollState();

    // Confirm that restoring the page state restores the parameters.
    webViewImpl->setPageScaleFactor(1.5f, WebPoint(16, 24));
    EXPECT_EQ(1.5f, webViewImpl->pageScaleFactor());
    EXPECT_EQ(16, webViewImpl->mainFrame()->scrollOffset().width);
    EXPECT_EQ(24, webViewImpl->mainFrame()->scrollOffset().height);
    // WebViewImpl::setPageScaleFactor is performing user scrolls, which will set the
    // wasScrolledByUser flag on the main frame, and prevent restoreScrollPositionAndViewState
    // from restoring the scrolling position.
    webViewImpl->page()->mainFrame()->view()->setWasScrolledByUser(false);
    webViewImpl->page()->mainFrame()->loader()->history()->restoreScrollPositionAndViewState();
    EXPECT_EQ(2.0f, webViewImpl->pageScaleFactor());
    EXPECT_EQ(116, webViewImpl->mainFrame()->scrollOffset().width);
    EXPECT_EQ(84, webViewImpl->mainFrame()->scrollOffset().height);
    webViewImpl->page()->mainFrame()->loader()->history()->saveDocumentAndScrollState();

    // Confirm that resetting the page state resets the saved scroll position.
    // The HistoryController treats a page scale factor of 0.0f as special and avoids
    // restoring it to the WebView.
    webViewImpl->resetScrollAndScaleState();
    EXPECT_EQ(1.0f, webViewImpl->pageScaleFactor());
    EXPECT_EQ(0, webViewImpl->mainFrame()->scrollOffset().width);
    EXPECT_EQ(0, webViewImpl->mainFrame()->scrollOffset().height);
    webViewImpl->page()->mainFrame()->loader()->history()->restoreScrollPositionAndViewState();
    EXPECT_EQ(1.0f, webViewImpl->pageScaleFactor());
    EXPECT_EQ(0, webViewImpl->mainFrame()->scrollOffset().width);
    EXPECT_EQ(0, webViewImpl->mainFrame()->scrollOffset().height);
    webViewImpl->close();
}

class EnterFullscreenWebViewClient : public WebViewClient {
public:
    // WebViewClient methods
    virtual bool enterFullScreen() { return true; }
    virtual void exitFullScreen() { }
};


TEST_F(WebViewTest, EnterFullscreenResetScrollAndScaleState)
{
    EnterFullscreenWebViewClient client;
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("hello_world.html"));
    WebViewImpl* webViewImpl = static_cast<WebViewImpl*>(FrameTestHelpers::createWebViewAndLoad(m_baseURL + "hello_world.html", true, 0, &client));
    webViewImpl->settings()->setFullScreenEnabled(true);
    webViewImpl->resize(WebSize(640, 480));
    webViewImpl->layout();
    EXPECT_EQ(0, webViewImpl->mainFrame()->scrollOffset().width);
    EXPECT_EQ(0, webViewImpl->mainFrame()->scrollOffset().height);

    // Make the page scale and scroll with the given paremeters.
    webViewImpl->setPageScaleFactor(2.0f, WebPoint(116, 84));
    EXPECT_EQ(2.0f, webViewImpl->pageScaleFactor());
    EXPECT_EQ(116, webViewImpl->mainFrame()->scrollOffset().width);
    EXPECT_EQ(84, webViewImpl->mainFrame()->scrollOffset().height);

    RefPtr<WebCore::Element> element = static_cast<PassRefPtr<WebCore::Element> >(webViewImpl->mainFrame()->document().body());
    webViewImpl->enterFullScreenForElement(element.get());
    webViewImpl->willEnterFullScreen();
    webViewImpl->didEnterFullScreen();

    // Page scale factor must be 1.0 during fullscreen for elements to be sized
    // properly.
    EXPECT_EQ(1.0f, webViewImpl->pageScaleFactor());

    // Make sure fullscreen nesting doesn't disrupt scroll/scale saving.
    RefPtr<WebCore::Element> otherElement = static_cast<PassRefPtr<WebCore::Element> >(webViewImpl->mainFrame()->document().head());
    webViewImpl->enterFullScreenForElement(otherElement.get());

    // Confirm that exiting fullscreen restores the parameters.
    webViewImpl->willExitFullScreen();
    webViewImpl->didExitFullScreen();
    EXPECT_EQ(2.0f, webViewImpl->pageScaleFactor());
    EXPECT_EQ(116, webViewImpl->mainFrame()->scrollOffset().width);
    EXPECT_EQ(84, webViewImpl->mainFrame()->scrollOffset().height);

    webViewImpl->close();
}

class ContentDetectorClient : public WebViewClient {
public:
    ContentDetectorClient() { reset(); }

    virtual WebContentDetectionResult detectContentAround(const WebHitTestResult& hitTest) OVERRIDE
    {
        m_contentDetectionRequested = true;
        return m_contentDetectionResult;
    }

    virtual void scheduleContentIntent(const WebURL& url) OVERRIDE
    {
        m_scheduledIntentURL = url;
    }

    virtual void cancelScheduledContentIntents() OVERRIDE
    {
        m_pendingIntentsCancelled = true;
    }

    void reset()
    {
        m_contentDetectionRequested = false;
        m_pendingIntentsCancelled = false;
        m_scheduledIntentURL = WebURL();
        m_contentDetectionResult = WebContentDetectionResult();
    }

    bool contentDetectionRequested() const { return m_contentDetectionRequested; }
    bool pendingIntentsCancelled() const { return m_pendingIntentsCancelled; }
    const WebURL& scheduledIntentURL() const { return m_scheduledIntentURL; }
    void setContentDetectionResult(const WebContentDetectionResult& result) { m_contentDetectionResult = result; }

private:
    bool m_contentDetectionRequested;
    bool m_pendingIntentsCancelled;
    WebURL m_scheduledIntentURL;
    WebContentDetectionResult m_contentDetectionResult;
};

static bool tapElementById(WebView* webView, WebInputEvent::Type type, const WebString& id)
{
    ASSERT(webView);
    RefPtr<WebCore::Element> element = static_cast<PassRefPtr<WebCore::Element> >(webView->mainFrame()->document().getElementById(id));
    if (!element)
        return false;

    element->scrollIntoViewIfNeeded();
    WebCore::IntPoint center = element->screenRect().center();

    WebGestureEvent event;
    event.type = type;
    event.x = center.x();
    event.y = center.y();

    webView->handleInputEvent(event);
    runPendingTasks();
    return true;
}

TEST_F(WebViewTest, DetectContentAroundPosition)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("content_listeners.html"));

    ContentDetectorClient client;
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "content_listeners.html", true, 0, &client);
    webView->resize(WebSize(500, 300));
    webView->layout();
    runPendingTasks();

    WebString clickListener = WebString::fromUTF8("clickListener");
    WebString touchstartListener = WebString::fromUTF8("touchstartListener");
    WebString mousedownListener = WebString::fromUTF8("mousedownListener");
    WebString noListener = WebString::fromUTF8("noListener");
    WebString link = WebString::fromUTF8("link");

    // Ensure content detection is not requested for nodes listening to click,
    // mouse or touch events when we do simple taps.
    EXPECT_TRUE(tapElementById(webView, WebInputEvent::GestureTap, clickListener));
    EXPECT_FALSE(client.contentDetectionRequested());
    client.reset();

    EXPECT_TRUE(tapElementById(webView, WebInputEvent::GestureTap, touchstartListener));
    EXPECT_FALSE(client.contentDetectionRequested());
    client.reset();

    EXPECT_TRUE(tapElementById(webView, WebInputEvent::GestureTap, mousedownListener));
    EXPECT_FALSE(client.contentDetectionRequested());
    client.reset();

    // Content detection should work normally without these event listeners.
    // The click listener in the body should be ignored as a special case.
    EXPECT_TRUE(tapElementById(webView, WebInputEvent::GestureTap, noListener));
    EXPECT_TRUE(client.contentDetectionRequested());
    EXPECT_FALSE(client.scheduledIntentURL().isValid());

    WebURL intentURL = toKURL(m_baseURL);
    client.setContentDetectionResult(WebContentDetectionResult(WebRange(), WebString(), intentURL));
    EXPECT_TRUE(tapElementById(webView, WebInputEvent::GestureTap, noListener));
    EXPECT_TRUE(client.scheduledIntentURL() == intentURL);

    // Tapping elsewhere should cancel the scheduled intent.
    WebGestureEvent event;
    event.type = WebInputEvent::GestureTap;
    webView->handleInputEvent(event);
    runPendingTasks();
    EXPECT_TRUE(client.pendingIntentsCancelled());
    webView->close();
}

TEST_F(WebViewTest, ClientTapHandling)
{
    TapHandlingWebViewClient client;
    client.reset();
    WebView* webView = FrameTestHelpers::createWebViewAndLoad("about:blank", true, 0, &client);
    WebGestureEvent event;
    event.type = WebInputEvent::GestureTap;
    event.x = 3;
    event.y = 8;
    webView->handleInputEvent(event);
    runPendingTasks();
    EXPECT_EQ(3, client.tapX());
    EXPECT_EQ(8, client.tapY());
    client.reset();
    event.type = WebInputEvent::GestureLongPress;
    event.x = 25;
    event.y = 7;
    webView->handleInputEvent(event);
    runPendingTasks();
    EXPECT_EQ(25, client.longpressX());
    EXPECT_EQ(7, client.longpressY());
    webView->close();
}

#if OS(ANDROID)
TEST_F(WebViewTest, LongPressSelection)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("longpress_selection.html"));

    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "longpress_selection.html", true);
    webView->resize(WebSize(500, 300));
    webView->layout();
    runPendingTasks();

    WebString target = WebString::fromUTF8("target");
    WebString onselectstartfalse = WebString::fromUTF8("onselectstartfalse");
    WebFrameImpl* frame = static_cast<WebFrameImpl*>(webView->mainFrame());

    EXPECT_TRUE(tapElementById(webView, WebInputEvent::GestureLongPress, onselectstartfalse));
    EXPECT_EQ("", std::string(frame->selectionAsText().utf8().data()));
    EXPECT_TRUE(tapElementById(webView, WebInputEvent::GestureLongPress, target));
    EXPECT_EQ("testword", std::string(frame->selectionAsText().utf8().data()));
    webView->close();
}
#endif

TEST_F(WebViewTest, SelectionOnDisabledInput)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("selection_disabled.html"));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "selection_disabled.html", true);
    webView->resize(WebSize(640, 480));
    webView->layout();
    runPendingTasks();

    std::string testWord = "This text should be selected.";

    WebFrameImpl* frame = static_cast<WebFrameImpl*>(webView->mainFrame());
    EXPECT_EQ(testWord, std::string(frame->selectionAsText().utf8().data()));

    size_t location;
    size_t length;
    WebViewImpl* webViewImpl = static_cast<WebViewImpl*>(webView);

    EXPECT_TRUE(webViewImpl->caretOrSelectionRange(&location, &length));
    EXPECT_EQ(location, 0UL);
    EXPECT_EQ(length, testWord.length());

    webView->close();
}

TEST_F(WebViewTest, SelectionOnReadOnlyInput)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("selection_readonly.html"));
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "selection_readonly.html", true);
    webView->resize(WebSize(640, 480));
    webView->layout();
    runPendingTasks();

    std::string testWord = "This text should be selected.";

    WebFrameImpl* frame = static_cast<WebFrameImpl*>(webView->mainFrame());
    EXPECT_EQ(testWord, std::string(frame->selectionAsText().utf8().data()));

    size_t location;
    size_t length;
    WebViewImpl* webViewImpl = static_cast<WebViewImpl*>(webView);

    EXPECT_TRUE(webViewImpl->caretOrSelectionRange(&location, &length));
    EXPECT_EQ(location, 0UL);
    EXPECT_EQ(length, testWord.length());

    webView->close();
}

class MockAutofillClient : public WebAutofillClient {
public:
    MockAutofillClient()
        : m_ignoreTextChanges(false)
        , m_textChangesWhileIgnored(0)
        , m_textChangesWhileNotIgnored(0) { }

    virtual ~MockAutofillClient() { }

    virtual void setIgnoreTextChanges(bool ignore) OVERRIDE { m_ignoreTextChanges = ignore; }
    virtual void textFieldDidChange(const WebInputElement&) OVERRIDE
    {
        if (m_ignoreTextChanges)
            ++m_textChangesWhileIgnored;
        else
            ++m_textChangesWhileNotIgnored;
    }

    void clearChangeCounts()
    {
        m_textChangesWhileIgnored = 0;
        m_textChangesWhileNotIgnored = 0;
    }

    int textChangesWhileIgnored() { return m_textChangesWhileIgnored; }
    int textChangesWhileNotIgnored() { return m_textChangesWhileNotIgnored; }

private:
    bool m_ignoreTextChanges;
    int m_textChangesWhileIgnored;
    int m_textChangesWhileNotIgnored;
};


TEST_F(WebViewTest, LosingFocusDoesNotTriggerAutofillTextChange)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("input_field_populated.html"));
    MockAutofillClient client;
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "input_field_populated.html");
    webView->setAutofillClient(&client);
    webView->setInitialFocus(false);

    // Set up a composition that needs to be committed.
    WebVector<WebCompositionUnderline> emptyUnderlines;
    webView->setEditableSelectionOffsets(4, 10);
    webView->setCompositionFromExistingText(8, 12, emptyUnderlines);
    WebTextInputInfo info = webView->textInputInfo();
    EXPECT_EQ(4, info.selectionStart);
    EXPECT_EQ(10, info.selectionEnd);
    EXPECT_EQ(8, info.compositionStart);
    EXPECT_EQ(12, info.compositionEnd);

    // Clear the focus and track that the subsequent composition commit does not trigger a
    // text changed notification for autofill.
    client.clearChangeCounts();
    webView->setFocus(false);
    EXPECT_EQ(1, client.textChangesWhileIgnored());
    EXPECT_EQ(0, client.textChangesWhileNotIgnored());

    webView->setAutofillClient(0);
    webView->close();
}

TEST_F(WebViewTest, ConfirmCompositionTriggersAutofillTextChange)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("input_field_populated.html"));
    MockAutofillClient client;
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "input_field_populated.html");
    webView->setAutofillClient(&client);
    webView->setInitialFocus(false);

    // Set up a composition that needs to be committed.
    std::string compositionText("testingtext");

    WebVector<WebCompositionUnderline> emptyUnderlines;
    webView->setComposition(WebString::fromUTF8(compositionText.c_str()), emptyUnderlines, 0, compositionText.length());

    WebTextInputInfo info = webView->textInputInfo();
    EXPECT_EQ(0, info.selectionStart);
    EXPECT_EQ((int) compositionText.length(), info.selectionEnd);
    EXPECT_EQ(0, info.compositionStart);
    EXPECT_EQ((int) compositionText.length(), info.compositionEnd);

    client.clearChangeCounts();
    webView->confirmComposition();
    EXPECT_EQ(0, client.textChangesWhileIgnored());
    EXPECT_EQ(1, client.textChangesWhileNotIgnored());

    webView->setAutofillClient(0);
    webView->close();
}

TEST_F(WebViewTest, SetCompositionFromExistingTextTriggersAutofillTextChange)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("input_field_populated.html"));
    MockAutofillClient client;
    WebView* webView = FrameTestHelpers::createWebViewAndLoad(m_baseURL + "input_field_populated.html", true);
    webView->setAutofillClient(&client);
    webView->setInitialFocus(false);

    WebVector<WebCompositionUnderline> emptyUnderlines;

    client.clearChangeCounts();
    webView->setCompositionFromExistingText(8, 12, emptyUnderlines);

    WebTextInputInfo info = webView->textInputInfo();
    EXPECT_EQ("0123456789abcdefghijklmnopqrstuvwxyz", std::string(info.value.utf8().data()));
    EXPECT_EQ(8, info.compositionStart);
    EXPECT_EQ(12, info.compositionEnd);

    EXPECT_EQ(0, client.textChangesWhileIgnored());
    EXPECT_EQ(0, client.textChangesWhileNotIgnored());

    WebDocument document = webView->mainFrame()->document();
    EXPECT_EQ(WebString::fromUTF8("none"),  document.getElementById("inputEvent").firstChild().nodeValue());

    webView->setAutofillClient(0);
    webView->close();
}

TEST_F(WebViewTest, ShadowRoot)
{
    URLTestHelpers::registerMockedURLFromBaseURL(WebString::fromUTF8(m_baseURL.c_str()), WebString::fromUTF8("shadow_dom_test.html"));
    WebViewImpl* webViewImpl = static_cast<WebViewImpl*>(FrameTestHelpers::createWebViewAndLoad(m_baseURL + "shadow_dom_test.html", true));

    WebDocument document = webViewImpl->mainFrame()->document();
    {
        WebElement elementWithShadowRoot = document.getElementById("shadowroot");
        EXPECT_FALSE(elementWithShadowRoot.isNull());
        WebNode shadowRoot = elementWithShadowRoot.shadowRoot();
        EXPECT_FALSE(shadowRoot.isNull());
    }
    {
        WebElement elementWithoutShadowRoot = document.getElementById("noshadowroot");
        EXPECT_FALSE(elementWithoutShadowRoot.isNull());
        WebNode shadowRoot = elementWithoutShadowRoot.shadowRoot();
        EXPECT_TRUE(shadowRoot.isNull());
    }
    webViewImpl->close();
}

TEST_F(WebViewTest, HelperPlugin)
{
    HelperPluginCreatingWebViewClient client;
    WebViewImpl* webViewImpl = static_cast<WebViewImpl*>(FrameTestHelpers::createWebView(true, 0, &client));

    WebFrameImpl* frame = static_cast<WebFrameImpl*>(webViewImpl->mainFrame());
    client.setWebFrameClient(frame->client());

    WebHelperPluginImpl* helperPlugin = webViewImpl->createHelperPlugin("dummy-plugin-type", frame->document());
    EXPECT_TRUE(helperPlugin);
    EXPECT_EQ(0, helperPlugin->getPlugin()); // Invalid plugin type means no plugin.

    webViewImpl->closeHelperPluginSoon(helperPlugin);

    webViewImpl->close();
}

}
