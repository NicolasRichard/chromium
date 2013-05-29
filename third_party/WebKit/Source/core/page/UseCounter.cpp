/*
 * Copyright (C) 2012 Google, Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. ``AS IS'' AND ANY
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

#include "config.h"
#include "core/page/UseCounter.h"

#include "core/css/CSSStyleSheet.h"
#include "core/css/StyleSheetContents.h"
#include "core/dom/Document.h"
#include "core/dom/ScriptExecutionContext.h"
#include "core/page/DOMWindow.h"
#include "core/page/Page.h"
#include "core/page/PageConsole.h"
#include "core/platform/HistogramSupport.h"
#include "wtf/text/WTFString.h"

namespace WebCore {

static int totalPagesMeasuredCSSSampleId() { return 1; }

// FIXME : This mapping should be autogenerated. This function should
//         be moved to a separate file and a script run at build time
//         to detect new values in CSSPropertyID and add them to the
//         end of this function. This file would be checked in.
//         https://code.google.com/p/chromium/issues/detail?id=234940
static int mapCSSPropertyIdToCSSSampleId(int id)
{
    CSSPropertyID cssPropertyID = convertToCSSPropertyID(id);

    switch (cssPropertyID) {
    // Begin at 2, because 1 is reserved for totalPagesMeasuredCSSSampleId.
    case CSSPropertyColor: return 2;
    case CSSPropertyDirection: return 3;
    case CSSPropertyDisplay: return 4;
    case CSSPropertyFont: return 5;
    case CSSPropertyFontFamily: return 6;
    case CSSPropertyFontSize: return 7;
    case CSSPropertyFontStyle: return 8;
    case CSSPropertyFontVariant: return 9;
    case CSSPropertyFontWeight: return 10;
    case CSSPropertyTextRendering: return 11;
    case CSSPropertyWebkitFontFeatureSettings: return 12;
    case CSSPropertyWebkitFontKerning: return 13;
    case CSSPropertyWebkitFontSmoothing: return 14;
    case CSSPropertyWebkitFontVariantLigatures: return 15;
    case CSSPropertyWebkitLocale: return 16;
    case CSSPropertyWebkitTextOrientation: return 17;
    case CSSPropertyWebkitWritingMode: return 18;
    case CSSPropertyZoom: return 19;
    case CSSPropertyLineHeight: return 20;
    case CSSPropertyBackground: return 21;
    case CSSPropertyBackgroundAttachment: return 22;
    case CSSPropertyBackgroundClip: return 23;
    case CSSPropertyBackgroundColor: return 24;
    case CSSPropertyBackgroundImage: return 25;
    case CSSPropertyBackgroundOrigin: return 26;
    case CSSPropertyBackgroundPosition: return 27;
    case CSSPropertyBackgroundPositionX: return 28;
    case CSSPropertyBackgroundPositionY: return 29;
    case CSSPropertyBackgroundRepeat: return 30;
    case CSSPropertyBackgroundRepeatX: return 31;
    case CSSPropertyBackgroundRepeatY: return 32;
    case CSSPropertyBackgroundSize: return 33;
    case CSSPropertyBorder: return 34;
    case CSSPropertyBorderBottom: return 35;
    case CSSPropertyBorderBottomColor: return 36;
    case CSSPropertyBorderBottomLeftRadius: return 37;
    case CSSPropertyBorderBottomRightRadius: return 38;
    case CSSPropertyBorderBottomStyle: return 39;
    case CSSPropertyBorderBottomWidth: return 40;
    case CSSPropertyBorderCollapse: return 41;
    case CSSPropertyBorderColor: return 42;
    case CSSPropertyBorderImage: return 43;
    case CSSPropertyBorderImageOutset: return 44;
    case CSSPropertyBorderImageRepeat: return 45;
    case CSSPropertyBorderImageSlice: return 46;
    case CSSPropertyBorderImageSource: return 47;
    case CSSPropertyBorderImageWidth: return 48;
    case CSSPropertyBorderLeft: return 49;
    case CSSPropertyBorderLeftColor: return 50;
    case CSSPropertyBorderLeftStyle: return 51;
    case CSSPropertyBorderLeftWidth: return 52;
    case CSSPropertyBorderRadius: return 53;
    case CSSPropertyBorderRight: return 54;
    case CSSPropertyBorderRightColor: return 55;
    case CSSPropertyBorderRightStyle: return 56;
    case CSSPropertyBorderRightWidth: return 57;
    case CSSPropertyBorderSpacing: return 58;
    case CSSPropertyBorderStyle: return 59;
    case CSSPropertyBorderTop: return 60;
    case CSSPropertyBorderTopColor: return 61;
    case CSSPropertyBorderTopLeftRadius: return 62;
    case CSSPropertyBorderTopRightRadius: return 63;
    case CSSPropertyBorderTopStyle: return 64;
    case CSSPropertyBorderTopWidth: return 65;
    case CSSPropertyBorderWidth: return 66;
    case CSSPropertyBottom: return 67;
    case CSSPropertyBoxShadow: return 68;
    case CSSPropertyBoxSizing: return 69;
    case CSSPropertyCaptionSide: return 70;
    case CSSPropertyClear: return 71;
    case CSSPropertyClip: return 72;
    case CSSPropertyWebkitClipPath: return 73;
    case CSSPropertyContent: return 74;
    case CSSPropertyCounterIncrement: return 75;
    case CSSPropertyCounterReset: return 76;
    case CSSPropertyCursor: return 77;
    case CSSPropertyEmptyCells: return 78;
    case CSSPropertyFloat: return 79;
    case CSSPropertyFontStretch: return 80;
    case CSSPropertyHeight: return 81;
    case CSSPropertyImageRendering: return 82;
    case CSSPropertyLeft: return 83;
    case CSSPropertyLetterSpacing: return 84;
    case CSSPropertyListStyle: return 85;
    case CSSPropertyListStyleImage: return 86;
    case CSSPropertyListStylePosition: return 87;
    case CSSPropertyListStyleType: return 88;
    case CSSPropertyMargin: return 89;
    case CSSPropertyMarginBottom: return 90;
    case CSSPropertyMarginLeft: return 91;
    case CSSPropertyMarginRight: return 92;
    case CSSPropertyMarginTop: return 93;
    case CSSPropertyMaxHeight: return 94;
    case CSSPropertyMaxWidth: return 95;
    case CSSPropertyMinHeight: return 96;
    case CSSPropertyMinWidth: return 97;
    case CSSPropertyOpacity: return 98;
    case CSSPropertyOrphans: return 99;
    case CSSPropertyOutline: return 100;
    case CSSPropertyOutlineColor: return 101;
    case CSSPropertyOutlineOffset: return 102;
    case CSSPropertyOutlineStyle: return 103;
    case CSSPropertyOutlineWidth: return 104;
    case CSSPropertyOverflow: return 105;
    case CSSPropertyOverflowWrap: return 106;
    case CSSPropertyOverflowX: return 107;
    case CSSPropertyOverflowY: return 108;
    case CSSPropertyPadding: return 109;
    case CSSPropertyPaddingBottom: return 110;
    case CSSPropertyPaddingLeft: return 111;
    case CSSPropertyPaddingRight: return 112;
    case CSSPropertyPaddingTop: return 113;
    case CSSPropertyPage: return 114;
    case CSSPropertyPageBreakAfter: return 115;
    case CSSPropertyPageBreakBefore: return 116;
    case CSSPropertyPageBreakInside: return 117;
    case CSSPropertyPointerEvents: return 118;
    case CSSPropertyPosition: return 119;
    case CSSPropertyQuotes: return 120;
    case CSSPropertyResize: return 121;
    case CSSPropertyRight: return 122;
    case CSSPropertySize: return 123;
    case CSSPropertySrc: return 124;
    case CSSPropertySpeak: return 125;
    case CSSPropertyTableLayout: return 126;
    case CSSPropertyTabSize: return 127;
    case CSSPropertyTextAlign: return 128;
    case CSSPropertyTextDecoration: return 129;
    case CSSPropertyTextIndent: return 130;
    /* Removed CSSPropertyTextLineThrough - 131 */
    case CSSPropertyTextLineThroughColor: return 132;
    case CSSPropertyTextLineThroughMode: return 133;
    case CSSPropertyTextLineThroughStyle: return 134;
    case CSSPropertyTextLineThroughWidth: return 135;
    case CSSPropertyTextOverflow: return 136;
    /* Removed CSSPropertyTextOverline - 137 */
    case CSSPropertyTextOverlineColor: return 138;
    case CSSPropertyTextOverlineMode: return 139;
    case CSSPropertyTextOverlineStyle: return 140;
    case CSSPropertyTextOverlineWidth: return 141;
    case CSSPropertyTextShadow: return 142;
    case CSSPropertyTextTransform: return 143;
    /* Removed CSSPropertyTextUnderline - 144 */
    case CSSPropertyTextUnderlineColor: return 145;
    case CSSPropertyTextUnderlineMode: return 146;
    case CSSPropertyTextUnderlineStyle: return 147;
    case CSSPropertyTextUnderlineWidth: return 148;
    case CSSPropertyTop: return 149;
    case CSSPropertyTransition: return 150;
    case CSSPropertyTransitionDelay: return 151;
    case CSSPropertyTransitionDuration: return 152;
    case CSSPropertyTransitionProperty: return 153;
    case CSSPropertyTransitionTimingFunction: return 154;
    case CSSPropertyUnicodeBidi: return 155;
    case CSSPropertyUnicodeRange: return 156;
    case CSSPropertyVerticalAlign: return 157;
    case CSSPropertyVisibility: return 158;
    case CSSPropertyWhiteSpace: return 159;
    case CSSPropertyWidows: return 160;
    case CSSPropertyWidth: return 161;
    case CSSPropertyWordBreak: return 162;
    case CSSPropertyWordSpacing: return 163;
    case CSSPropertyWordWrap: return 164;
    case CSSPropertyZIndex: return 165;
    case CSSPropertyWebkitAnimation: return 166;
    case CSSPropertyWebkitAnimationDelay: return 167;
    case CSSPropertyWebkitAnimationDirection: return 168;
    case CSSPropertyWebkitAnimationDuration: return 169;
    case CSSPropertyWebkitAnimationFillMode: return 170;
    case CSSPropertyWebkitAnimationIterationCount: return 171;
    case CSSPropertyWebkitAnimationName: return 172;
    case CSSPropertyWebkitAnimationPlayState: return 173;
    case CSSPropertyWebkitAnimationTimingFunction: return 174;
    case CSSPropertyWebkitAppearance: return 175;
    case CSSPropertyWebkitAspectRatio: return 176;
    case CSSPropertyWebkitBackfaceVisibility: return 177;
    case CSSPropertyWebkitBackgroundClip: return 178;
    case CSSPropertyWebkitBackgroundComposite: return 179;
    case CSSPropertyWebkitBackgroundOrigin: return 180;
    case CSSPropertyWebkitBackgroundSize: return 181;
    case CSSPropertyWebkitBorderAfter: return 182;
    case CSSPropertyWebkitBorderAfterColor: return 183;
    case CSSPropertyWebkitBorderAfterStyle: return 184;
    case CSSPropertyWebkitBorderAfterWidth: return 185;
    case CSSPropertyWebkitBorderBefore: return 186;
    case CSSPropertyWebkitBorderBeforeColor: return 187;
    case CSSPropertyWebkitBorderBeforeStyle: return 188;
    case CSSPropertyWebkitBorderBeforeWidth: return 189;
    case CSSPropertyWebkitBorderEnd: return 190;
    case CSSPropertyWebkitBorderEndColor: return 191;
    case CSSPropertyWebkitBorderEndStyle: return 192;
    case CSSPropertyWebkitBorderEndWidth: return 193;
    case CSSPropertyWebkitBorderFit: return 194;
    case CSSPropertyWebkitBorderHorizontalSpacing: return 195;
    case CSSPropertyWebkitBorderImage: return 196;
    case CSSPropertyWebkitBorderRadius: return 197;
    case CSSPropertyWebkitBorderStart: return 198;
    case CSSPropertyWebkitBorderStartColor: return 199;
    case CSSPropertyWebkitBorderStartStyle: return 200;
    case CSSPropertyWebkitBorderStartWidth: return 201;
    case CSSPropertyWebkitBorderVerticalSpacing: return 202;
    case CSSPropertyWebkitBoxAlign: return 203;
    case CSSPropertyWebkitBoxDirection: return 204;
    case CSSPropertyWebkitBoxFlex: return 205;
    case CSSPropertyWebkitBoxFlexGroup: return 206;
    case CSSPropertyWebkitBoxLines: return 207;
    case CSSPropertyWebkitBoxOrdinalGroup: return 208;
    case CSSPropertyWebkitBoxOrient: return 209;
    case CSSPropertyWebkitBoxPack: return 210;
    case CSSPropertyWebkitBoxReflect: return 211;
    case CSSPropertyWebkitBoxShadow: return 212;
    case CSSPropertyWebkitColorCorrection: return 213;
    case CSSPropertyWebkitColumnAxis: return 214;
    case CSSPropertyWebkitColumnBreakAfter: return 215;
    case CSSPropertyWebkitColumnBreakBefore: return 216;
    case CSSPropertyWebkitColumnBreakInside: return 217;
    case CSSPropertyWebkitColumnCount: return 218;
    case CSSPropertyWebkitColumnGap: return 219;
    case CSSPropertyWebkitColumnProgression: return 220;
    case CSSPropertyWebkitColumnRule: return 221;
    case CSSPropertyWebkitColumnRuleColor: return 222;
    case CSSPropertyWebkitColumnRuleStyle: return 223;
    case CSSPropertyWebkitColumnRuleWidth: return 224;
    case CSSPropertyWebkitColumnSpan: return 225;
    case CSSPropertyWebkitColumnWidth: return 226;
    case CSSPropertyWebkitColumns: return 227;
#if defined(ENABLE_CSS_BOX_DECORATION_BREAK) && ENABLE_CSS_BOX_DECORATION_BREAK
    case CSSPropertyWebkitBoxDecorationBreak: return 228;
#endif
#if defined(ENABLE_CSS_FILTERS) && ENABLE_CSS_FILTERS
    case CSSPropertyWebkitFilter: return 229;
#endif
    case CSSPropertyWebkitAlignContent: return 230;
    case CSSPropertyWebkitAlignItems: return 231;
    case CSSPropertyWebkitAlignSelf: return 232;
    case CSSPropertyWebkitFlex: return 233;
    case CSSPropertyWebkitFlexBasis: return 234;
    case CSSPropertyWebkitFlexDirection: return 235;
    case CSSPropertyWebkitFlexFlow: return 236;
    case CSSPropertyWebkitFlexGrow: return 237;
    case CSSPropertyWebkitFlexShrink: return 238;
    case CSSPropertyWebkitFlexWrap: return 239;
    case CSSPropertyWebkitJustifyContent: return 240;
    case CSSPropertyWebkitFontSizeDelta: return 241;
    case CSSPropertyWebkitGridColumns: return 242;
    case CSSPropertyWebkitGridRows: return 243;
    case CSSPropertyWebkitGridStart: return 244;
    case CSSPropertyWebkitGridEnd: return 245;
    case CSSPropertyWebkitGridBefore: return 246;
    case CSSPropertyWebkitGridAfter: return 247;
    case CSSPropertyWebkitGridColumn: return 248;
    case CSSPropertyWebkitGridRow: return 249;
    case CSSPropertyWebkitGridAutoFlow: return 250;
    case CSSPropertyWebkitHighlight: return 251;
    case CSSPropertyWebkitHyphenateCharacter: return 252;
    case CSSPropertyWebkitHyphenateLimitAfter: return 253;
    case CSSPropertyWebkitHyphenateLimitBefore: return 254;
    case CSSPropertyWebkitHyphenateLimitLines: return 255;
    case CSSPropertyWebkitHyphens: return 256;
    case CSSPropertyWebkitLineBoxContain: return 257;
    case CSSPropertyWebkitLineAlign: return 258;
    case CSSPropertyWebkitLineBreak: return 259;
    case CSSPropertyWebkitLineClamp: return 260;
    case CSSPropertyWebkitLineGrid: return 261;
    case CSSPropertyWebkitLineSnap: return 262;
    case CSSPropertyWebkitLogicalWidth: return 263;
    case CSSPropertyWebkitLogicalHeight: return 264;
    case CSSPropertyWebkitMarginAfterCollapse: return 265;
    case CSSPropertyWebkitMarginBeforeCollapse: return 266;
    case CSSPropertyWebkitMarginBottomCollapse: return 267;
    case CSSPropertyWebkitMarginTopCollapse: return 268;
    case CSSPropertyWebkitMarginCollapse: return 269;
    case CSSPropertyWebkitMarginAfter: return 270;
    case CSSPropertyWebkitMarginBefore: return 271;
    case CSSPropertyWebkitMarginEnd: return 272;
    case CSSPropertyWebkitMarginStart: return 273;
    case CSSPropertyWebkitMarquee: return 274;
    case CSSPropertyWebkitMarqueeDirection: return 275;
    case CSSPropertyWebkitMarqueeIncrement: return 276;
    case CSSPropertyWebkitMarqueeRepetition: return 277;
    case CSSPropertyWebkitMarqueeSpeed: return 278;
    case CSSPropertyWebkitMarqueeStyle: return 279;
    case CSSPropertyWebkitMask: return 280;
    case CSSPropertyWebkitMaskBoxImage: return 281;
    case CSSPropertyWebkitMaskBoxImageOutset: return 282;
    case CSSPropertyWebkitMaskBoxImageRepeat: return 283;
    case CSSPropertyWebkitMaskBoxImageSlice: return 284;
    case CSSPropertyWebkitMaskBoxImageSource: return 285;
    case CSSPropertyWebkitMaskBoxImageWidth: return 286;
    case CSSPropertyWebkitMaskClip: return 287;
    case CSSPropertyWebkitMaskComposite: return 288;
    case CSSPropertyWebkitMaskImage: return 289;
    case CSSPropertyWebkitMaskOrigin: return 290;
    case CSSPropertyWebkitMaskPosition: return 291;
    case CSSPropertyWebkitMaskPositionX: return 292;
    case CSSPropertyWebkitMaskPositionY: return 293;
    case CSSPropertyWebkitMaskRepeat: return 294;
    case CSSPropertyWebkitMaskRepeatX: return 295;
    case CSSPropertyWebkitMaskRepeatY: return 296;
    case CSSPropertyWebkitMaskSize: return 297;
    case CSSPropertyWebkitMaxLogicalWidth: return 298;
    case CSSPropertyWebkitMaxLogicalHeight: return 299;
    case CSSPropertyWebkitMinLogicalWidth: return 300;
    case CSSPropertyWebkitMinLogicalHeight: return 301;
    // WebkitNbspMode has been deleted, was return 302;
    case CSSPropertyWebkitOrder: return 303;
    case CSSPropertyWebkitPaddingAfter: return 304;
    case CSSPropertyWebkitPaddingBefore: return 305;
    case CSSPropertyWebkitPaddingEnd: return 306;
    case CSSPropertyWebkitPaddingStart: return 307;
    case CSSPropertyWebkitPerspective: return 308;
    case CSSPropertyWebkitPerspectiveOrigin: return 309;
    case CSSPropertyWebkitPerspectiveOriginX: return 310;
    case CSSPropertyWebkitPerspectiveOriginY: return 311;
    case CSSPropertyWebkitPrintColorAdjust: return 312;
    case CSSPropertyWebkitRtlOrdering: return 313;
    case CSSPropertyWebkitRubyPosition: return 314;
    case CSSPropertyWebkitTextCombine: return 315;
    case CSSPropertyWebkitTextDecorationsInEffect: return 316;
    case CSSPropertyWebkitTextEmphasis: return 317;
    case CSSPropertyWebkitTextEmphasisColor: return 318;
    case CSSPropertyWebkitTextEmphasisPosition: return 319;
    case CSSPropertyWebkitTextEmphasisStyle: return 320;
    case CSSPropertyWebkitTextFillColor: return 321;
    case CSSPropertyWebkitTextSecurity: return 322;
    case CSSPropertyWebkitTextStroke: return 323;
    case CSSPropertyWebkitTextStrokeColor: return 324;
    case CSSPropertyWebkitTextStrokeWidth: return 325;
    case CSSPropertyWebkitTransform: return 326;
    case CSSPropertyWebkitTransformOrigin: return 327;
    case CSSPropertyWebkitTransformOriginX: return 328;
    case CSSPropertyWebkitTransformOriginY: return 329;
    case CSSPropertyWebkitTransformOriginZ: return 330;
    case CSSPropertyWebkitTransformStyle: return 331;
    case CSSPropertyWebkitTransition: return 332;
    case CSSPropertyWebkitTransitionDelay: return 333;
    case CSSPropertyWebkitTransitionDuration: return 334;
    case CSSPropertyWebkitTransitionProperty: return 335;
    case CSSPropertyWebkitTransitionTimingFunction: return 336;
    case CSSPropertyWebkitUserDrag: return 337;
    case CSSPropertyWebkitUserModify: return 338;
    case CSSPropertyWebkitUserSelect: return 339;
    case CSSPropertyWebkitFlowInto: return 340;
    case CSSPropertyWebkitFlowFrom: return 341;
    case CSSPropertyWebkitRegionOverflow: return 342;
    case CSSPropertyWebkitRegionBreakAfter: return 343;
    case CSSPropertyWebkitRegionBreakBefore: return 344;
    case CSSPropertyWebkitRegionBreakInside: return 345;
    case CSSPropertyWebkitShapeInside: return 346;
    case CSSPropertyWebkitShapeOutside: return 347;
    case CSSPropertyWebkitShapeMargin: return 348;
    case CSSPropertyWebkitShapePadding: return 349;
    case CSSPropertyWebkitWrapFlow: return 350;
    case CSSPropertyWebkitWrapThrough: return 351;
    // CSSPropertyWebkitWrap was 352.
#if defined(ENABLE_TOUCH_EVENTS) && ENABLE_TOUCH_EVENTS
    case CSSPropertyWebkitTapHighlightColor: return 353;
#endif
#if defined(ENABLE_DRAGGABLE_REGION) && ENABLE_DRAGGABLE_REGION
    case CSSPropertyWebkitAppRegion: return 354;
#endif
    case CSSPropertyClipPath: return 355;
    case CSSPropertyClipRule: return 356;
    case CSSPropertyMask: return 357;
    case CSSPropertyEnableBackground: return 358;
    case CSSPropertyFilter: return 359;
    case CSSPropertyFloodColor: return 360;
    case CSSPropertyFloodOpacity: return 361;
    case CSSPropertyLightingColor: return 362;
    case CSSPropertyStopColor: return 363;
    case CSSPropertyStopOpacity: return 364;
    case CSSPropertyColorInterpolation: return 365;
    case CSSPropertyColorInterpolationFilters: return 366;
    case CSSPropertyColorProfile: return 367;
    case CSSPropertyColorRendering: return 368;
    case CSSPropertyFill: return 369;
    case CSSPropertyFillOpacity: return 370;
    case CSSPropertyFillRule: return 371;
    case CSSPropertyMarker: return 372;
    case CSSPropertyMarkerEnd: return 373;
    case CSSPropertyMarkerMid: return 374;
    case CSSPropertyMarkerStart: return 375;
    case CSSPropertyMaskType: return 376;
    case CSSPropertyShapeRendering: return 377;
    case CSSPropertyStroke: return 378;
    case CSSPropertyStrokeDasharray: return 379;
    case CSSPropertyStrokeDashoffset: return 380;
    case CSSPropertyStrokeLinecap: return 381;
    case CSSPropertyStrokeLinejoin: return 382;
    case CSSPropertyStrokeMiterlimit: return 383;
    case CSSPropertyStrokeOpacity: return 384;
    case CSSPropertyStrokeWidth: return 385;
    case CSSPropertyAlignmentBaseline: return 386;
    case CSSPropertyBaselineShift: return 387;
    case CSSPropertyDominantBaseline: return 388;
    case CSSPropertyGlyphOrientationHorizontal: return 389;
    case CSSPropertyGlyphOrientationVertical: return 390;
    case CSSPropertyKerning: return 391;
    case CSSPropertyTextAnchor: return 392;
    case CSSPropertyVectorEffect: return 393;
    case CSSPropertyWritingMode: return 394;
    case CSSPropertyWebkitSvgShadow: return 395;
#if defined(ENABLE_CURSOR_VISIBILITY) && ENABLE_CURSOR_VISIBILITY
    case CSSPropertyWebkitCursorVisibility: return 396;
#endif
    // CSSPropertyImageOrientation has been removed, was return 397;
    // CSSPropertyImageResolution has been removed, was return 398;
#if defined(ENABLE_CSS_COMPOSITING) && ENABLE_CSS_COMPOSITING
    case CSSPropertyWebkitBlendMode: return 399;
    case CSSPropertyWebkitBackgroundBlendMode: return 400;
#endif
#if defined(ENABLE_CSS3_TEXT) && ENABLE_CSS3_TEXT
    case CSSPropertyWebkitTextDecorationLine: return 401;
    case CSSPropertyWebkitTextDecorationStyle: return 402;
    case CSSPropertyWebkitTextDecorationColor: return 403;
    case CSSPropertyWebkitTextAlignLast: return 404;
    case CSSPropertyWebkitTextUnderlinePosition: return 405;
#endif
#if defined(ENABLE_CSS_DEVICE_ADAPTATION) && ENABLE_CSS_DEVICE_ADAPTATION
    case CSSPropertyMaxZoom: return 406;
    case CSSPropertyMinZoom: return 407;
    case CSSPropertyOrientation: return 408;
    case CSSPropertyUserZoom: return 409;
#endif
#if defined(ENABLE_DASHBOARD_SUPPORT) && ENABLE_DASHBOARD_SUPPORT
    case CSSPropertyWebkitDashboardRegion: return 410;
#endif
#if defined(ENABLE_ACCELERATED_OVERFLOW_SCROLLING) && ENABLE_ACCELERATED_OVERFLOW_SCROLLING
    case CSSPropertyWebkitOverflowScrolling: return 411;
#endif
    case CSSPropertyWebkitAppRegion: return 412;
    case CSSPropertyWebkitFilter: return 413;
    case CSSPropertyWebkitBoxDecorationBreak: return 414;
    case CSSPropertyWebkitTapHighlightColor: return 415;
    case CSSPropertyBufferedRendering: return 416;
    case CSSPropertyWebkitGridAutoRows: return 417;
    case CSSPropertyWebkitGridAutoColumns: return 418;
    case CSSPropertyBackgroundBlendMode: return 419;
    case CSSPropertyMixBlendMode: return 420;
    case CSSPropertyTouchAction: return 421;

    // Add new features above this line (don't change the assigned numbers of the existing
    // items) and update maximumCSSSampleId() with the new maximum value.

    case CSSPropertyInvalid:
    case CSSPropertyVariable:
        ASSERT_NOT_REACHED();
        return 0;
    }

    ASSERT_NOT_REACHED();
    return 0;
}

static int maximumCSSSampleId() { return 421; }

UseCounter::UseCounter()
{
    m_CSSFeatureBits.ensureSize(lastCSSProperty + 1);
    m_CSSFeatureBits.clearAll();
}

UseCounter::~UseCounter()
{
    // We always log PageDestruction so that we have a scale for the rest of the features.
    HistogramSupport::histogramEnumeration("WebCore.FeatureObserver", PageDestruction, NumberOfFeatures);

    updateMeasurements();
}

void UseCounter::updateMeasurements()
{
    HistogramSupport::histogramEnumeration("WebCore.FeatureObserver", PageVisits, NumberOfFeatures);

    if (m_countBits) {
        for (unsigned i = 0; i < NumberOfFeatures; ++i) {
            if (m_countBits->quickGet(i))
                HistogramSupport::histogramEnumeration("WebCore.FeatureObserver", i, NumberOfFeatures);
        }
        // Clearing count bits is timing sensitive.
        m_countBits->clearAll();
    }

    // FIXME: Sometimes this function is called more than once per page. The following
    //        bool guards against incrementing the page count when there are no CSS
    //        bits set. http://crbug.com/236262.
    bool needsPagesMeasuredUpdate = false;
    for (int i = firstCSSProperty; i <= lastCSSProperty; ++i) {
        if (m_CSSFeatureBits.quickGet(i)) {
            int cssSampleId = mapCSSPropertyIdToCSSSampleId(i);
            HistogramSupport::histogramEnumeration("WebCore.FeatureObserver.CSSProperties", cssSampleId, maximumCSSSampleId());
            needsPagesMeasuredUpdate = true;
        }
    }

    if (needsPagesMeasuredUpdate)
        HistogramSupport::histogramEnumeration("WebCore.FeatureObserver.CSSProperties", totalPagesMeasuredCSSSampleId(), maximumCSSSampleId());

    m_CSSFeatureBits.clearAll();
}

void UseCounter::didCommitLoad()
{
    updateMeasurements();
}

void UseCounter::count(Document* document, Feature feature)
{
    if (!document)
        return;

    Page* page = document->page();
    if (!page)
        return;

    ASSERT(page->useCounter()->deprecationMessage(feature).isEmpty());
    page->useCounter()->recordMeasurement(feature);
}

void UseCounter::count(DOMWindow* domWindow, Feature feature)
{
    ASSERT(domWindow);
    count(domWindow->document(), feature);
}

void UseCounter::countDeprecation(ScriptExecutionContext* context, Feature feature)
{
    if (!context || !context->isDocument())
        return;
    UseCounter::countDeprecation(toDocument(context), feature);
}

void UseCounter::countDeprecation(DOMWindow* window, Feature feature)
{
    if (!window)
        return;
    UseCounter::countDeprecation(window->document(), feature);
}

void UseCounter::countDeprecation(Document* document, Feature feature)
{
    if (!document)
        return;

    Page* page = document->page();
    if (!page)
        return;

    if (page->useCounter()->recordMeasurement(feature)) {
        ASSERT(!page->useCounter()->deprecationMessage(feature).isEmpty());
        page->console()->addMessage(DeprecationMessageSource, WarningMessageLevel, page->useCounter()->deprecationMessage(feature));
    }
}

String UseCounter::deprecationMessage(Feature feature)
{
    switch (feature) {
    // Content Security Policy
    case PrefixedContentSecurityPolicy:
    case PrefixedContentSecurityPolicyReportOnly:
        return "The 'X-WebKit-CSP' headers are deprecated; please consider using the canonical 'Content-Security-Policy' header instead.";

    // HTMLMediaElement
    case PrefixedMediaGenerateKeyRequest:
        return "'HTMLMediaElement.webkitGenerateKeyRequest()' is deprecated. Please use 'MediaKeys.createSession()' instead.";

    // Quota
    case StorageInfo:
        return "'window.webkitStorageInfo' is deprecated. Please use 'navigator.webkitTemporaryStorage' or 'navigator.webkitPersistentStorage' instead.";

    // Performance
    case PrefixedPerformanceTimeline:
        return "'window.performance.webkitGet*' methods have been deprecated. Please use the unprefixed 'performance.get*' methods instead.";
    case PrefixedUserTiming:
        return "'window.performance.webkit*' methods have been deprecated. Please use the unprefixed 'window.performance.*' methods instead.";

    // Web Audio
    case WebAudioLooping:
        return "AudioBufferSourceNode 'looping' attribute is deprecated.  Use 'loop' instead.";

    case DocumentClear:
        return "document.clear() is deprecated. This method doesn't do anything.";

    case PrefixedTransitionMediaFeature:
        return "The '(-webkit-transition)' media query feature is deprecated; please consider using the more exact conditional \"@supports('(transition-property: prop_name)')\" instead.";

    // Features that aren't deprecated don't have a deprecation message.
    default:
        return String();
    }
}

void UseCounter::count(CSSPropertyID feature)
{
    ASSERT(feature >= firstCSSProperty);
    ASSERT(feature <= lastCSSProperty);
    m_CSSFeatureBits.quickSet(feature);
}

UseCounter* UseCounter::getFrom(const Document* document)
{
    if (document && document->page())
        return document->page()->useCounter();
    return 0;
}

UseCounter* UseCounter::getFrom(const CSSStyleSheet* sheet)
{
    if (sheet)
        return getFrom(sheet->contents());
    return 0;
}

UseCounter* UseCounter::getFrom(const StyleSheetContents* sheetContents)
{
    // FIXME: We may want to handle stylesheets that have multiple owners
    //        http://crbug.com/242125
    if (sheetContents && !sheetContents->isUserStyleSheet() && sheetContents->hasSingleOwnerNode())
        return getFrom(sheetContents->singleOwnerDocument());
    return 0;
}

} // namespace WebCore
