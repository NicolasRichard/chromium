// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/gtk/options/fonts_page_gtk.h"

#include "app/l10n_util.h"
#include "chrome/browser/gtk/options/options_layout_gtk.h"
#include "chrome/common/gtk_util.h"
#include "chrome/common/pref_names.h"
#include "grit/generated_resources.h"

namespace {

// Make a Gtk font name string from a font family name and pixel size.
std::string MakeFontName(std::wstring family_name, int pixel_size) {
  std::string fontname;
  // TODO(mattm): We can pass in the size in pixels (px), and the font button
  // actually honors it, but when you open the selector it interprets it as
  // points.  See crbug.com/17857
  SStringPrintf(&fontname, "%s %dpx", WideToUTF8(family_name).c_str(),
                pixel_size);
  return fontname;
}

}  // namespace

FontsPageGtk::FontsPageGtk(Profile* profile) : OptionsPageBase(profile) {
  Init();
}

FontsPageGtk::~FontsPageGtk() {
}

void FontsPageGtk::Init() {
  OptionsLayoutBuilderGtk options_builder;

  serif_font_button_ = gtk_font_button_new();
  gtk_font_button_set_use_font(GTK_FONT_BUTTON(serif_font_button_), TRUE);
  gtk_font_button_set_use_size(GTK_FONT_BUTTON(serif_font_button_), TRUE);
  g_signal_connect(serif_font_button_, "font-set", G_CALLBACK(OnSerifFontSet),
                   this);

  sans_font_button_ = gtk_font_button_new();
  gtk_font_button_set_use_font(GTK_FONT_BUTTON(sans_font_button_), TRUE);
  gtk_font_button_set_use_size(GTK_FONT_BUTTON(sans_font_button_), TRUE);
  g_signal_connect(sans_font_button_, "font-set", G_CALLBACK(OnSansFontSet),
                   this);

  fixed_font_button_ = gtk_font_button_new();
  gtk_font_button_set_use_font(GTK_FONT_BUTTON(fixed_font_button_), TRUE);
  gtk_font_button_set_use_size(GTK_FONT_BUTTON(fixed_font_button_), TRUE);
  g_signal_connect(fixed_font_button_, "font-set", G_CALLBACK(OnFixedFontSet),
                   this);

  GtkWidget* font_controls = gtk_util::CreateLabeledControlsGroup(NULL,
      l10n_util::GetStringUTF8(
          IDS_FONT_LANGUAGE_SETTING_FONT_SELECTOR_SERIF_LABEL).c_str(),
      serif_font_button_,
      l10n_util::GetStringUTF8(
        IDS_FONT_LANGUAGE_SETTING_FONT_SELECTOR_SANS_SERIF_LABEL).c_str(),
      sans_font_button_,
      l10n_util::GetStringUTF8(
        IDS_FONT_LANGUAGE_SETTING_FONT_SELECTOR_FIXED_WIDTH_LABEL).c_str(),
      fixed_font_button_,
      NULL);

  options_builder.AddOptionGroup(l10n_util::GetStringUTF8(
        IDS_FONT_LANGUAGE_SETTING_FONT_SUB_DIALOG_FONT_TITLE),
      font_controls, false);

  // TODO(mattm): default encoding
  options_builder.AddOptionGroup(l10n_util::GetStringUTF8(
        IDS_FONT_LANGUAGE_SETTING_FONT_SUB_DIALOG_ENCODING_TITLE),
      gtk_label_new("todo"), false);

  page_ = options_builder.get_page_widget();

  serif_name_.Init(prefs::kWebKitSerifFontFamily, profile()->GetPrefs(), this);
  sans_serif_name_.Init(prefs::kWebKitSansSerifFontFamily,
                        profile()->GetPrefs(), this);
  variable_width_size_.Init(prefs::kWebKitDefaultFontSize,
                            profile()->GetPrefs(), this);

  fixed_width_name_.Init(prefs::kWebKitFixedFontFamily, profile()->GetPrefs(),
                         this);
  fixed_width_size_.Init(prefs::kWebKitDefaultFixedFontSize,
                         profile()->GetPrefs(), this);

  default_encoding_.Init(prefs::kDefaultCharset, profile()->GetPrefs(), this);

  NotifyPrefChanged(NULL);
}

void FontsPageGtk::NotifyPrefChanged(const std::wstring* pref_name) {
  if (!pref_name || *pref_name == prefs::kWebKitSerifFontFamily ||
      *pref_name == prefs::kWebKitDefaultFontSize) {
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(serif_font_button_),
        MakeFontName(serif_name_.GetValue(),
          variable_width_size_.GetValue()).c_str());
  }
  if (!pref_name || *pref_name == prefs::kWebKitSansSerifFontFamily ||
      *pref_name == prefs::kWebKitDefaultFontSize) {
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(sans_font_button_),
        MakeFontName(sans_serif_name_.GetValue(),
          variable_width_size_.GetValue()).c_str());
  }
  if (!pref_name || *pref_name == prefs::kWebKitFixedFontFamily ||
      *pref_name == prefs::kWebKitDefaultFixedFontSize) {
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(fixed_font_button_),
        MakeFontName(fixed_width_name_.GetValue(),
          fixed_width_size_.GetValue()).c_str());
  }
  if (!pref_name || *pref_name == prefs::kDefaultCharset) {
    // TODO
  }
}

void FontsPageGtk::SetFontsFromButton(StringPrefMember* name_pref,
                        IntegerPrefMember* size_pref,
                        GtkFontButton* font_button) {
  PangoFontDescription* desc = pango_font_description_from_string(
      gtk_font_button_get_font_name(font_button));
  int size = pango_font_description_get_size(desc);
  name_pref->SetValue(UTF8ToWide(pango_font_description_get_family(desc)));
  size_pref->SetValue(size / PANGO_SCALE);
  pango_font_description_free(desc);
  // Reset the button font in px, since the chooser will have set it in points.
  // Also, both sans and serif share the same size so we need to update them
  // both.
  NotifyPrefChanged(NULL);
}


// static
void FontsPageGtk::OnSerifFontSet(GtkFontButton* font_button,
                                  FontsPageGtk* fonts_page) {
  fonts_page->SetFontsFromButton(&fonts_page->serif_name_,
                                 &fonts_page->variable_width_size_,
                                 font_button);
}

// static
void FontsPageGtk::OnSansFontSet(GtkFontButton* font_button,
                                 FontsPageGtk* fonts_page) {
  fonts_page->SetFontsFromButton(&fonts_page->sans_serif_name_,
                                 &fonts_page->variable_width_size_,
                                 font_button);
}

// static
void FontsPageGtk::OnFixedFontSet(GtkFontButton* font_button,
                                  FontsPageGtk* fonts_page) {
  fonts_page->SetFontsFromButton(&fonts_page->fixed_width_name_,
                                 &fonts_page->fixed_width_size_,
                                 font_button);
}
