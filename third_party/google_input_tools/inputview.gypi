# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is auto-generated using update.py.
{
  'variables': {
    'inputview_sources': [
      'src/chrome/os/common.js',
      'src/chrome/os/datasource.js',
      'src/chrome/os/inputview/adapter.js',
      'src/chrome/os/inputview/candidatesinfo.js',
      'src/chrome/os/inputview/canvas.js',
      'src/chrome/os/inputview/conditionname.js',
      'src/chrome/os/inputview/config/compact_letter_characters.js',
      'src/chrome/os/inputview/config/compact_more_characters.js',
      'src/chrome/os/inputview/config/compact_numberpad_characters.js',
      'src/chrome/os/inputview/config/compact_symbol_characters.js',
      'src/chrome/os/inputview/config/compact_util.js',
      'src/chrome/os/inputview/config/constants.js',
      'src/chrome/os/inputview/config/contextlayoututil.js',
      'src/chrome/os/inputview/config/util.js',
      'src/chrome/os/inputview/controller.js',
      'src/chrome/os/inputview/covariance.js',
      'src/chrome/os/inputview/css.js',
      'src/chrome/os/inputview/direction.js',
      'src/chrome/os/inputview/dom.js',
      'src/chrome/os/inputview/elements/content/altdataview.js',
      'src/chrome/os/inputview/elements/content/backspacekey.js',
      'src/chrome/os/inputview/elements/content/candidate.js',
      'src/chrome/os/inputview/elements/content/candidatebutton.js',
      'src/chrome/os/inputview/elements/content/candidateview.js',
      'src/chrome/os/inputview/elements/content/canvasview.js',
      'src/chrome/os/inputview/elements/content/character.js',
      'src/chrome/os/inputview/elements/content/characterkey.js',
      'src/chrome/os/inputview/elements/content/charactermodel.js',
      'src/chrome/os/inputview/elements/content/compactkey.js',
      'src/chrome/os/inputview/elements/content/compactkeymodel.js',
      'src/chrome/os/inputview/elements/content/emojikey.js',
      'src/chrome/os/inputview/elements/content/emojiview.js',
      'src/chrome/os/inputview/elements/content/enswitcherkey.js',
      'src/chrome/os/inputview/elements/content/enterkey.js',
      'src/chrome/os/inputview/elements/content/expandedcandidateview.js',
      'src/chrome/os/inputview/elements/content/functionalkey.js',
      'src/chrome/os/inputview/elements/content/gaussianestimator.js',
      'src/chrome/os/inputview/elements/content/gesturecanvasview.js',
      'src/chrome/os/inputview/elements/content/handwritingview.js',
      'src/chrome/os/inputview/elements/content/keyboardview.js',
      'src/chrome/os/inputview/elements/content/keysetview.js',
      'src/chrome/os/inputview/elements/content/material/spacekey.js',
      'src/chrome/os/inputview/elements/content/menuitem.js',
      'src/chrome/os/inputview/elements/content/menukey.js',
      'src/chrome/os/inputview/elements/content/menuview.js',
      'src/chrome/os/inputview/elements/content/modifierkey.js',
      'src/chrome/os/inputview/elements/content/morekeysshiftoperation.js',
      'src/chrome/os/inputview/elements/content/pageindicator.js',
      'src/chrome/os/inputview/elements/content/selectview.js',
      'src/chrome/os/inputview/elements/content/softkey.js',
      'src/chrome/os/inputview/elements/content/spacekey.js',
      'src/chrome/os/inputview/elements/content/spanelement.js',
      'src/chrome/os/inputview/elements/content/swipeview.js',
      'src/chrome/os/inputview/elements/content/switcherkey.js',
      'src/chrome/os/inputview/elements/content/tabbarkey.js',
      'src/chrome/os/inputview/elements/content/toolbarbutton.js',
      'src/chrome/os/inputview/elements/content/voiceview.js',
      'src/chrome/os/inputview/elements/element.js',
      'src/chrome/os/inputview/elements/elementtype.js',
      'src/chrome/os/inputview/elements/layout/extendedlayout.js',
      'src/chrome/os/inputview/elements/layout/handwritinglayout.js',
      'src/chrome/os/inputview/elements/layout/linearlayout.js',
      'src/chrome/os/inputview/elements/layout/softkeyview.js',
      'src/chrome/os/inputview/elements/layout/verticallayout.js',
      'src/chrome/os/inputview/elements/weightable.js',
      'src/chrome/os/inputview/emojitype.js',
      'src/chrome/os/inputview/events.js',
      'src/chrome/os/inputview/events/keycodes.js',
      'src/chrome/os/inputview/featurename.js',
      'src/chrome/os/inputview/featuretracker.js',
      'src/chrome/os/inputview/globalflags.js',
      'src/chrome/os/inputview/globalsettings.js',
      'src/chrome/os/inputview/handler/pointeractionbundle.js',
      'src/chrome/os/inputview/handler/pointerhandler.js',
      'src/chrome/os/inputview/handler/swipestate.js',
      'src/chrome/os/inputview/handler/util.js',
      'src/chrome/os/inputview/hwt_css.js',
      'src/chrome/os/inputview/hwt_eventtype.js',
      'src/chrome/os/inputview/hwt_util.js',
      'src/chrome/os/inputview/imewindows/accents.js',
      'src/chrome/os/inputview/inputtoolcode.js',
      'src/chrome/os/inputview/keyboardcontainer.js',
      'src/chrome/os/inputview/layouts/compactspacerow.js',
      'src/chrome/os/inputview/layouts/material/compactspacerow.js',
      'src/chrome/os/inputview/layouts/material/rowsof101.js',
      'src/chrome/os/inputview/layouts/material/rowsof102.js',
      'src/chrome/os/inputview/layouts/material/rowsofcompact.js',
      'src/chrome/os/inputview/layouts/material/rowsofjp.js',
      'src/chrome/os/inputview/layouts/material/spacerow.js',
      'src/chrome/os/inputview/layouts/material/util.js',
      'src/chrome/os/inputview/layouts/rowsof101.js',
      'src/chrome/os/inputview/layouts/rowsof102.js',
      'src/chrome/os/inputview/layouts/rowsofcompact.js',
      'src/chrome/os/inputview/layouts/rowsofjp.js',
      'src/chrome/os/inputview/layouts/rowsofnumberpad.js',
      'src/chrome/os/inputview/layouts/spacerow.js',
      'src/chrome/os/inputview/layouts/util.js',
      'src/chrome/os/inputview/m17nmodel.js',
      'src/chrome/os/inputview/model.js',
      'src/chrome/os/inputview/perftracker.js',
      'src/chrome/os/inputview/pointerconfig.js',
      'src/chrome/os/inputview/readystate.js',
      'src/chrome/os/inputview/settings.js',
      'src/chrome/os/inputview/sizespec.js',
      'src/chrome/os/inputview/specnodename.js',
      'src/chrome/os/inputview/statemanager.js',
      'src/chrome/os/inputview/statetype.js',
      'src/chrome/os/inputview/strokehandler.js',
      'src/chrome/os/inputview/swipedirection.js',
      'src/chrome/os/inputview/util.js',
      'src/chrome/os/keyboard/eventtype.js',
      'src/chrome/os/keyboard/keycode.js',
      'src/chrome/os/keyboard/layoutevent.js',
      'src/chrome/os/keyboard/model.js',
      'src/chrome/os/keyboard/parsedlayout.js',
      'src/chrome/os/message/contexttype.js',
      'src/chrome/os/message/event.js',
      'src/chrome/os/message/name.js',
      'src/chrome/os/message/source.js',
      'src/chrome/os/message/type.js',
      'src/chrome/os/sounds/soundcontroller.js',
      'src/chrome/os/sounds/sounds.js',
      'src/chrome/os/statistics.js',
      'third_party/closure_library/closure/goog/a11y/aria/announcer.js',
      'third_party/closure_library/closure/goog/a11y/aria/aria.js',
      'third_party/closure_library/closure/goog/a11y/aria/attributes.js',
      'third_party/closure_library/closure/goog/a11y/aria/datatables.js',
      'third_party/closure_library/closure/goog/a11y/aria/roles.js',
      'third_party/closure_library/closure/goog/array/array.js',
      'third_party/closure_library/closure/goog/asserts/asserts.js',
      'third_party/closure_library/closure/goog/async/delay.js',
      'third_party/closure_library/closure/goog/async/nexttick.js',
      'third_party/closure_library/closure/goog/async/run.js',
      'third_party/closure_library/closure/goog/debug/debug.js',
      'third_party/closure_library/closure/goog/debug/entrypointregistry.js',
      'third_party/closure_library/closure/goog/debug/error.js',
      'third_party/closure_library/closure/goog/debug/logbuffer.js',
      'third_party/closure_library/closure/goog/debug/logger.js',
      'third_party/closure_library/closure/goog/debug/logrecord.js',
      'third_party/closure_library/closure/goog/disposable/disposable.js',
      'third_party/closure_library/closure/goog/disposable/idisposable.js',
      'third_party/closure_library/closure/goog/dom/browserfeature.js',
      'third_party/closure_library/closure/goog/dom/classlist.js',
      'third_party/closure_library/closure/goog/dom/dom.js',
      'third_party/closure_library/closure/goog/dom/nodetype.js',
      'third_party/closure_library/closure/goog/dom/tagname.js',
      'third_party/closure_library/closure/goog/dom/vendor.js',
      'third_party/closure_library/closure/goog/events/browserevent.js',
      'third_party/closure_library/closure/goog/events/browserfeature.js',
      'third_party/closure_library/closure/goog/events/event.js',
      'third_party/closure_library/closure/goog/events/eventhandler.js',
      'third_party/closure_library/closure/goog/events/eventid.js',
      'third_party/closure_library/closure/goog/events/events.js',
      'third_party/closure_library/closure/goog/events/eventtarget.js',
      'third_party/closure_library/closure/goog/events/eventtype.js',
      'third_party/closure_library/closure/goog/events/keycodes.js',
      'third_party/closure_library/closure/goog/events/keyhandler.js',
      'third_party/closure_library/closure/goog/events/listenable.js',
      'third_party/closure_library/closure/goog/events/listener.js',
      'third_party/closure_library/closure/goog/events/listenermap.js',
      'third_party/closure_library/closure/goog/functions/functions.js',
      'third_party/closure_library/closure/goog/i18n/bidi.js',
      'third_party/closure_library/closure/goog/iter/iter.js',
      'third_party/closure_library/closure/goog/labs/useragent/browser.js',
      'third_party/closure_library/closure/goog/labs/useragent/engine.js',
      'third_party/closure_library/closure/goog/labs/useragent/util.js',
      'third_party/closure_library/closure/goog/log/log.js',
      'third_party/closure_library/closure/goog/math/box.js',
      'third_party/closure_library/closure/goog/math/coordinate.js',
      'third_party/closure_library/closure/goog/math/math.js',
      'third_party/closure_library/closure/goog/math/rect.js',
      'third_party/closure_library/closure/goog/math/size.js',
      'third_party/closure_library/closure/goog/net/jsloader.js',
      'third_party/closure_library/closure/goog/object/object.js',
      'third_party/closure_library/closure/goog/positioning/abstractposition.js',
      'third_party/closure_library/closure/goog/positioning/anchoredposition.js',
      'third_party/closure_library/closure/goog/positioning/anchoredviewportposition.js',
      'third_party/closure_library/closure/goog/positioning/positioning.js',
      'third_party/closure_library/closure/goog/promise/promise.js',
      'third_party/closure_library/closure/goog/promise/resolver.js',
      'third_party/closure_library/closure/goog/promise/thenable.js',
      'third_party/closure_library/closure/goog/reflect/reflect.js',
      'third_party/closure_library/closure/goog/string/string.js',
      'third_party/closure_library/closure/goog/structs/collection.js',
      'third_party/closure_library/closure/goog/structs/map.js',
      'third_party/closure_library/closure/goog/structs/set.js',
      'third_party/closure_library/closure/goog/structs/structs.js',
      'third_party/closure_library/closure/goog/style/bidi.js',
      'third_party/closure_library/closure/goog/style/style.js',
      'third_party/closure_library/closure/goog/testing/watchers.js',
      'third_party/closure_library/closure/goog/timer/timer.js',
      'third_party/closure_library/closure/goog/ui/component.js',
      'third_party/closure_library/closure/goog/ui/container.js',
      'third_party/closure_library/closure/goog/ui/containerrenderer.js',
      'third_party/closure_library/closure/goog/ui/control.js',
      'third_party/closure_library/closure/goog/ui/controlcontent.js',
      'third_party/closure_library/closure/goog/ui/controlrenderer.js',
      'third_party/closure_library/closure/goog/ui/decorate.js',
      'third_party/closure_library/closure/goog/ui/idgenerator.js',
      'third_party/closure_library/closure/goog/ui/registry.js',
      'third_party/closure_library/closure/goog/uri/utils.js',
      'third_party/closure_library/closure/goog/useragent/useragent.js',
      'third_party/closure_library/third_party/closure/goog/mochikit/async/deferred.js'
    ]
  }
}