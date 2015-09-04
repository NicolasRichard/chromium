// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.contextualsearch;

import android.app.Activity;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.ViewTreeObserver.OnGlobalFocusChangeListener;

import org.chromium.base.ActivityState;
import org.chromium.base.ApplicationStatus;
import org.chromium.base.ApplicationStatus.ActivityStateListener;
import org.chromium.base.SysUtils;
import org.chromium.base.VisibleForTesting;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.ChromeActivity;
import org.chromium.chrome.browser.compositor.bottombar.contextualsearch.ContextualSearchPanel.PanelState;
import org.chromium.chrome.browser.compositor.bottombar.contextualsearch.ContextualSearchPanel.StateChangeReason;
import org.chromium.chrome.browser.compositor.bottombar.contextualsearch.ContextualSearchPanelDelegate;
import org.chromium.chrome.browser.contextualsearch.ContextualSearchSelectionController.SelectionType;
import org.chromium.chrome.browser.device.DeviceClassManager;
import org.chromium.chrome.browser.externalnav.ExternalNavigationHandler;
import org.chromium.chrome.browser.externalnav.ExternalNavigationHandler.OverrideUrlLoadingResult;
import org.chromium.chrome.browser.externalnav.ExternalNavigationParams;
import org.chromium.chrome.browser.gsa.GSAContextDisplaySelection;
import org.chromium.chrome.browser.infobar.InfoBarContainer;
import org.chromium.chrome.browser.preferences.PrefServiceBridge;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tab.TabRedirectHandler;
import org.chromium.chrome.browser.tabmodel.EmptyTabModelObserver;
import org.chromium.chrome.browser.tabmodel.TabModel;
import org.chromium.chrome.browser.tabmodel.TabModel.TabLaunchType;
import org.chromium.chrome.browser.tabmodel.TabModel.TabSelectionType;
import org.chromium.chrome.browser.tabmodel.TabModelObserver;
import org.chromium.chrome.browser.tabmodel.TabModelSelector;
import org.chromium.chrome.browser.tabmodel.TabModelSelectorTabObserver;
import org.chromium.chrome.browser.widget.findinpage.FindToolbarManager;
import org.chromium.chrome.browser.widget.findinpage.FindToolbarObserver;
import org.chromium.components.navigation_interception.NavigationParams;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.ContextualSearchClient;
import org.chromium.content_public.browser.GestureStateListener;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.content_public.browser.NavigationEntry;
import org.chromium.content_public.browser.WebContentsObserver;
import org.chromium.content_public.common.TopControlsState;
import org.chromium.ui.base.WindowAndroid;

import java.net.MalformedURLException;
import java.net.URL;

import javax.annotation.Nullable;


/**
 * Manager for the Contextual Search feature.
 * This class keeps track of the status of Contextual Search and coordinates the control
 * with the layout.
 */
public class ContextualSearchManager extends ContextualSearchObservable
        implements ContextualSearchManagementDelegate,
                ContextualSearchNetworkCommunicator, ContextualSearchSelectionHandler,
                ContextualSearchClient, ActivityStateListener {

    private static final String TAG = "ContextualSearch";

    private static final boolean ALWAYS_USE_RESOLVED_SEARCH_TERM = true;
    private static final boolean NEVER_USE_RESOLVED_SEARCH_TERM = false;

    private static final String INTENT_URL_PREFIX = "intent:";

    // The animation duration of a URL being promoted to a tab when triggered by an
    // intercept navigation. This is faster than the standard tab promotion animation
    // so that it completes before the navigation.
    private static final long INTERCEPT_NAVIGATION_PROMOTION_ANIMATION_DURATION_MS = 40;

    // We blacklist this URL because malformed URLs may bring up this page.
    private static final String BLACKLISTED_URL = "about:blank";

    private final ContextualSearchSelectionController mSelectionController;
    private final ChromeActivity mActivity;
    private ViewGroup mParentView;
    private final ViewTreeObserver.OnGlobalFocusChangeListener mOnFocusChangeListener;

    private final WindowAndroid mWindowAndroid;
    private WebContentsObserver mSearchWebContentsObserver;
    private ContextualSearchContentViewDelegate mSearchContentViewDelegate;
    private final ContextualSearchTabPromotionDelegate mTabPromotionDelegate;
    private TabModelSelectorTabObserver mTabModelSelectorTabObserver;
    private TabModelObserver mTabModelObserver;
    private FindToolbarManager mFindToolbarManager;
    private FindToolbarObserver mFindToolbarObserver;
    private boolean mIsSearchContentViewShowing;
    private boolean mDidLoadResolvedSearchRequest;
    private long mLoadedSearchUrlTimeMs;
    // TODO(donnd): consider changing this member's name to indicate "opened" instead of "seen".
    private boolean mWereSearchResultsSeen;
    private boolean mWereInfoBarsHidden;
    private boolean mDidLoadAnyUrl;
    private boolean mDidPromoteSearchNavigation;
    private boolean mDidBasePageLoadJustStart;
    private boolean mWasActivatedByTap;
    private boolean mIsInitialized;

    private boolean mIsShowingPromo;
    private boolean mDidLogPromoOutcome;

    /**
     * Whether contextual search manager is currently promoting a tab. We should be ignoring hide
     * requests when mIsPromotingTab is set to true.
     */
    private boolean mIsPromotingToTab;

    private ContextualSearchNetworkCommunicator mNetworkCommunicator;
    private ContextualSearchPanelDelegate mSearchPanelDelegate;

    // TODO(pedrosimonetti): also store selected text, surroundings, url, bounding rect of selected
    // text, and make sure that all states are cleared when starting a new contextual search to
    // avoid having the values in a funky state.
    private ContextualSearchRequest mSearchRequest;

    // The native manager associated with this object.
    private long mNativeContextualSearchManagerPtr;

    private TabRedirectHandler mTabRedirectHandler;

    /**
     * The delegate that is notified when the Search Panel ContentViewCore is ready to be rendered.
     */
    public interface ContextualSearchContentViewDelegate {
        /**
         * Sets the {@code ContentViewCore} associated to the Contextual Search Panel.
         * @param contentViewCore Reference to the ContentViewCore.
         */
        void setContextualSearchContentViewCore(ContentViewCore contentViewCore);

        /**
         * Releases the {@code ContentViewCore} associated to the Contextual Search Panel.
         */
        void releaseContextualSearchContentViewCore();
    }

    /**
     * The delegate that is responsible for promoting a {@link ContentViewCore} to a {@link Tab}
     * when necessary.
     */
    public interface ContextualSearchTabPromotionDelegate {
        /**
         * Called when {@code searchContentViewCore} should be promoted to a {@link Tab}.
         * @param searchUrl The Search URL to be promoted.
         */
        void createContextualSearchTab(String searchUrl);
    }

    /**
     * Constructs the manager for the given activity, and will attach views to the given parent.
     * @param activity             The {@code ChromeActivity} in use.
     * @param windowAndroid        The {@code WindowAndroid} associated with Chrome.
     * @param tabPromotionDelegate The {@link ContextualSearchTabPromotionDelegate} that is
     *                             responsible for building tabs from contextual search
     *                             {@link ContentViewCore}s.
     */
    public ContextualSearchManager(ChromeActivity activity, WindowAndroid windowAndroid,
            ContextualSearchTabPromotionDelegate tabPromotionDelegate) {
        super(activity);
        mActivity = activity;
        mWindowAndroid = windowAndroid;
        mTabPromotionDelegate = tabPromotionDelegate;

        mSelectionController = new ContextualSearchSelectionController(activity, this);

        final View controlContainer = mActivity.findViewById(R.id.control_container);
        mOnFocusChangeListener = new OnGlobalFocusChangeListener() {
            @Override
            public void onGlobalFocusChanged(View oldFocus, View newFocus) {
                if (controlContainer != null && controlContainer.hasFocus()) {
                    hideContextualSearch(StateChangeReason.UNKNOWN);
                }
            }
        };

        mTabModelObserver = new EmptyTabModelObserver() {
            @Override
            public void didSelectTab(Tab tab, TabSelectionType type, int lastId) {
                if (!mIsPromotingToTab && tab.getId() != lastId) {
                    hideContextualSearch(StateChangeReason.UNKNOWN);
                }
            }

            @Override
            public void didAddTab(Tab tab, TabLaunchType type) {
                // If we're in the process of promoting this tab, just return and don't mess with
                // this state.
                if (tab.getContentViewCore() == mSearchPanelDelegate.getContentViewCore()) return;
                hideContextualSearch(StateChangeReason.UNKNOWN);
            }
        };
    }

    /**
     * Initializes this manager.  Must be called before {@link #getContextualSearchControl()}.
     * @param parentView The parent view to attach Contextual Search UX to.
     */
    public void initialize(ViewGroup parentView) {
        mParentView = parentView;
        mParentView.getViewTreeObserver().addOnGlobalFocusChangeListener(mOnFocusChangeListener);
        mNativeContextualSearchManagerPtr = nativeInit();
        listenForHideNotifications();
        mTabRedirectHandler = new TabRedirectHandler(mActivity);

        mIsShowingPromo = false;
        mDidLogPromoOutcome = false;
        mDidLoadResolvedSearchRequest = false;
        mWereSearchResultsSeen = false;
        mNetworkCommunicator = this;
        ApplicationStatus.registerStateListenerForActivity(this, mActivity);
        mIsInitialized = true;
    }

    /**
     * Sets the {@link FindToolbarManager} and attaches an observer that dismisses the Contextual
     * Search panel when the find toolbar is shown.
     *
     * @param findToolbarManager The {@link FindToolbarManager} for the current activity.
     */
    public void setFindToolbarManager(FindToolbarManager findToolbarManager) {
        if (mFindToolbarManager != null) {
            mFindToolbarManager.removeObserver(mFindToolbarObserver);
        }

        mFindToolbarManager = findToolbarManager;

        if (mFindToolbarObserver == null) {
            mFindToolbarObserver = new FindToolbarObserver() {
                @Override
                public void onFindToolbarShown() {
                    hideContextualSearch(StateChangeReason.UNKNOWN);
                }
            };
        }
        mFindToolbarManager.addObserver(mFindToolbarObserver);
    }

    /**
     * Destroys the native Contextual Search Manager.
     * Call this method before orphaning this object to allow it to be garbage collected.
     */
    public void destroy() {
        if (!mIsInitialized) return;

        hideContextualSearch(StateChangeReason.UNKNOWN);
        mParentView.getViewTreeObserver().removeOnGlobalFocusChangeListener(mOnFocusChangeListener);
        nativeDestroy(mNativeContextualSearchManagerPtr);
        stopListeningForHideNotifications();
        mTabRedirectHandler.clear();
        ApplicationStatus.unregisterActivityStateListener(this);
        if (mFindToolbarManager != null) {
            mFindToolbarManager.removeObserver(mFindToolbarObserver);
            mFindToolbarManager = null;
            mFindToolbarObserver = null;
        }
    }

    @Override
    public void setContextualSearchPanelDelegate(ContextualSearchPanelDelegate delegate) {
        mSearchPanelDelegate = delegate;

        mSearchPanelDelegate.setChromeActivity(mActivity);
    }

    @Override
    public boolean isCustomTab() {
        return mActivity.isCustomTab();
    }

    /**
     * @return The {@link ContextualSearchPanelDelegate}, for testing purposes only.
     */
    @VisibleForTesting
    public ContextualSearchPanelDelegate getContextualSearchPanelDelegate() {
        return mSearchPanelDelegate;
    }

    /**
     * Sets the selection controller for testing purposes.
     */
    @VisibleForTesting
    ContextualSearchSelectionController getSelectionController() {
        return mSelectionController;
    }

    @VisibleForTesting
    boolean isSearchPanelShowing() {
        return mSearchPanelDelegate.isShowing();
    }

    /**
     * @return Whether the Search Panel is opened. That is, whether it is EXPANDED or MAXIMIZED.
     */
    public boolean isSearchPanelOpened() {
        PanelState state = mSearchPanelDelegate.getPanelState();
        return state == PanelState.EXPANDED || state == PanelState.MAXIMIZED;
    }

    /**
     * @return The Base Page's {@link ContentViewCore}.
     */
    @Nullable private ContentViewCore getBaseContentView() {
        return mSelectionController.getBaseContentView();
    }

    @Override
    public boolean isShowingSearchPanel() {
        return mSearchPanelDelegate.isShowing();
    }

    @Override
    public void setPreferenceState(boolean enabled) {
        PrefServiceBridge.getInstance().setContextualSearchState(enabled);
    }

    @Override
    public boolean isPromoAvailable() {
        return mPolicy.isPromoAvailable();
    }

    @Override
    public int getControlContainerHeightResource() {
        return mActivity.getControlContainerHeightResource();
    }

    /**
     * Hides the Contextual Search UX.
     * @param reason The {@link StateChangeReason} for hiding Contextual Search.
     */
    public void hideContextualSearch(StateChangeReason reason) {
        if (mSearchPanelDelegate == null) return;

        if (mSearchPanelDelegate.isShowing()) {
            mSearchPanelDelegate.closePanel(reason, false);
        }
    }

    @Override
    public void onCloseContextualSearch(StateChangeReason reason) {
        // If the user explicitly closes the panel after establishing a selection with long press,
        // it should not reappear until a new selection is made. This prevents the panel from
        // reappearing when a long press selection is modified after the user has taken action to
        // get rid of the panel. See crbug.com/489461.
        if (shouldPreventHandlingCurrentSelectionModification(reason)) {
            mSelectionController.preventHandlingCurrentSelectionModification();
        }

        if (mSearchPanelDelegate == null) return;

        // NOTE(pedrosimonetti): hideContextualSearch() will also be called after swiping the
        // Panel down in order to dismiss it. In this case, hideContextualSearch() will be called
        // after completing the hide animation, and at that moment the Panel will not be showing
        // anymore. Therefore, we need to always clear selection, regardless of when the Panel
        // was still visible, in order to make sure the selection will be cleared appropriately.
        if (mSelectionController.getSelectionType() == SelectionType.TAP) {
            mSelectionController.clearSelection();
        }

        // Show the infobar container if it was visible before Contextual Search was shown.
        if (mWereInfoBarsHidden) {
            mWereInfoBarsHidden = false;
            InfoBarContainer container = getInfoBarContainer();
            if (container != null) {
                container.setVisibility(View.VISIBLE);
                container.setDoStayInvisible(false);
            }
        }

        if (!mWereSearchResultsSeen && mLoadedSearchUrlTimeMs != 0L) {
            removeLastSearchVisit();
        }

        // Clear the timestamp. This is to avoid future calls to hideContextualSearch clearing
        // the current URL.
        mLoadedSearchUrlTimeMs = 0L;
        mWereSearchResultsSeen = false;

        mSearchRequest = null;

        if (mIsShowingPromo && !mDidLogPromoOutcome) {
            logPromoOutcome();
        }

        mIsShowingPromo = false;
        mSearchPanelDelegate.setIsPromoActive(false);
        notifyHideContextualSearch();
    }

    /**
     * Returns true if the StateChangeReason corresponds to an explicit action used to close
     * the Contextual Search panel.
     * @param reason The reason the panel is closing.
     */
    private boolean shouldPreventHandlingCurrentSelectionModification(StateChangeReason reason) {
        return mSelectionController.getSelectionType() == SelectionType.LONG_PRESS
                && (reason == StateChangeReason.BACK_PRESS
                || reason == StateChangeReason.BASE_PAGE_SCROLL
                || reason == StateChangeReason.SWIPE
                || reason == StateChangeReason.FLING);
    }

    /**
     * Called when the system back button is pressed. Will hide the layout.
     */
    public boolean onBackPressed() {
        if (!mIsInitialized || !mSearchPanelDelegate.isShowing()) return false;
        hideContextualSearch(StateChangeReason.BACK_PRESS);
        return true;
    }

    /**
     * Called when the orientation of the device changes.
     */
    public void onOrientationChange() {
        if (!mIsInitialized) return;
        hideContextualSearch(StateChangeReason.UNKNOWN);
    }

    /**
     * Sets the {@link ContextualSearchNetworkCommunicator} to use for server requests.
     * @param networkCommunicator The communicator for all future requests.
     */
    @VisibleForTesting
    public void setNetworkCommunicator(ContextualSearchNetworkCommunicator networkCommunicator) {
        mNetworkCommunicator = networkCommunicator;
    }

    /**
     * Shows the Contextual Search UX.
     * Calls back into onGetContextualSearchQueryResponse.
     * @param stateChangeReason The reason explaining the change of state.
     */
    private void showContextualSearch(StateChangeReason stateChangeReason) {
        if (mFindToolbarManager != null) {
            mFindToolbarManager.hideToolbar(false);
        }

        // Dismiss the undo SnackBar if present by committing all tab closures.
        mActivity.getTabModelSelector().commitAllTabClosures();

        if (!mSearchPanelDelegate.isShowing()) {
            // If visible, hide the infobar container before showing the Contextual Search panel.
            InfoBarContainer container = getInfoBarContainer();
            if (container != null && container.getVisibility() == View.VISIBLE) {
                mWereInfoBarsHidden = true;
                container.setVisibility(View.INVISIBLE);
                container.setDoStayInvisible(true);
            }
        }

        // If the user is jumping from one unseen search to another search, remove the last search
        // from history.
        PanelState state = mSearchPanelDelegate.getPanelState();
        if (!mWereSearchResultsSeen && mLoadedSearchUrlTimeMs != 0L
                && state != PanelState.UNDEFINED && state != PanelState.CLOSED) {
            removeLastSearchVisit();
        }

        boolean isTap = mSelectionController.getSelectionType() == SelectionType.TAP;
        boolean didRequestSurroundings = false;
        if (isTap && mPolicy.shouldPreviousTapResolve(
                mNetworkCommunicator.getBasePageUrl())) {
            mNetworkCommunicator.startSearchTermResolutionRequest(
                    mSelectionController.getSelectedText());
            didRequestSurroundings = true;
        } else {
            boolean shouldPrefetch = mPolicy.shouldPrefetchSearchResult(isTap);
            mSearchRequest = new ContextualSearchRequest(mSelectionController.getSelectedText(),
                    null, shouldPrefetch);
            mDidLoadResolvedSearchRequest = false;
            mSearchPanelDelegate.displaySearchTerm(mSelectionController.getSelectedText());
            if (shouldPrefetch) loadSearchUrl();
        }

        if (!didRequestSurroundings) {
            // Gather surrounding text for Icing integration, which will make the selection and
            // a shorter version of the surroundings available for Conversational Search.
            // Although the surroundings are extracted, they will not be sent to the server as
            // part of search term resolution, just sent to Icing which keeps them local until
            // the user activates a Voice Search.
            nativeGatherSurroundingText(mNativeContextualSearchManagerPtr,
                    mSelectionController.getSelectedText(), NEVER_USE_RESOLVED_SEARCH_TERM,
                    getBaseContentView(), mPolicy.maySendBasePageUrl());
        }

        mWereSearchResultsSeen = false;

        // TODO(donnd): although we are showing the bar here, we have not yet set the text!
        // Refactor to show the bar and set the text at the same time!
        // TODO(donnd): If there was a previously ongoing contextual search, we should ensure
        // it's registered as closed.
        mSearchPanelDelegate.peekPanel(stateChangeReason);

        // Note: now that the contextual search has properly started, set the promo involvement.
        if (mPolicy.isPromoAvailable()) {
            mIsShowingPromo = true;
            mDidLogPromoOutcome = false;
            mSearchPanelDelegate.setIsPromoActive(true);
            mSearchPanelDelegate.setDidSearchInvolvePromo();
        }

        assert mSelectionController.getSelectionType() != SelectionType.UNDETERMINED;
        mWasActivatedByTap = mSelectionController.getSelectionType() == SelectionType.TAP;
    }

    @Override
    public void startSearchTermResolutionRequest(String selection) {
        ContentViewCore baseContentView = getBaseContentView();
        if (baseContentView != null) {
            nativeStartSearchTermResolutionRequest(mNativeContextualSearchManagerPtr, selection,
                    ALWAYS_USE_RESOLVED_SEARCH_TERM, getBaseContentView(),
                    mPolicy.maySendBasePageUrl());
        }
    }

    @Override
    @Nullable public URL getBasePageUrl() {
        ContentViewCore baseContentViewCore = getBaseContentView();
        if (baseContentViewCore == null) return null;

        try {
            return new URL(baseContentViewCore.getWebContents().getUrl());
        } catch (MalformedURLException e) {
            return null;
        }
    }

    @Override
    public void updateTopControlsState(int current, boolean animate) {
        Tab currentTab = mActivity.getActivityTab();
        if (currentTab != null) {
            currentTab.updateTopControlsState(current, animate);
        }
    }

    /**
     * Accessor for the {@code InfoBarContainer} currently attached to the {@code Tab}.
     */
    private InfoBarContainer getInfoBarContainer() {
        Tab tab = mActivity.getActivityTab();
        return tab == null ? null : tab.getInfoBarContainer();
    }

    /**
     * Listens for notifications that should hide the Contextual Search bar.
     */
    private void listenForHideNotifications() {
        TabModelSelector selector = mActivity.getTabModelSelector();

        mTabModelSelectorTabObserver = new TabModelSelectorTabObserver(selector) {
            @Override
            public void onPageLoadStarted(Tab tab, String url) {
                hideContextualSearch(StateChangeReason.UNKNOWN);
                mDidBasePageLoadJustStart = true;
            }

            @Override
            public void onCrash(Tab tab, boolean sadTabShown) {
                if (sadTabShown) {
                    // Hide contextual search if the foreground tab crashed
                    hideContextualSearch(StateChangeReason.UNKNOWN);
                }
            }

            @Override
            public void onClosingStateChanged(Tab tab, boolean closing) {
                if (closing) hideContextualSearch(StateChangeReason.UNKNOWN);
            }
        };

        for (TabModel tabModel : selector.getModels()) {
            tabModel.addObserver(mTabModelObserver);
        }
    }

    /**
     * Stops listening for notifications that should hide the Contextual Search bar.
     */
    private void stopListeningForHideNotifications() {
        if (mTabModelSelectorTabObserver != null) mTabModelSelectorTabObserver.destroy();

        TabModelSelector selector = mActivity.getTabModelSelector();
        if (selector != null) {
            for (TabModel tabModel : selector.getModels()) {
                tabModel.removeObserver(mTabModelObserver);
            }
        }
    }

    @Override
    public void onActivityStateChange(Activity activity, int newState) {
        if (newState == ActivityState.RESUMED || newState == ActivityState.STOPPED
                || newState == ActivityState.DESTROYED) {
            hideContextualSearch(StateChangeReason.UNKNOWN);
        } else if (newState == ActivityState.PAUSED) {
            mPolicy.logCurrentState(getBaseContentView());
        }
    }

    /**
     * Clears our private member referencing the native manager.
     */
    @CalledByNative
    public void clearNativeManager() {
        assert mNativeContextualSearchManagerPtr != 0;
        mNativeContextualSearchManagerPtr = 0;
    }

    /**
     * Sets our private member referencing the native manager.
     * @param nativeManager The pointer to the native Contextual Search manager.
     */
    @CalledByNative
    public void setNativeManager(long nativeManager) {
        assert mNativeContextualSearchManagerPtr == 0;
        mNativeContextualSearchManagerPtr = nativeManager;
    }

    /**
     * Called when surrounding text is available.
     * @param beforeText to be shown before the selected word.
     * @param afterText to be shown after the selected word.
     */
    @CalledByNative
    private void onSurroundingTextAvailable(final String beforeText, final String afterText) {
        if (mSearchPanelDelegate.isShowing()) {
            mSearchPanelDelegate.displaySearchContext(
                    mSelectionController.getSelectedText(), beforeText, afterText);
        }
    }

    /**
     * Called by native code when a selection is available to share with Icing (for Conversational
     * Search).
     */
    @CalledByNative
    private void onIcingSelectionAvailable(
            final String encoding, final String surroundingText, int startOffset, int endOffset) {
        GSAContextDisplaySelection selection =
                new GSAContextDisplaySelection(encoding, surroundingText, startOffset, endOffset);
        notifyShowContextualSearch(selection, mNetworkCommunicator.getBasePageUrl());
    }

    /**
     * Called in response to the
     * {@link ContextualSearchManager#nativeStartSearchTermResolutionRequest} method.
     * @param isNetworkUnavailable Indicates if the network is unavailable, in which case all other
     *        parameters should be ignored.
     * @param responseCode The HTTP response code.  If the code is not OK, the query
     *        should be ignored.
     * @param searchTerm The term to use in our subsequent search.
     * @param displayText The text to display in our UX.
     * @param alternateTerm The alternate term to display on the results page.
     * @param selectionStartAdjust A positive number of characters that the start of the existing
     *        selection should be expanded by.
     * @param selectionEndAdjust A positive number of characters that the end of the existing
     *        selection should be expanded by.
     */
    @CalledByNative
    public void onSearchTermResolutionResponse(boolean isNetworkUnavailable, int responseCode,
            final String searchTerm, final String displayText, final String alternateTerm,
            boolean doPreventPreload, int selectionStartAdjust, int selectionEndAdjust) {
        mNetworkCommunicator.handleSearchTermResolutionResponse(isNetworkUnavailable, responseCode,
                searchTerm, displayText, alternateTerm, doPreventPreload, selectionStartAdjust,
                selectionEndAdjust);
    }

    @Override
    public void handleSearchTermResolutionResponse(boolean isNetworkUnavailable, int responseCode,
            String searchTerm, String displayText, String alternateTerm, boolean doPreventPreload,
            int selectionStartAdjust, int selectionEndAdjust) {
        if (!mSearchPanelDelegate.isShowing()) return;

        // Show an appropriate message for what to search for.
        String message;
        boolean doLiteralSearch = false;
        if (isNetworkUnavailable) {
            message = mActivity.getResources().getString(
                    R.string.contextual_search_network_unavailable);
        } else if (!isHttpFailureCode(responseCode)) {
            message = displayText;
        } else if (!mPolicy.shouldShowErrorCodeInBar()) {
            message = mSelectionController.getSelectedText();
            doLiteralSearch = true;
        } else {
            message = mActivity.getResources().getString(
                    R.string.contextual_search_error, responseCode);
            doLiteralSearch = true;
        }
        mSearchPanelDelegate.onSearchTermResolutionResponse(message);

        // If there was an error, fall back onto a literal search for the selection.
        // Since we're showing the panel, there must be a selection.
        if (doLiteralSearch) {
            searchTerm = mSelectionController.getSelectedText();
            alternateTerm = null;
            doPreventPreload = true;
        }
        if (!searchTerm.isEmpty()) {
            // TODO(donnd): Instead of preloading, we should prefetch (ie the URL should not
            // appear in the user's history until the user views it).  See crbug.com/406446.
            boolean shouldPreload = !doPreventPreload && mPolicy.shouldPrefetchSearchResult(true);
            mSearchRequest = new ContextualSearchRequest(searchTerm, alternateTerm, shouldPreload);
            mDidLoadResolvedSearchRequest = false;
            if (mSearchPanelDelegate.isContentViewShowing()) {
                mSearchRequest.setNormalPriority();
            }
            if (mSearchPanelDelegate.isContentViewShowing() || shouldPreload) {
                loadSearchUrl();
            }
            mPolicy.logSearchTermResolutionDetails(searchTerm,
                    mNetworkCommunicator.getBasePageUrl());
        }

        if (selectionStartAdjust != 0 || selectionEndAdjust != 0) {
            mSelectionController.adjustSelection(selectionStartAdjust, selectionEndAdjust);
        }
    }

    /**
     * Loads a Search Request in the Contextual Search's Content View.
     */
    private void loadSearchUrl() {
        mLoadedSearchUrlTimeMs = System.currentTimeMillis();
        mSearchPanelDelegate.loadUrlInPanel(mSearchRequest.getSearchUrl());
        mDidLoadResolvedSearchRequest = true;

        // TODO(pedrosimonetti): If the user taps on a word and quickly after that taps on the
        // peeking Search Bar, the Search Content View will not be displayed. It seems that
        // calling ContentViewCore.onShow() while it's being created has no effect. Need
        // to coordinate with Chrome-Android folks to come up with a proper fix for this.
        // For now, we force the ContentView to be displayed by calling onShow() again
        // when a URL is being loaded. See: crbug.com/398206
        if (mSearchPanelDelegate.isContentViewShowing()
                && mSearchPanelDelegate.getContentViewCore() != null) {
            mSearchPanelDelegate.getContentViewCore().onShow();
        }
    }

    /**
     * @return Whether a Tap gesture is currently supported.
     */
    private boolean isTapSupported() {
        // Base page just started navigating away, so taps should be ignored.
        if (mDidBasePageLoadJustStart) return false;

        return mPolicy.isTapSupported();
    }

    // --------------------------------------------------------------------------------------------
    // Search Content View
    // --------------------------------------------------------------------------------------------

    /**
     * Gets the {@code ContentViewCore} associated with Contextual Search Panel.
     * @return Contextual Search Panel's {@code ContentViewCore}.
     */
    @Override
    public ContentViewCore getSearchContentViewCore() {
        return mSearchPanelDelegate.getContentViewCore();
    }

    /**
     * Sets the {@code ContextualSearchContentViewDelegate} associated with the Content View.
     * @param delegate
     */
    public void setSearchContentViewDelegate(ContextualSearchContentViewDelegate delegate) {
        mSearchContentViewDelegate = delegate;
    }

    /**
     * Removes the last resolved search URL from the Chrome history.
     */
    private void removeLastSearchVisit() {
        if (mSearchRequest != null) {
            mSearchPanelDelegate.removeLastHistoryEntry(mSearchRequest.getSearchUrl(),
                    mLoadedSearchUrlTimeMs);
        }
    }

    /**
     * Called when the Search content view navigates to a contextual search request URL.
     * This navigation could be for a prefetch when the panel is still closed, or
     * a load of a user-visible search result.
     * @param isFailure Whether the navigation failed.
     */
    @Override
    public void onContextualSearchRequestNavigation(boolean isFailure) {
        if (mSearchRequest == null) return;

        if (mSearchRequest.isUsingLowPriority()) {
            ContextualSearchUma.logLowPrioritySearchRequestOutcome(isFailure);
        } else {
            ContextualSearchUma.logNormalPrioritySearchRequestOutcome(isFailure);
            if (mSearchRequest.getHasFailed()) {
                ContextualSearchUma.logFallbackSearchRequestOutcome(isFailure);
            }
        }

        if (isFailure && mSearchRequest.isUsingLowPriority()) {
            // We're navigating to an error page, so we want to stop and retry.
            // Stop loading the page that displays the error to the user.
            if (mSearchPanelDelegate.getContentViewCore() != null) {
                // When running tests the Content View might not exist.
                mSearchPanelDelegate.getContentViewCore().getWebContents().stop();
            }
            mSearchRequest.setHasFailed();
            mSearchRequest.setNormalPriority();
            // If the content view is showing, load at normal priority now.
            if (mSearchPanelDelegate.isContentViewShowing()) {
                loadSearchUrl();
            } else {
                mDidLoadResolvedSearchRequest = false;
            }
        }
    }

    @Override
    public void logPromoOutcome() {
        ContextualSearchUma.logPromoOutcome(mWasActivatedByTap);
        mDidLogPromoOutcome = true;
    }

    /**
     * Called when the Search Content view has finished loading to record how long it takes the SERP
     * to load after opening the panel.
     */
    @Override
    public void onSearchResultsLoaded() {
        if (mSearchRequest == null) return;

        mSearchPanelDelegate.onSearchResultsLoaded(mSearchRequest.wasPrefetch());

        // Any time we place a page in a ContentViewCore, clear history if needed.
        // This prevents error URLs from appearing in the Tab's history stack.
        // Also please note that clearHistory() will not
        // clear the current entry (search results page in this case),
        // and it will not work properly if there are pending navigations.
        // That's why we need to clear the history here, after the navigation
        // is completed.
        boolean shouldClearHistory = mSearchRequest.getHasFailed();
        if (shouldClearHistory && mSearchPanelDelegate.getContentViewCore() != null) {
            mSearchPanelDelegate.getContentViewCore().getWebContents().getNavigationController()
                    .clearHistory();
        }
    }

    @Override
    public void handleDidNavigateMainFrame(String url, int httpResultCode) {
        if (shouldPromoteSearchNavigation()) {
            onExternalNavigation(url);
        } else {
            // Could be just prefetching, check if that failed.
            boolean isFailure = isHttpFailureCode(httpResultCode);
            onContextualSearchRequestNavigation(isFailure);
        }
    }

    @Override
    public void onStartedLoading() {
        mDidPromoteSearchNavigation = false;
    }


    @Override
    public boolean shouldInterceptNavigation(ExternalNavigationHandler externalNavHandler,
            NavigationParams navigationParams) {
        ExternalNavigationParams params = new ExternalNavigationParams.Builder(
                navigationParams.url, false, navigationParams.referrer,
                navigationParams.pageTransitionType, navigationParams.isRedirect)
                .setApplicationMustBeInForeground(true)
                .setRedirectHandler(mTabRedirectHandler)
                .setIsMainFrame(navigationParams.isMainFrame)
                .build();
        if (externalNavHandler.shouldOverrideUrlLoading(params)
                != OverrideUrlLoadingResult.NO_OVERRIDE) {
            mSearchPanelDelegate.maximizePanelThenPromoteToTab(StateChangeReason.TAB_PROMOTION,
                    INTERCEPT_NAVIGATION_PROMOTION_ANIMATION_DURATION_MS);
            return false;
        }
        if (navigationParams.isExternalProtocol) {
            return false;
        }
        return true;
    }

    /**
     * @return Whether the given HTTP result code represents a failure or not.
     */
    private boolean isHttpFailureCode(int httpResultCode) {
        return httpResultCode <= 0 || httpResultCode >= 400;
    }

    /**
     * @return whether a navigation in the search content view should promote to a separate tab.
     */
    private boolean shouldPromoteSearchNavigation() {
        // A navigation can be due to us loading a URL, or a touch in the search content view.
        // Require a touch, but no recent loading, in order to promote to a separate tab.
        // Note that tapping the opt-in button requires checking for recent loading.
        return mSearchPanelDelegate.didTouchSearchContentView()
                && !mSearchPanelDelegate.didLoadAnyUrl();
    }

    /**
     * Called to check if an external navigation is being done and take the appropriate action:
     * Auto-promotes the panel into a separate tab if that's not already being done.
     * @param url The URL we are navigating to.
     */
    @Override
    public void onExternalNavigation(String url) {
        mSearchPanelDelegate.updateTopControlState();

        if (!mDidPromoteSearchNavigation
                && !BLACKLISTED_URL.equals(url)
                && !url.startsWith(INTENT_URL_PREFIX)
                && shouldPromoteSearchNavigation()) {
            // Do not promote to a regular tab if we're loading our Resolved Search
            // URL, otherwise we'll promote it when prefetching the Serp.
            // Don't promote URLs when they are navigating to an intent - this is
            // handled by the InterceptNavigationDelegate which uses a faster
            // maximizing animation.
            mDidPromoteSearchNavigation = true;
            mSearchPanelDelegate.maximizePanelThenPromoteToTab(StateChangeReason.SERP_NAVIGATION);
        }
    }

    @Override
    public void onContentViewCreated(ContentViewCore contentView) {
        // TODO(mdjones): Move SearchContentViewDelegate ownership to panel.
        mSearchContentViewDelegate.setContextualSearchContentViewCore(contentView);
    }

    @Override
    public void onContentViewDestroyed() {
        if (mSearchContentViewDelegate != null) {
            mSearchContentViewDelegate.releaseContextualSearchContentViewCore();
        }
    }

    @Override
    public void openResolvedSearchUrlInNewTab() {
        if (mSearchRequest != null && mSearchRequest.getSearchUrl() != null) {
            openUrlInNewTab(mSearchRequest.getSearchUrl());
        }
    }

    /**
     * Convenience method for opening a specific |url| in a new Tab.
     */
    private void openUrlInNewTab(String url) {
        TabModelSelector tabModelSelector = mActivity.getTabModelSelector();
        tabModelSelector.openNewTab(
                new LoadUrlParams(url),
                TabLaunchType.FROM_MENU_OR_OVERVIEW,
                tabModelSelector.getCurrentTab(),
                tabModelSelector.isIncognitoSelected());
    }

    @Override
    public boolean isRunningInCompatibilityMode() {
        return DeviceClassManager.isAccessibilityModeEnabled(mActivity)
                || SysUtils.isLowEndDevice();
    }

    @Override
    public void promoteToTab() {
        // TODO(pedrosimonetti): Consider removing this member.
        mIsPromotingToTab = true;

        // If the request object is null that means that a Contextual Search has just started
        // and the Search Term Resolution response hasn't arrived yet. In this case, promoting
        // the Panel to a Tab will result in creating a new tab with URL about:blank. To prevent
        // this problem, we are ignoring tap gestures in the Search Bar if we don't know what
        // to search for.
        if (mSearchRequest != null
                && mSearchPanelDelegate.getContentViewCore() != null
                && mSearchPanelDelegate.getContentViewCore().getWebContents() != null) {
            String url = getContentViewUrl(mSearchPanelDelegate.getContentViewCore());

            // If it's a search URL, formats it so the SearchBox becomes visible.
            if (mSearchRequest.isContextualSearchUrl(url)) {
                url = mSearchRequest.getSearchUrlForPromotion();
            }

            if (url != null) {
                mTabPromotionDelegate.createContextualSearchTab(url);
                mSearchPanelDelegate.closePanel(StateChangeReason.TAB_PROMOTION, false);
            }
        }
        mIsPromotingToTab = false;
    }

    /**
     * Gets the current loaded URL in a ContentViewCore.
     *
     * @param searchContentViewCore The given ContentViewCore.
     * @return The current loaded URL.
     */
    private String getContentViewUrl(ContentViewCore searchContentViewCore) {
        // First, check the pending navigation entry, because there might be an navigation
        // not yet committed being processed. Otherwise, get the URL from the WebContents.
        NavigationEntry entry =
                searchContentViewCore.getWebContents().getNavigationController().getPendingEntry();
        String url = entry != null
                ? entry.getUrl() : searchContentViewCore.getWebContents().getUrl();
        return url;
    }

    @Override
    public float getSearchContentViewVerticalScroll() {
        return mSearchPanelDelegate.getContentViewCore() != null
                ? mSearchPanelDelegate.getContentViewCore().computeVerticalScrollOffset() : -1.f;
    }

    @Override
    public void onContentViewVisibilityChanged(boolean isVisible) {
        if (isVisible) {
            mWereSearchResultsSeen = true;
            // If there's no current request, then either a search term resolution
            // is in progress or we should do a verbatim search now.
            if (mSearchRequest == null
                    && mPolicy.shouldCreateVerbatimRequest(mSelectionController,
                            mNetworkCommunicator.getBasePageUrl())) {
                mSearchRequest = new ContextualSearchRequest(
                        mSelectionController.getSelectedText());
                mDidLoadResolvedSearchRequest = false;
            }
            if (mSearchRequest != null && !mDidLoadResolvedSearchRequest) {
                mSearchRequest.setNormalPriority();
                loadSearchUrl();
            }
            mPolicy.updateCountersForOpen();
        }
    }

    @Override
    public void preserveBasePageSelectionOnNextLossOfFocus() {
        ContentViewCore basePageContentView = getBaseContentView();
        if (basePageContentView != null) {
            basePageContentView.preserveSelectionOnNextLossOfFocus();
        }
    }

    @Override
    public void dismissContextualSearchBar() {
        hideContextualSearch(StateChangeReason.UNKNOWN);
    }

    // --------------------------------------------------------------------------------------------
    // ContextualSearchClient -- interface used by ContentViewCore.
    // --------------------------------------------------------------------------------------------

    @Override
    public void onSelectionChanged(String selection) {
        mSelectionController.handleSelectionChanged(selection);
        updateTopControlsState(TopControlsState.BOTH, true);
    }

    @Override
    public void onSelectionEvent(int eventType, float posXPix, float posYPix) {
        mSelectionController.handleSelectionEvent(eventType, posXPix, posYPix);
    }

    @Override
    public void showUnhandledTapUIIfNeeded(final int x, final int y) {
        mDidBasePageLoadJustStart = false;
        mSelectionController.handleShowUnhandledTapUIIfNeeded(x, y);
    }

    // --------------------------------------------------------------------------------------------
    // Selection
    // --------------------------------------------------------------------------------------------

    /**
     * Returns a new {@code GestureStateListener} that will listen for events in the Base Page.
     * This listener will handle all Contextual Search-related interactions that go through the
     * listener.
     */
    public GestureStateListener getGestureStateListener() {
        return mSelectionController.getGestureStateListener();
    }

    @Override
    public void handleScroll() {
        hideContextualSearch(StateChangeReason.BASE_PAGE_SCROLL);
    }

    @Override
    public void handleInvalidTap() {
        hideContextualSearch(StateChangeReason.BASE_PAGE_TAP);
    }

    @Override
    public void handleValidTap() {
        if (isTapSupported()) {
            // Here we are starting a new Contextual Search with a Tap gesture, therefore
            // we need to clear to properly reflect that a search just started and we don't
            // have the resolved search term yet.
            mSearchRequest = null;

            // Let the policy know that a tap gesture has been received.
            mPolicy.registerTap();

            ContentViewCore baseContentView = getBaseContentView();
            if (baseContentView != null) baseContentView.getWebContents().selectWordAroundCaret();
        }
    }

    @Override
    public void handleSelection(String selection, boolean selectionValid, SelectionType type,
            float x, float y) {
        if (!selection.isEmpty()) {
            StateChangeReason stateChangeReason = type == SelectionType.TAP
                    ? StateChangeReason.TEXT_SELECT_TAP : StateChangeReason.TEXT_SELECT_LONG_PRESS;
            ContextualSearchUma.logSelectionIsValid(selectionValid);
            // Workaround to disable Contextual Search in HTML fullscreen mode. crbug.com/511977
            boolean isInFullscreenMode =
                    mActivity.getFullscreenManager().getPersistentFullscreenMode();
            if (selectionValid && !isInFullscreenMode) {
                mSearchPanelDelegate.updateBasePageSelectionYPx(y);
                showContextualSearch(stateChangeReason);
            } else {
                hideContextualSearch(stateChangeReason);
            }
        }
    }

    @Override
    public void handleSelectionDismissal() {
        if (mSearchPanelDelegate.isShowing()
                && !mIsPromotingToTab
                // If the selection is dismissed when the Panel is not peeking anymore,
                // which means the Panel is at least partially expanded, then it means
                // the selection was cleared by an external source (like JavaScript),
                // so we should not dismiss the UI in here.
                // See crbug.com/516665
                && mSearchPanelDelegate.isPeeking()) {
            hideContextualSearch(StateChangeReason.CLEARED_SELECTION);
        }
    }

    @Override
    public void handleSelectionModification(
            String selection, boolean selectionValid, float x, float y) {
        if (mSearchPanelDelegate.isShowing()) {
            if (selectionValid) {
                mSearchPanelDelegate.displaySearchTerm(selection);
            } else {
                hideContextualSearch(StateChangeReason.INVALID_SELECTION);
            }
        }
    }

    // --------------------------------------------------------------------------------------------
    // Native calls
    // --------------------------------------------------------------------------------------------

    private native long nativeInit();
    private native void nativeDestroy(long nativeContextualSearchManager);
    private native void nativeStartSearchTermResolutionRequest(long nativeContextualSearchManager,
            String selection, boolean useResolvedSearchTerm, ContentViewCore baseContentViewCore,
            boolean maySendBasePageUrl);
    private native void nativeGatherSurroundingText(long nativeContextualSearchManager,
            String selection, boolean useResolvedSearchTerm, ContentViewCore baseContentViewCore,
            boolean maySendBasePageUrl);
}
