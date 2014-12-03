// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.infobar;

import org.chromium.content_public.browser.WebContents;

/**
 * Generates an InfoBar for the data reduction proxy that contains a message and a link to the
 * data reduction proxy settings menu.
 */
public class DataReductionProxyInfoBar extends ConfirmInfoBar {
    private static String sTitle;
    private static String sLinkText;

    /**
     * Launch a data reduction proxy {@link InfoBar} with the specified title and link
     * text. Clicking the link will open the specified settings page.
     * @param webContents The {@link WebContents} in which to open the {@link InfoBar}.
     * @param title The text to display in the {@link InfoBar}.
     * @param linkText The text to display in the link in the {@link InfoBar}.
     * @param linkUrl The URL to be loaded when the link text is clicked.
     */
    public static void launch(WebContents webContents,
            String title,
            String linkText,
            String linkUrl) {
        sTitle = title;
        sLinkText = linkText;
        DataReductionProxyInfoBarDelegate.launch(webContents, linkUrl);
    }

    /**
     * Use the method above instead. TODO(newt): delete this once all callers are updated.
     */
    public static void launch(WebContents webContents,
            String unused1,
            String unused2,
            String title,
            String linkText,
            String linkUrl) {
        launch(webContents, title, linkText, linkUrl);
    }

    DataReductionProxyInfoBar(long nativeInfoBar, int iconDrawableId) {
        super(nativeInfoBar, null, iconDrawableId, sTitle,
                sLinkText, null, null);
    }

    @Override
    public void createContent(InfoBarLayout layout) {
        layout.setButtons(sLinkText, null);
    }

    @Override
    public void onButtonClicked(boolean isPrimaryButton) {
        if (!isPrimaryButton) return;
        onLinkClicked();
    }
}
