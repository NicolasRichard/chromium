all:
    touch "$(WEBKITOUTPUTDIR)\buildfailed"
    -mkdir 2>NUL "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WebKit2.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKArray.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKBackForwardList.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKBackForwardListItem.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKBase.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKContext.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKContextPrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKData.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKDictionary.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKError.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKFormSubmissionListener.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKFrame.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKFramePolicyListener.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKNavigationData.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPage.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPageNamespace.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPreferences.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPreferencesPrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKString.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKType.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKURL.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKURLRequest.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKURLResponse.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\cf\WKStringCF.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\cf\WKURLCF.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\cf\WKURLRequestCF.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\cf\WKURLResponseCF.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\win\WKContextPrivateWin.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\cpp\WKRetainPtr.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\win\WKBaseWin.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\win\WKView.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundle.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleBase.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleFrame.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleFramePrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleInitialize.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleNode.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleNodeHandle.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundlePage.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundlePagePrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundlePrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleRange.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleScriptWorld.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    -del "$(WEBKITOUTPUTDIR)\buildfailed"

clean:
    -del "$(WEBKITOUTPUTDIR)\buildfailed"
    -del /s /q "$(WEBKITOUTPUTDIR)\include\WebKit2"
