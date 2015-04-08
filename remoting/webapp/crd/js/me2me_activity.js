// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @suppress {duplicate} */
var remoting = remoting || {};

(function() {

'use strict';

/**
 * @param {remoting.SessionConnector} sessionConnector
 * @param {remoting.Host} host
 *
 * @constructor
 * @implements {remoting.Activity}
 */
remoting.Me2MeActivity = function(sessionConnector, host) {
  /** @private */
  this.host_ = host;
  /** @private */
  this.connector_ = sessionConnector;
  /** @private */
  this.pinDialog_ =
      new remoting.PinDialog(document.getElementById('pin-dialog'), host);
  /** @private */
  this.hostUpdateDialog_ = new remoting.HostNeedsUpdateDialog(
      document.getElementById('host-needs-update-dialog'), this.host_);

  /** @private */
  this.retryOnHostOffline_ = true;
};

remoting.Me2MeActivity.prototype.dispose = function() {};

remoting.Me2MeActivity.prototype.start = function() {
  var webappVersion = chrome.runtime.getManifest().version;
  var that = this;

  this.hostUpdateDialog_.showIfNecessary(webappVersion).then(function() {
    return that.host_.options.load();
  }).then(function() {
    that.connect_();
  }).catch(function(/** remoting.Error */ error) {
    if (error.hasTag(remoting.Error.Tag.CANCELLED)) {
      remoting.setMode(remoting.AppMode.HOME);
    }
  });
};

/** @private */
remoting.Me2MeActivity.prototype.connect_ = function() {
  remoting.setMode(remoting.AppMode.CLIENT_CONNECTING);
  var host = this.host_;

  /**
   * @param {string} tokenUrl Token-issue URL received from the host.
   * @param {string} hostPublicKey Host public key (DER and Base64 encoded).
   * @param {string} scope OAuth scope to request the token for.
   * @param {function(string, string):void} onThirdPartyTokenFetched Callback.
   */
  var fetchThirdPartyToken = function(
      tokenUrl, hostPublicKey, scope, onThirdPartyTokenFetched) {
    var thirdPartyTokenFetcher = new remoting.ThirdPartyTokenFetcher(
        tokenUrl, hostPublicKey, scope, host.tokenUrlPatterns,
        onThirdPartyTokenFetched);
    thirdPartyTokenFetcher.fetchToken();
  };

  var that = this;
  /**
   * @param {boolean} supportsPairing
   * @param {function(string):void} onPinFetched
   */
  var requestPin = function(supportsPairing, onPinFetched) {
    that.pinDialog_.show(supportsPairing).then(function(/** string */ pin) {
      remoting.setMode(remoting.AppMode.CLIENT_CONNECTING);
      onPinFetched(pin);
    }).catch(function(/** remoting.Error */ error) {
      base.debug.assert(error.hasTag(remoting.Error.Tag.CANCELLED));
      remoting.setMode(remoting.AppMode.HOME);
    });
  };

  var pairingInfo = host.options.pairingInfo;
  this.connector_.connectMe2Me(host, requestPin, fetchThirdPartyToken,
                               pairingInfo.clientId, pairingInfo.sharedSecret);
};

/**
 * @param {!remoting.Error} error
 */
remoting.Me2MeActivity.prototype.onConnectionFailed = function(error) {
  var that = this;
  var onHostListRefresh = function(/** boolean */ success) {
    if (success) {
      // Get the host from the hostList for the refreshed JID.
      var host = remoting.hostList.getHostForId(that.host_.hostId);
      that.connector_.retryConnectMe2Me(host);
      return;
    }
    that.onError(error);
  };

  if (error.hasTag(remoting.Error.Tag.HOST_IS_OFFLINE) &&
      this.retryOnHostOffline_) {
    this.retryOnHostOffline_ = false;

    // The plugin will be re-created when the host finished refreshing
    remoting.hostList.refresh(onHostListRefresh);
  } else {
    this.onError(error);
  }
};

/**
 * @param {!remoting.ConnectionInfo} connectionInfo
 */
remoting.Me2MeActivity.prototype.onConnected = function(connectionInfo) {
  // Reset the refresh flag so that the next connection will retry if needed.
  this.retryOnHostOffline_ = true;

  var plugin = connectionInfo.plugin();
  if (remoting.app.hasCapability(remoting.ClientSession.Capability.CAST)) {
    plugin.extensions().register(new remoting.CastExtensionHandler());
  }
  plugin.extensions().register(new remoting.GnubbyAuthHandler());
  this.pinDialog_.requestPairingIfNecessary(connectionInfo.plugin(),
                                            this.connector_);
};

remoting.Me2MeActivity.prototype.onDisconnected = function() {
  this.showFinishDialog_(remoting.AppMode.CLIENT_SESSION_FINISHED_ME2ME);
};

/**
 * @param {!remoting.Error} error
 */
remoting.Me2MeActivity.prototype.onError = function(error) {
  var errorDiv = document.getElementById('connect-error-message');
  l10n.localizeElementFromTag(errorDiv, error.getTag());
  this.showFinishDialog_(remoting.AppMode.CLIENT_CONNECT_FAILED_ME2ME);
};

/**
 * @param {remoting.AppMode} mode
 * @private
 */
remoting.Me2MeActivity.prototype.showFinishDialog_ = function(mode) {
  var dialog = new remoting.MessageDialog(
      mode,
      document.getElementById('client-finished-me2me-button'),
      document.getElementById('client-reconnect-button'));

  /** @typedef {remoting.MessageDialog.Result} */
  var Result = remoting.MessageDialog.Result;
  var that = this;

  dialog.show().then(function(/** Result */result) {
    if (result === Result.PRIMARY) {
      remoting.setMode(remoting.AppMode.HOME);
    } else {
      that.connector_.reconnect();
      remoting.setMode(remoting.AppMode.CLIENT_CONNECTING);
    }
  });
};

/**
 * @param {HTMLElement} rootElement
 * @param {remoting.Host} host
 * @constructor
 */
remoting.HostNeedsUpdateDialog = function(rootElement, host) {
  /** @private */
  this.host_ = host;
  /** @private */
  this.dialog_ = new remoting.MessageDialog(
      remoting.AppMode.CLIENT_HOST_NEEDS_UPGRADE,
      rootElement.querySelector('.connect-button'),
      rootElement.querySelector('.cancel-button'));

  l10n.localizeElementFromTag(
      rootElement.querySelector('.host-needs-update-message'),
      /*i18n-content*/'HOST_NEEDS_UPDATE_TITLE', host.hostName);
};

/**
 * Shows the HostNeedsUpdateDialog if the user is trying to connect to a legacy
 * host.
 *
 * @param {string} webappVersion
 * @return {Promise}  Promise that resolves when no update is required or
 *    when the user ignores the update warning.  Rejects with
 *    |remoting.Error.CANCELLED| if the user cancels the connection.
 */
remoting.HostNeedsUpdateDialog.prototype.showIfNecessary =
    function(webappVersion) {
  if (!remoting.Host.needsUpdate(this.host_, webappVersion)) {
    return Promise.resolve();
  }
  /** @typedef {remoting.MessageDialog.Result} */
  var Result = remoting.MessageDialog.Result;
  return this.dialog_.show().then(function(/** Result */ result) {
    if (result === Result.SECONDARY) {
      return Promise.reject(new remoting.Error(remoting.Error.Tag.CANCELLED));
    }
  });
};

/**
 * @param {HTMLElement} rootElement
 * @param {remoting.Host} host
 * @constructor
 */
remoting.PinDialog = function(rootElement, host) {
  /** @private */
  this.rootElement_ = rootElement;
  /** @private */
  this.pairingCheckbox_ = rootElement.querySelector('.pairing-checkbox');
  /** @private */
  this.pinInput_ = rootElement.querySelector('.pin-inputField');
  /** @private */
  this.host_ = host;
  /** @private */
  this.dialog_ = new remoting.InputDialog(
    remoting.AppMode.CLIENT_PIN_PROMPT,
    this.rootElement_.querySelector('form'),
    this.pinInput_,
    this.rootElement_.querySelector('.cancel-button'));
};


/**
 * @param {boolean} supportsPairing
 * @return {Promise<string>}  Promise that resolves with the PIN or rejects with
 *    |remoting.Error.CANCELLED| if the user cancels the connection.
 */
remoting.PinDialog.prototype.show = function(supportsPairing) {
  // Reset the UI.
  this.pairingCheckbox_.checked = false;
  this.rootElement_.querySelector('.pairing-section').hidden = !supportsPairing;
  var message = this.rootElement_.querySelector('.pin-message');
  l10n.localizeElement(message, this.host_.hostName);
  this.pinInput_.value = '';
  return this.dialog_.show();
};

/**
 * @param {remoting.ClientPlugin} plugin
 * @param {remoting.SessionConnector} connector
 */
remoting.PinDialog.prototype.requestPairingIfNecessary =
    function(plugin, connector) {
  if (this.pairingCheckbox_.checked) {
    var that = this;
    /**
     * @param {string} clientId
     * @param {string} sharedSecret
     */
    var onPairingComplete = function(clientId, sharedSecret) {
      that.host_.options.pairingInfo.clientId = clientId;
      that.host_.options.pairingInfo.sharedSecret = sharedSecret;
      that.host_.options.save();
      connector.updatePairingInfo(clientId, sharedSecret);
    };

    // Use the platform name as a proxy for the local computer name.
    // TODO(jamiewalch): Use a descriptive name for the local computer, for
    // example, its Chrome Sync name.
    var clientName = '';
    if (remoting.platformIsMac()) {
      clientName = 'Mac';
    } else if (remoting.platformIsWindows()) {
      clientName = 'Windows';
    } else if (remoting.platformIsChromeOS()) {
      clientName = 'ChromeOS';
    } else if (remoting.platformIsLinux()) {
      clientName = 'Linux';
    } else {
      console.log('Unrecognized client platform. Using navigator.platform.');
      clientName = navigator.platform;
    }
    plugin.requestPairing(clientName, onPairingComplete);
  }
};

})();
