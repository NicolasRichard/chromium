/*
 * Copyright (C) 2003, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *           (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

#ifndef Settings_h
#define Settings_h

#include "AtomicString.h"
#include "EditingBehaviorTypes.h"
#include "FontRenderingMode.h"
#include "KURL.h"
#include "ZoomMode.h"

namespace WebCore {

    class Page;

    enum EditableLinkBehavior {
        EditableLinkDefaultBehavior,
        EditableLinkAlwaysLive,
        EditableLinkOnlyLiveWithShiftKey,
        EditableLinkLiveWhenNotFocused,
        EditableLinkNeverLive
    };

    enum TextDirectionSubmenuInclusionBehavior {
        TextDirectionSubmenuNeverIncluded,
        TextDirectionSubmenuAutomaticallyIncluded,
        TextDirectionSubmenuAlwaysIncluded
    };

    class Settings : public Noncopyable {
    public:
        Settings(Page*);

        void setStandardFontFamily(const AtomicString&);
        const AtomicString& standardFontFamily() const { return m_standardFontFamily; }

        void setFixedFontFamily(const AtomicString&);
        const AtomicString& fixedFontFamily() const { return m_fixedFontFamily; }

        void setSerifFontFamily(const AtomicString&);
        const AtomicString& serifFontFamily() const { return m_serifFontFamily; }

        void setSansSerifFontFamily(const AtomicString&);
        const AtomicString& sansSerifFontFamily() const { return m_sansSerifFontFamily; }

        void setCursiveFontFamily(const AtomicString&);
        const AtomicString& cursiveFontFamily() const { return m_cursiveFontFamily; }

        void setFantasyFontFamily(const AtomicString&);
        const AtomicString& fantasyFontFamily() const { return m_fantasyFontFamily; }

        void setMinimumFontSize(int);
        int minimumFontSize() const { return m_minimumFontSize; }

        void setMinimumLogicalFontSize(int);
        int minimumLogicalFontSize() const { return m_minimumLogicalFontSize; }

        void setDefaultFontSize(int);
        int defaultFontSize() const { return m_defaultFontSize; }

        void setDefaultFixedFontSize(int);
        int defaultFixedFontSize() const { return m_defaultFixedFontSize; }

        // Unlike areImagesEnabled, this only suppresses the network load of
        // the image URL.  A cached image will still be rendered if requested.
        void setLoadsImagesAutomatically(bool);
        bool loadsImagesAutomatically() const { return m_loadsImagesAutomatically; }

        void setJavaScriptEnabled(bool);
        // Instead of calling isJavaScriptEnabled directly, please consider calling
        // ScriptController::canExecuteScripts, which takes things like the
        // HTML sandbox attribute into account.
        bool isJavaScriptEnabled() const { return m_isJavaScriptEnabled; }

        void setWebSecurityEnabled(bool);
        bool isWebSecurityEnabled() const { return m_isWebSecurityEnabled; }

        void setAllowUniversalAccessFromFileURLs(bool);
        bool allowUniversalAccessFromFileURLs() const { return m_allowUniversalAccessFromFileURLs; }

        void setAllowFileAccessFromFileURLs(bool);
        bool allowFileAccessFromFileURLs() const { return m_allowFileAccessFromFileURLs; }

        void setJavaScriptCanOpenWindowsAutomatically(bool);
        bool javaScriptCanOpenWindowsAutomatically() const { return m_javaScriptCanOpenWindowsAutomatically; }

        void setJavaScriptCanAccessClipboard(bool);
        bool javaScriptCanAccessClipboard() const { return m_javaScriptCanAccessClipboard; }

        void setSpatialNavigationEnabled(bool);
        bool isSpatialNavigationEnabled() const { return m_isSpatialNavigationEnabled; }

        void setJavaEnabled(bool);
        bool isJavaEnabled() const { return m_isJavaEnabled; }

        void setImagesEnabled(bool);
        bool areImagesEnabled() const { return m_areImagesEnabled; }

        void setMediaEnabled(bool);
        bool isMediaEnabled() const { return m_isMediaEnabled; }

        void setPluginsEnabled(bool);
        bool arePluginsEnabled() const { return m_arePluginsEnabled; }

        void setLocalStorageEnabled(bool);
        bool localStorageEnabled() const { return m_localStorageEnabled; }

#if ENABLE(DOM_STORAGE)        
        void setLocalStorageQuota(unsigned);
        unsigned localStorageQuota() const { return m_localStorageQuota; }

        // Allow clients concerned with memory consumption to set a quota on session storage
        // since the memory used won't be released until the Page is destroyed.
        // Default is noQuota.
        void setSessionStorageQuota(unsigned);
        unsigned sessionStorageQuota() const { return m_sessionStorageQuota; }
#endif

        // When this option is set, WebCore will avoid storing any record of browsing activity
        // that may persist on disk or remain displayed when the option is reset.
        // This option does not affect the storage of such information in RAM.
        // The following functions respect this setting:
        //  - HTML5/DOM Storage
        //  - Icon Database
        //  - Console Messages
        //  - Cache
        //  - Application Cache
        //  - Back/Forward Page History
        //  - Page Search Results
        void setPrivateBrowsingEnabled(bool);
        bool privateBrowsingEnabled() const { return m_privateBrowsingEnabled; }

        void setCaretBrowsingEnabled(bool);
        bool caretBrowsingEnabled() const { return m_caretBrowsingEnabled; }

        void setDefaultTextEncodingName(const String&);
        const String& defaultTextEncodingName() const { return m_defaultTextEncodingName; }
        
        void setUsesEncodingDetector(bool);
        bool usesEncodingDetector() const { return m_usesEncodingDetector; }

        void setDNSPrefetchingEnabled(bool);
        bool dnsPrefetchingEnabled() const { return m_dnsPrefetchingEnabled; }

        void setUserStyleSheetLocation(const KURL&);
        const KURL& userStyleSheetLocation() const { return m_userStyleSheetLocation; }

        void setShouldPrintBackgrounds(bool);
        bool shouldPrintBackgrounds() const { return m_shouldPrintBackgrounds; }

        void setTextAreasAreResizable(bool);
        bool textAreasAreResizable() const { return m_textAreasAreResizable; }

        void setEditableLinkBehavior(EditableLinkBehavior);
        EditableLinkBehavior editableLinkBehavior() const { return m_editableLinkBehavior; }

        void setTextDirectionSubmenuInclusionBehavior(TextDirectionSubmenuInclusionBehavior);
        TextDirectionSubmenuInclusionBehavior textDirectionSubmenuInclusionBehavior() const { return m_textDirectionSubmenuInclusionBehavior; }

#if ENABLE(DASHBOARD_SUPPORT)
        void setUsesDashboardBackwardCompatibilityMode(bool);
        bool usesDashboardBackwardCompatibilityMode() const { return m_usesDashboardBackwardCompatibilityMode; }
#endif
        
        void setNeedsAdobeFrameReloadingQuirk(bool);
        bool needsAcrobatFrameReloadingQuirk() const { return m_needsAdobeFrameReloadingQuirk; }

        void setNeedsKeyboardEventDisambiguationQuirks(bool);
        bool needsKeyboardEventDisambiguationQuirks() const { return m_needsKeyboardEventDisambiguationQuirks; }

        void setTreatsAnyTextCSSLinkAsStylesheet(bool);
        bool treatsAnyTextCSSLinkAsStylesheet() const { return m_treatsAnyTextCSSLinkAsStylesheet; }

        void setNeedsLeopardMailQuirks(bool);
        bool needsLeopardMailQuirks() const { return m_needsLeopardMailQuirks; }

        void setNeedsTigerMailQuirks(bool);
        bool needsTigerMailQuirks() const { return m_needsTigerMailQuirks; }

        void setDOMPasteAllowed(bool);
        bool isDOMPasteAllowed() const { return m_isDOMPasteAllowed; }
        
        void setUsesPageCache(bool);
        bool usesPageCache() const { return m_usesPageCache; }

        void setShrinksStandaloneImagesToFit(bool);
        bool shrinksStandaloneImagesToFit() const { return m_shrinksStandaloneImagesToFit; }

        void setShowsURLsInToolTips(bool);
        bool showsURLsInToolTips() const { return m_showsURLsInToolTips; }

        void setFTPDirectoryTemplatePath(const String&);
        const String& ftpDirectoryTemplatePath() const { return m_ftpDirectoryTemplatePath; }
        
        void setForceFTPDirectoryListings(bool);
        bool forceFTPDirectoryListings() const { return m_forceFTPDirectoryListings; }
        
        void setDeveloperExtrasEnabled(bool);
        bool developerExtrasEnabled() const { return m_developerExtrasEnabled; }

        void setFrameFlatteningEnabled(bool);
        bool frameFlatteningEnabled() const { return m_frameFlatteningEnabled; }

        void setAuthorAndUserStylesEnabled(bool);
        bool authorAndUserStylesEnabled() const { return m_authorAndUserStylesEnabled; }
        
        void setFontRenderingMode(FontRenderingMode mode);
        FontRenderingMode fontRenderingMode() const;

        void setNeedsSiteSpecificQuirks(bool);
        bool needsSiteSpecificQuirks() const { return m_needsSiteSpecificQuirks; }
        
        void setWebArchiveDebugModeEnabled(bool);
        bool webArchiveDebugModeEnabled() const { return m_webArchiveDebugModeEnabled; }

        void setLocalFileContentSniffingEnabled(bool);
        bool localFileContentSniffingEnabled() const { return m_localFileContentSniffingEnabled; }

        void setLocalStorageDatabasePath(const String&);
        const String& localStorageDatabasePath() const { return m_localStorageDatabasePath; }
        
        void setApplicationChromeMode(bool);
        bool inApplicationChromeMode() const { return m_inApplicationChromeMode; }

        void setOfflineWebApplicationCacheEnabled(bool);
        bool offlineWebApplicationCacheEnabled() const { return m_offlineWebApplicationCacheEnabled; }

        void setShouldPaintCustomScrollbars(bool);
        bool shouldPaintCustomScrollbars() const { return m_shouldPaintCustomScrollbars; }

        void setZoomMode(ZoomMode);
        ZoomMode zoomMode() const { return m_zoomMode; }
        
        void setEnforceCSSMIMETypeInStrictMode(bool);
        bool enforceCSSMIMETypeInStrictMode() { return m_enforceCSSMIMETypeInStrictMode; }

        void setMaximumDecodedImageSize(size_t size) { m_maximumDecodedImageSize = size; }
        size_t maximumDecodedImageSize() const { return m_maximumDecodedImageSize; }

#if USE(SAFARI_THEME)
        // Windows debugging pref (global) for switching between the Aqua look and a native windows look.
        static void setShouldPaintNativeControls(bool);
        static bool shouldPaintNativeControls() { return gShouldPaintNativeControls; }
#endif

        void setAllowScriptsToCloseWindows(bool);
        bool allowScriptsToCloseWindows() const { return m_allowScriptsToCloseWindows; }

        void setEditingBehaviorType(EditingBehaviorType behavior) { m_editingBehaviorType = behavior; }
        EditingBehaviorType editingBehaviorType() const { return static_cast<EditingBehaviorType>(m_editingBehaviorType); }

        void setDownloadableBinaryFontsEnabled(bool);
        bool downloadableBinaryFontsEnabled() const { return m_downloadableBinaryFontsEnabled; }

        void setXSSAuditorEnabled(bool);
        bool xssAuditorEnabled() const { return m_xssAuditorEnabled; }

        void setAcceleratedCompositingEnabled(bool);
        bool acceleratedCompositingEnabled() const { return m_acceleratedCompositingEnabled; }

        void setShowDebugBorders(bool);
        bool showDebugBorders() const { return m_showDebugBorders; }

        void setShowRepaintCounter(bool);
        bool showRepaintCounter() const { return m_showRepaintCounter; }

        void setExperimentalNotificationsEnabled(bool);
        bool experimentalNotificationsEnabled() const { return m_experimentalNotificationsEnabled; }

#if PLATFORM(WIN) || (OS(WINDOWS) && PLATFORM(WX))
        static void setShouldUseHighResolutionTimers(bool);
        static bool shouldUseHighResolutionTimers() { return gShouldUseHighResolutionTimers; }
#endif

        void setPluginAllowedRunTime(unsigned);
        unsigned pluginAllowedRunTime() const { return m_pluginAllowedRunTime; }

        void setWebGLEnabled(bool);
        bool webGLEnabled() const { return m_webGLEnabled; }

        void setAccelerated2dCanvasEnabled(bool);
        bool accelerated2dCanvasEnabled() const { return m_acceleratedCanvas2dEnabled; }

        void setLoadDeferringEnabled(bool);
        bool loadDeferringEnabled() const { return m_loadDeferringEnabled; }
        
        void setTiledBackingStoreEnabled(bool);
        bool tiledBackingStoreEnabled() const { return m_tiledBackingStoreEnabled; }

        void setHTML5ParserEnabled(bool flag) { m_html5ParserEnabled = flag; }
        bool html5ParserEnabled() const { return m_html5ParserEnabled; }

        // NOTE: This code will be deleted once the HTML5TreeBuilder is ready
        // to replace LegacyHTMLTreeBuilder.  Using the HTML5DocumentParser
        // with LegacyHTMLTreeBuilder will not be supported long-term.
        // Setting is only for testing the new tree builder in DumpRenderTree.
        void setHTML5TreeBuilderEnabled_DO_NOT_USE(bool flag) { m_html5TreeBuilderEnabled = flag; }
        bool html5TreeBuilderEnabled() const { return m_html5TreeBuilderEnabled; }

        void setPaginateDuringLayoutEnabled(bool flag) { m_paginateDuringLayoutEnabled = flag; }
        bool paginateDuringLayoutEnabled() const { return m_paginateDuringLayoutEnabled; }

        void setMemoryInfoEnabled(bool flag) { m_memoryInfoEnabled = flag; }
        bool memoryInfoEnabled() const { return m_memoryInfoEnabled; }

        // This setting will be removed when an HTML5 compatibility issue is
        // resolved and WebKit implementation of interactive validation is
        // completed. See http://webkit.org/b/40520, http://webkit.org/b/40747,
        // and http://webkit.org/b/40908
        void setInteractiveFormValidationEnabled(bool flag) { m_interactiveFormValidation = flag; }
        bool interactiveFormValidationEnabled() const { return m_interactiveFormValidation; }

    private:
        Page* m_page;
        
        String m_defaultTextEncodingName;
        String m_ftpDirectoryTemplatePath;
        String m_localStorageDatabasePath;
        KURL m_userStyleSheetLocation;
        AtomicString m_standardFontFamily;
        AtomicString m_fixedFontFamily;
        AtomicString m_serifFontFamily;
        AtomicString m_sansSerifFontFamily;
        AtomicString m_cursiveFontFamily;
        AtomicString m_fantasyFontFamily;
        EditableLinkBehavior m_editableLinkBehavior;
        TextDirectionSubmenuInclusionBehavior m_textDirectionSubmenuInclusionBehavior;
        int m_minimumFontSize;
        int m_minimumLogicalFontSize;
        int m_defaultFontSize;
        int m_defaultFixedFontSize;
        size_t m_maximumDecodedImageSize;
#if ENABLE(DOM_STORAGE)        
        unsigned m_localStorageQuota;
        unsigned m_sessionStorageQuota;
#endif
        unsigned m_pluginAllowedRunTime;
        ZoomMode m_zoomMode;
        bool m_isSpatialNavigationEnabled : 1;
        bool m_isJavaEnabled : 1;
        bool m_loadsImagesAutomatically : 1;
        bool m_privateBrowsingEnabled : 1;
        bool m_caretBrowsingEnabled : 1;
        bool m_areImagesEnabled : 1;
        bool m_isMediaEnabled : 1;
        bool m_arePluginsEnabled : 1;
        bool m_localStorageEnabled : 1;
        bool m_isJavaScriptEnabled : 1;
        bool m_isWebSecurityEnabled : 1;
        bool m_allowUniversalAccessFromFileURLs: 1;
        bool m_allowFileAccessFromFileURLs: 1;
        bool m_javaScriptCanOpenWindowsAutomatically : 1;
        bool m_javaScriptCanAccessClipboard : 1;
        bool m_shouldPrintBackgrounds : 1;
        bool m_textAreasAreResizable : 1;
#if ENABLE(DASHBOARD_SUPPORT)
        bool m_usesDashboardBackwardCompatibilityMode : 1;
#endif
        bool m_needsAdobeFrameReloadingQuirk : 1;
        bool m_needsKeyboardEventDisambiguationQuirks : 1;
        bool m_treatsAnyTextCSSLinkAsStylesheet : 1;
        bool m_needsLeopardMailQuirks : 1;
        bool m_needsTigerMailQuirks : 1;
        bool m_isDOMPasteAllowed : 1;
        bool m_shrinksStandaloneImagesToFit : 1;
        bool m_usesPageCache: 1;
        bool m_showsURLsInToolTips : 1;
        bool m_forceFTPDirectoryListings : 1;
        bool m_developerExtrasEnabled : 1;
        bool m_authorAndUserStylesEnabled : 1;
        bool m_needsSiteSpecificQuirks : 1;
        unsigned m_fontRenderingMode : 1;
        bool m_frameFlatteningEnabled : 1;
        bool m_webArchiveDebugModeEnabled : 1;
        bool m_localFileContentSniffingEnabled : 1;
        bool m_inApplicationChromeMode : 1;
        bool m_offlineWebApplicationCacheEnabled : 1;
        bool m_shouldPaintCustomScrollbars : 1;
        bool m_enforceCSSMIMETypeInStrictMode : 1;
        bool m_usesEncodingDetector : 1;
        bool m_allowScriptsToCloseWindows : 1;
        unsigned m_editingBehaviorType : 1;
        bool m_downloadableBinaryFontsEnabled : 1;
        bool m_xssAuditorEnabled : 1;
        bool m_acceleratedCompositingEnabled : 1;
        bool m_showDebugBorders : 1;
        bool m_showRepaintCounter : 1;
        bool m_experimentalNotificationsEnabled : 1;
        bool m_webGLEnabled : 1;
        bool m_acceleratedCanvas2dEnabled : 1;
        bool m_loadDeferringEnabled : 1;
        bool m_tiledBackingStoreEnabled : 1;
        bool m_html5ParserEnabled: 1;
        bool m_html5TreeBuilderEnabled: 1; // Will be deleted soon, do not use.
        bool m_paginateDuringLayoutEnabled : 1;
        bool m_dnsPrefetchingEnabled : 1;
        bool m_memoryInfoEnabled: 1;
        bool m_interactiveFormValidation: 1;
    
#if USE(SAFARI_THEME)
        static bool gShouldPaintNativeControls;
#endif
#if PLATFORM(WIN) || (OS(WINDOWS) && PLATFORM(WX))
        static bool gShouldUseHighResolutionTimers;
#endif
    };

} // namespace WebCore

#endif // Settings_h
