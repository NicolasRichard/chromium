// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//<include src="../../../../ui/webui/resources/js/cr.js">
//<include src="../../../../ui/webui/resources/js/load_time_data.js">
//<include src="../../../../ui/webui/resources/js/i18n_template_no_process.js">
//<include src="../../../../ui/webui/resources/js/cr/event_target.js">
//<include src="../../../../ui/webui/resources/js/cr/ui/dialogs.js">

(function() {
'use strict';

//<include src="../cws_widget/app_installer.js">
//<include src="../cws_widget/cws_webview_client.js">
//<include src="../cws_widget/cws_widget_container.js">
//<include src="../cws_widget/cws_widget_container_error_dialog.js">

/**
 * @type {?{
 *   filter: !Object.<string, *>,
 *   webstoreUrl: ?string
 * }}
 */
window.params = window.params || null;

/**
 * @param {string} id Element id.
 * @return {HTMLElement} The found element, or null.
 */
function $(id) {
  return document.getElementById(id);
}

/**
 * Default strings.
 */
var defaultStrings = {
  'language': 'en',
  'LINK_TO_WEBSTORE': '[LOCALIZE ME] Learn more...',
  'INSTALLATION_FAILED_MESSAGE': '[LOCALIZE ME] Installation failed!',
  'OK_BUTTON': '[LOCALIZE ME] OK',
  'TITLE_PRINTER_PROVIDERS': '[LOCALIZE ME] Select app for your printer'
};

/**
 * @param {string} id The string id.
 * @return {string}
 */
function getString(id) {
  return loadTimeData.getString(id) || defaultStrings[id] || '';
}

/**
 * @param {Object.<string, string>} strings Localized strings used by the
 *     container.
 * @return {!CWSWidgetContainer.PlatformDelegate}
 */
function createPlatformDelegate(strings) {
  return {
    strings: {
      UI_LOCALE: getString('language'),
      LINK_TO_WEBSTORE: getString('LINK_TO_WEBSTORE'),
      INSTALLATION_FAILED_MESSAGE: getString('INSTALLATION_FAILED_MESSAGE')
    },

    metricsImpl: {
      /**
       * @param {string} enumName
       * @param {number} value
       * @param {number} enumSize
       */
      recordEnum: function(enumName, value, enumSize) {},

      /** @param {string} actionName */
      recordUserAction: function(actionName) {},

      /** @param {string} intervalName */
      startInterval: function(intervalName) {},

      /** @param {string} intervalName */
      recordInterval: function(intervalName) {}
    },

    /**
     * @param {string} itemId Item to be installed.
     * @param {function(?string)} callback Callback param is the error message,
     *     which is set to null on success.
     */
    installWebstoreItem: function(itemId, callback) {
      chrome.webstoreWidgetPrivate.installWebstoreItem(
          itemId,
          false,
          function() {
            callback(chrome.runtime.lastError ?
                chrome.runtime.lastError.message || 'UNKNOWN_ERROR' : null);
          });
    },

    /** @param {function(Array.<string>)} callback */
    getInstalledItems: function(callback) { callback([]); },

    /**
     * @param {function(?string)} callback The argument is the fetche3d access
     *     token. Null on error.
     */
    requestWebstoreAccessToken: function(callback) {
      chrome.fileManagerPrivate.requestWebStoreAccessToken(function(token) {
        if (chrome.runtime.lastError) {
          console.error('Error getting access token: ' +
                        chrome.runtime.lastError.message);
          callback(null);
          return;
        }
        callback(token);
      });
    }
  };
}

function initializeTopbarButtons() {
  $('close-button').addEventListener('click', function(e) {
    e.preventDefault();
    chrome.app.window.current().close();
  });

  $('close-button').addEventListener('mousedown', function(e) {
    e.preventDefault();
  });

  $('minimize-button').addEventListener('click', function(e) {
    e.preventDefault();
    chrome.app.window.current().minimize();
  });

  $('minimize-button').addEventListener('mousedown', function(e) {
    e.preventDefault();
  });
}

window.addEventListener('DOMContentLoaded', function() {
  initializeTopbarButtons();

  chrome.webstoreWidgetPrivate.getStrings(function(strings) {
    loadTimeData.data = strings;
    i18nTemplate.process(document, loadTimeData);

    cr.ui.dialogs.BaseDialog.OK_LABEL = getString('OK_BUTTON');

    document.title = getString('TITLE_PRINTER_PROVIDERS');
    $('title').textContent = document.title;

    if (!window.params) {
      console.error('Params not set!');
      return;
    }

    /** @type {!CWSWidgetContainer.PlatformDelegate} */
    var platformDelegate = createPlatformDelegate(strings);

    var root = $('widget-container-root');
    if (!root) {
      console.error('No root element');
      return;
    }

    /** @type {!CWSWidgetContainer} */
    var widgetContainer = new CWSWidgetContainer(
        document, root, platformDelegate, {} /* state */);

    widgetContainer.ready()
        /** @return {!Promise.<CWSWidgetContainer.ResolveReason>} */
        .then(function() {
          return widgetContainer.start(window.params.filter,
                                       window.params.webstoreUrl);
        })
        /** @param {!CWSWidgetContainer.ResolveReason} reason */
        .then(function(reason) {
          chrome.app.window.current().close();
        })
        /** @param {*} error */
        .catch(function(error) {
          // TODO(tbarzic): Add error UI.
          console.error(error);
        });
  });
});
})();
