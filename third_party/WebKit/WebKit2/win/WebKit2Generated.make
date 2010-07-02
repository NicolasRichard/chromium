all:
    touch "$(WEBKITOUTPUTDIR)\buildfailed"
    -mkdir 2>NUL "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WebKit2.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKBackForwardList.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKBackForwardListItem.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKBase.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKContext.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKContextPrivate.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKFrame.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKFramePolicyListener.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKNavigationData.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPage.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPageNamespace.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKPreferences.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKString.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\WKURL.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\cf\WKStringCF.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\C\cf\WKURLCF.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\cpp\WKRetainPtr.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\win\WKBaseWin.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\UIProcess\API\win\WKView.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleBase.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundle.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    xcopy /y /d "..\WebProcess\InjectedBundle\API\c\WKBundleInitialize.h" "$(WEBKITOUTPUTDIR)\include\WebKit2"
    -del "$(WEBKITOUTPUTDIR)\buildfailed"

clean:
    -del "$(WEBKITOUTPUTDIR)\buildfailed"
    -del /s /q "$(WEBKITOUTPUTDIR)\include\WebKit2"
