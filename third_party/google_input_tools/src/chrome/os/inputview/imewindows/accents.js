// Copyright 2014 The ChromeOS IME Authors. All Rights Reserved.
// limitations under the License.
// See the License for the specific language governing permissions and
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// distributed under the License is distributed on an "AS-IS" BASIS,
// Unless required by applicable law or agreed to in writing, software
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// You may obtain a copy of the License at
// you may not use this file except in compliance with the License.
// Licensed under the Apache License, Version 2.0 (the "License");
//
goog.provide('i18n.input.chrome.inputview.Accents');

goog.require('goog.dom');
goog.require('goog.dom.TagName');
goog.require('goog.dom.classlist');
goog.require('goog.style');
goog.require('i18n.input.chrome.inputview.Css');
goog.require('i18n.input.chrome.inputview.util');


goog.scope(function() {
var Accents = i18n.input.chrome.inputview.Accents;
var Css = i18n.input.chrome.inputview.Css;


/**
 * The highlighted element.
 *
 * @type {Element}
 * @private
 */
Accents.highlightedItem_ = null;


/**
 * Gets the highlighted character.
 *
 * @return {string} The character.
 * @private
 */
Accents.getHighlightedAccent_ = function() {
  return Accents.highlightedItem_ ?
      Accents.highlightedItem_.textContent.trim() : '';
};


/**
 * Highlights the item according to the current coordinate of the finger.
 *
 * @param {number} x The x position of finger in screen coordinate system.
 * @param {number} y The y position of finger in screen coordinate system.
 * @param {number} offset The offset to cancel highlight.
 * @private
 */
Accents.highlightItem_ = function(x, y, offset) {
  var highlightedItem = Accents.getHighlightedItem_(x, y, offset);
  if (Accents.highlightedItem_ != highlightedItem) {
    if (Accents.highlightedItem_) {
      goog.dom.classlist.remove(Accents.highlightedItem_,
          i18n.input.chrome.inputview.Css.ELEMENT_HIGHLIGHT);
    }
    Accents.highlightedItem_ = highlightedItem;
    if (Accents.highlightedItem_ &&
        Accents.highlightedItem_.textContent.trim()) {
      goog.dom.classlist.add(Accents.highlightedItem_,
          i18n.input.chrome.inputview.Css.ELEMENT_HIGHLIGHT);
    }
  }
};


/**
 * Gets the higlighted item from |x| and |y| position.
 * @param {number} x The x position of finger in screen coordinate system.
 * @param {number} y The y position of finger in screen coordinate system.
 * @param {number} offset The offset to cancel highlight.
 * @return {Element} .
 * @private
 */
Accents.getHighlightedItem_ = function(x, y, offset) {
  var dom = goog.dom.getDomHelper();
  var row = null;
  var rows = dom.getElementsByClass(i18n.input.chrome.inputview.Css.ACCENT_ROW);
  for (var i = 0; i < rows.length; i++) {
    var coordinate = goog.style.getClientPosition(rows[i]);
    var size = goog.style.getSize(rows[i]);
    var screenYStart = coordinate.y + window.screenY;
    screenYStart = i == 0 ? screenYStart - offset : screenYStart;
    var screenYEnd = coordinate.y + window.screenY + size.height;
    screenYEnd = i == rows.length - 1 ? screenYEnd + offset : screenYEnd;
    if (screenYStart < y && screenYEnd > y) {
      row = rows[i];
      break;
    }
  }
  if (row) {
    var children = dom.getChildren(row);
    for (var i = 0; i < children.length; i++) {
      var coordinate = goog.style.getClientPosition(children[i]);
      var size = goog.style.getSize(children[i]);
      var screenXStart = coordinate.x + window.screenX;
      screenXStart = i == 0 ? screenXStart - offset : screenXStart;
      var screenXEnd = coordinate.x + window.screenX + size.width;
      screenXEnd = i == children.length - 1 ? screenXEnd + offset : screenXEnd;
      if (screenXStart < x && screenXEnd > x) {
        return children[i];
      }
    }
  }
  return null;
};


/**
 * Sets the accents which this window should display.
 *
 * @param {!Array.<string>} accents The accents to display.
 * @param {!number} numOfColumns The number of colums of this accents window.
 * @param {!number} numOfRows The number of rows of this accents window.
 * @param {number} width The width of accent key.
 * @param {number} height The height of accent key.
 * @param {number} startKeyIndex The index of the start key in bottom row.
 * @param {boolean} isCompact True if this accents window is for compact
 * keyboard.
 * @private
 */
Accents.setAccents_ = function(accents, numOfColumns, numOfRows, width,
    height, startKeyIndex, isCompact) {
  var TagName = goog.dom.TagName;
  var dom = goog.dom.getDomHelper();
  var container = dom.createDom(TagName.DIV, Css.ACCENT_CONTAINER);
  container.id = 'container';

  var orderedAccents = Accents.reorderAccents_(accents, numOfColumns, numOfRows,
      startKeyIndex);
  var row = null;
  for (var i = 0; i < orderedAccents.length; i++) {
    var keyElem = dom.createDom(TagName.DIV, Css.ACCENT_KEY);
    // Even if this is an empty key, we still need to add textDiv. Otherwise,
    // the keys have layout issues.
    var textDiv = dom.createElement(TagName.DIV);
    var text = i18n.input.chrome.inputview.util.getVisibleCharacter(
        orderedAccents[i]);
    textDiv.textContent = text;
    // If accent is a word use a smaller font size.
    goog.dom.classlist.add(textDiv, text.length > 1 ? Css.FONT_SMALL :
        (isCompact ? Css.ACCENT_COMPACT_FONT : Css.ACCENT_FULL_FONT));

    goog.style.setStyle(textDiv, 'lineHeight', height + 'px');
    dom.appendChild(keyElem, textDiv);
    if (!orderedAccents[i]) {
      goog.dom.classlist.add(keyElem, Css.ACCENT_EMPTY_KEY);
    }
    goog.style.setSize(keyElem, width, height);
    if (i % numOfColumns == 0) {
      if (row) {
        container.appendChild(row);
      }
      row = dom.createDom(TagName.DIV, Css.ACCENT_ROW);
    }
    dom.appendChild(row, keyElem);
  }
  dom.appendChild(container, row);
  dom.appendChild(document.body, container);
};


/**
 * Generates the reordered accents which is optimized for creating accent key
 * elements sequentially(from top to bottom, left to right).
 * Accent in |accents| is ordered according to its frequency(more frequently
 * used appears first). Once reordered, the more frequently used accents will be
 * positioned closer to the parent key. See tests for example.
 * @param {!Array.<string>} accents The accents to display.
 * @param {!number} numOfColumns The number of colums of this accents window.
 * @param {!number} numOfRows The number of rows of this accents window.
 * @param {number} startKeyIndex The index of the start key in bottom row.
 * @return {!Array.<string>} .
 * @private
 */
Accents.reorderAccents_ = function(accents, numOfColumns, numOfRows,
    startKeyIndex) {
  var orderedAccents = new Array(numOfColumns * numOfRows);

  var index = 0;
  // Generates the order to fill keys in a row. Start with startKeyIndex, we try
  // to fill keys on both side(right side first) of the start key.
  var rowOrder = new Array(numOfColumns);
  rowOrder[startKeyIndex] = index;
  for (var i = 1;
      startKeyIndex + i < numOfColumns || startKeyIndex - i >= 0;
      i++) {
    if (startKeyIndex + i < numOfColumns) {
      rowOrder[startKeyIndex + i] = ++index;
    }
    if (startKeyIndex - i >= 0) {
      rowOrder[startKeyIndex - i] = ++index;
    }
  }

  for (var i = numOfRows - 1; i >= 0; i--) {
    for (var j = numOfColumns - 1; j >= 0; j--) {
      index = rowOrder[j] + numOfColumns * (numOfRows - i - 1);
      if (index >= accents.length) {
        orderedAccents[i * numOfColumns + j] = '';
      } else {
        orderedAccents[i * numOfColumns + j] = accents[index];
      }
    }
  }

  return orderedAccents;
};

goog.exportSymbol('accents.highlightedAccent', Accents.getHighlightedAccent_);
goog.exportSymbol('accents.highlightItem', Accents.highlightItem_);
goog.exportSymbol('accents.setAccents', Accents.setAccents_);

});  // goog.scope
