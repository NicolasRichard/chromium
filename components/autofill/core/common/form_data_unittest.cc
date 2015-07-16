// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/common/form_data.h"

#include "base/pickle.h"
#include "base/strings/utf_string_conversions.h"
#include "components/autofill/core/common/form_field_data.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace autofill {

namespace {

// This function serializes the form data into the pickle in version one format.
// It should always be possible to deserialize it using DeserializeFormData(),
// even when version changes. See kPickleVersion in form_data.cc.
void SerializeInVersion1Format(const FormData& form_data,
                               base::Pickle* pickle) {
  pickle->WriteInt(1);
  pickle->WriteString16(form_data.name);
  base::string16 method(base::ASCIIToUTF16("POST"));
  pickle->WriteString16(method);
  pickle->WriteString(form_data.origin.spec());
  pickle->WriteString(form_data.action.spec());
  pickle->WriteBool(true);  // Used to be |user_submitted|, which was removed.
  pickle->WriteInt(static_cast<int>(form_data.fields.size()));
  for (size_t i = 0; i < form_data.fields.size(); ++i) {
    SerializeFormFieldData(form_data.fields[i], pickle);
  }
}

void SerializeInVersion2Format(const FormData& form_data,
                               base::Pickle* pickle) {
  pickle->WriteInt(2);
  pickle->WriteString16(form_data.name);
  pickle->WriteString(form_data.origin.spec());
  pickle->WriteString(form_data.action.spec());
  pickle->WriteBool(true);  // Used to be |user_submitted|, which was removed.
  pickle->WriteInt(static_cast<int>(form_data.fields.size()));
  for (size_t i = 0; i < form_data.fields.size(); ++i) {
    SerializeFormFieldData(form_data.fields[i], pickle);
  }
}

void SerializeInVersion3Format(const FormData& form_data,
                               base::Pickle* pickle) {
  pickle->WriteInt(3);
  pickle->WriteString16(form_data.name);
  pickle->WriteString(form_data.origin.spec());
  pickle->WriteString(form_data.action.spec());
  pickle->WriteBool(true);  // Used to be |user_submitted|, which was removed.
  pickle->WriteInt(static_cast<int>(form_data.fields.size()));
  for (size_t i = 0; i < form_data.fields.size(); ++i) {
    SerializeFormFieldData(form_data.fields[i], pickle);
  }
  pickle->WriteBool(form_data.is_form_tag);
}

void SerializeInVersion4Format(const FormData& form_data,
                               base::Pickle* pickle) {
  pickle->WriteInt(4);
  pickle->WriteString16(form_data.name);
  pickle->WriteString(form_data.origin.spec());
  pickle->WriteString(form_data.action.spec());
  pickle->WriteInt(static_cast<int>(form_data.fields.size()));
  for (size_t i = 0; i < form_data.fields.size(); ++i) {
    SerializeFormFieldData(form_data.fields[i], pickle);
  }
  pickle->WriteBool(form_data.is_form_tag);
}

// This function serializes the form data into the pickle in incorrect format
// (no version number).
void SerializeIncorrectFormat(const FormData& form_data, base::Pickle* pickle) {
  pickle->WriteString16(form_data.name);
  pickle->WriteString(form_data.origin.spec());
  pickle->WriteString(form_data.action.spec());
  pickle->WriteBool(true);  // Used to be |user_submitted|, which was removed.
  pickle->WriteInt(static_cast<int>(form_data.fields.size()));
  for (size_t i = 0; i < form_data.fields.size(); ++i) {
    SerializeFormFieldData(form_data.fields[i], pickle);
  }
}

void FillInDummyFormData(FormData* data) {
  data->name = base::ASCIIToUTF16("name");
  data->origin = GURL("origin");
  data->action = GURL("action");
  data->is_form_tag = true;  // Default value.

  FormFieldData field_data;
  field_data.label = base::ASCIIToUTF16("label");
  field_data.name = base::ASCIIToUTF16("name");
  field_data.value = base::ASCIIToUTF16("value");
  field_data.form_control_type = "password";
  field_data.autocomplete_attribute = "off";
  field_data.max_length = 200;
  field_data.is_autofilled = true;
  field_data.is_checked = true;
  field_data.is_checkable = true;
  field_data.is_focusable = true;
  field_data.should_autocomplete = false;
  field_data.text_direction = base::i18n::RIGHT_TO_LEFT;
  field_data.option_values.push_back(base::ASCIIToUTF16("First"));
  field_data.option_values.push_back(base::ASCIIToUTF16("Second"));
  field_data.option_contents.push_back(base::ASCIIToUTF16("First"));
  field_data.option_contents.push_back(base::ASCIIToUTF16("Second"));

  data->fields.push_back(field_data);

  // Change a few fields.
  field_data.max_length = 150;
  field_data.option_values.push_back(base::ASCIIToUTF16("Third"));
  field_data.option_contents.push_back(base::ASCIIToUTF16("Third"));
  data->fields.push_back(field_data);
}

}  // namespace

TEST(FormDataTest, SerializeAndDeserialize) {
  FormData data;
  FillInDummyFormData(&data);

  base::Pickle pickle;
  SerializeFormData(data, &pickle);

  base::PickleIterator iter(pickle);
  FormData actual;
  EXPECT_TRUE(DeserializeFormData(&iter, &actual));

  EXPECT_TRUE(actual.SameFormAs(data));
}

TEST(FormDataTest, SerializeAndDeserializeInStrings) {
  FormData data;
  FillInDummyFormData(&data);
  data.is_form_tag = false;

  std::string serialized_data;
  SerializeFormDataToBase64String(data, &serialized_data);

  FormData actual;
  EXPECT_TRUE(DeserializeFormDataFromBase64String(serialized_data, &actual));

  EXPECT_TRUE(actual.SameFormAs(data));
}

TEST(FormDataTest, Serialize_v1_Deserialize_vCurrent) {
  FormData data;
  FillInDummyFormData(&data);

  base::Pickle pickle;
  SerializeInVersion1Format(data, &pickle);

  base::PickleIterator iter(pickle);
  FormData actual;
  EXPECT_TRUE(DeserializeFormData(&iter, &actual));

  EXPECT_TRUE(actual.SameFormAs(data));
}

TEST(FormDataTest, Serialize_v2_Deserialize_vCurrent) {
  FormData data;
  FillInDummyFormData(&data);

  base::Pickle pickle;
  SerializeInVersion2Format(data, &pickle);

  base::PickleIterator iter(pickle);
  FormData actual;
  EXPECT_TRUE(DeserializeFormData(&iter, &actual));

  EXPECT_TRUE(actual.SameFormAs(data));
}

TEST(FormDataTest, Serialize_v3_Deserialize_vCurrent) {
  FormData data;
  FillInDummyFormData(&data);

  base::Pickle pickle;
  SerializeInVersion3Format(data, &pickle);

  base::PickleIterator iter(pickle);
  FormData actual;
  EXPECT_TRUE(DeserializeFormData(&iter, &actual));

  EXPECT_TRUE(actual.SameFormAs(data));
}

TEST(FormDataTest, Serialize_v3_Deserialize_vCurrent_IsFormTagFalse) {
  FormData data;
  FillInDummyFormData(&data);
  data.is_form_tag = false;

  base::Pickle pickle;
  SerializeInVersion3Format(data, &pickle);

  base::PickleIterator iter(pickle);
  FormData actual;
  EXPECT_TRUE(DeserializeFormData(&iter, &actual));

  EXPECT_TRUE(actual.SameFormAs(data));
}

TEST(FormDataTest, Serialize_v4_Deserialize_vCurrent) {
  FormData data;
  FillInDummyFormData(&data);

  base::Pickle pickle;
  SerializeInVersion4Format(data, &pickle);

  base::PickleIterator iter(pickle);
  FormData actual;
  EXPECT_TRUE(DeserializeFormData(&iter, &actual));

  EXPECT_TRUE(actual.SameFormAs(data));
}

TEST(FormDataTest, SerializeIncorrectFormatAndDeserialize) {
  FormData data;
  FillInDummyFormData(&data);

  base::Pickle pickle;
  SerializeIncorrectFormat(data, &pickle);

  base::PickleIterator iter(pickle);
  FormData actual;
  EXPECT_FALSE(DeserializeFormData(&iter, &actual));

  FormData empty;
  EXPECT_TRUE(actual.SameFormAs(empty));
}

}  // namespace autofill
