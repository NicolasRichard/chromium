// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/tracing/tracing_ui.h"

#include <set>
#include <string>
#include <vector>

#include "base/base64.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/command_line.h"
#include "base/format_macros.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/trace_event/trace_event.h"
#include "base/values.h"
#include "content/browser/tracing/grit/tracing_resources.h"
#include "content/browser/tracing/tracing_controller_impl.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/trace_uploader.h"
#include "content/public/browser/tracing_controller.h"
#include "content/public/browser/tracing_delegate.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/public/common/content_client.h"
#include "content/public/common/url_constants.h"

namespace content {
namespace {

void OnGotCategories(const WebUIDataSource::GotDataCallback& callback,
                     const std::set<std::string>& categorySet) {
  base::ListValue category_list;
  for (std::set<std::string>::const_iterator it = categorySet.begin();
       it != categorySet.end(); it++) {
    category_list.AppendString(*it);
  }

  base::RefCountedString* res = new base::RefCountedString();
  base::JSONWriter::Write(category_list, &res->data());
  callback.Run(res);
}

bool GetTracingOptions(const std::string& data64,
                       base::trace_event::TraceConfig* trace_config) {
  std::string data;
  if (!base::Base64Decode(data64, &data)) {
    LOG(ERROR) << "Options were not base64 encoded.";
    return false;
  }

  scoped_ptr<base::Value> optionsRaw = base::JSONReader::Read(data);
  if (!optionsRaw) {
    LOG(ERROR) << "Options were not valid JSON";
    return false;
  }
  base::DictionaryValue* options;
  if (!optionsRaw->GetAsDictionary(&options)) {
    LOG(ERROR) << "Options must be dict";
    return false;
  }

  if (!trace_config) {
    LOG(ERROR) << "trace_config can't be passed as NULL";
    return false;
  }

  bool options_ok = true;
  std::string category_filter_string;
  options_ok &= options->GetString("categoryFilter", &category_filter_string);

  std::string record_mode;
  options_ok &= options->GetString("tracingRecordMode", &record_mode);

  *trace_config = base::trace_event::TraceConfig(category_filter_string,
                                                 record_mode);

  bool enable_systrace;
  options_ok &= options->GetBoolean("useSystemTracing", &enable_systrace);
  if (enable_systrace)
    trace_config->EnableSystrace();

  bool enable_sampling;
  options_ok &= options->GetBoolean("useSampling", &enable_sampling);
  if (enable_sampling)
    trace_config->EnableSampling();

  if (!options_ok) {
    LOG(ERROR) << "Malformed options";
    return false;
  }
  return true;
}

void OnRecordingEnabledAck(const WebUIDataSource::GotDataCallback& callback);

bool BeginRecording(const std::string& data64,
                    const WebUIDataSource::GotDataCallback& callback) {
  base::trace_event::TraceConfig trace_config("", "");
  if (!GetTracingOptions(data64, &trace_config))
    return false;

  return TracingController::GetInstance()->EnableRecording(
      trace_config,
      base::Bind(&OnRecordingEnabledAck, callback));
}

void OnRecordingEnabledAck(const WebUIDataSource::GotDataCallback& callback) {
  base::RefCountedString* res = new base::RefCountedString();
  callback.Run(res);
}

void OnTraceBufferUsageResult(const WebUIDataSource::GotDataCallback& callback,
                              float percent_full,
                              size_t approximate_event_count) {
  std::string str = base::DoubleToString(percent_full);
  callback.Run(base::RefCountedString::TakeString(&str));
}

void OnTraceBufferStatusResult(const WebUIDataSource::GotDataCallback& callback,
                               float percent_full,
                               size_t approximate_event_count) {
  base::DictionaryValue status;
  status.SetDouble("percentFull", percent_full);
  status.SetInteger("approximateEventCount", approximate_event_count);

  std::string status_json;
  base::JSONWriter::Write(status, &status_json);

  base::RefCountedString* status_base64 = new base::RefCountedString();
  base::Base64Encode(status_json, &status_base64->data());
  callback.Run(status_base64);
}

void OnMonitoringEnabledAck(const WebUIDataSource::GotDataCallback& callback);

bool EnableMonitoring(const std::string& data64,
                      const WebUIDataSource::GotDataCallback& callback) {
  base::trace_event::TraceConfig trace_config("", "");
  if (!GetTracingOptions(data64, &trace_config))
    return false;

  return TracingController::GetInstance()->EnableMonitoring(
      trace_config,
      base::Bind(OnMonitoringEnabledAck, callback));
}

void OnMonitoringEnabledAck(const WebUIDataSource::GotDataCallback& callback) {
  base::RefCountedString* res = new base::RefCountedString();
  callback.Run(res);
}

void OnMonitoringDisabled(const WebUIDataSource::GotDataCallback& callback) {
  base::RefCountedString* res = new base::RefCountedString();
  callback.Run(res);
}

void GetMonitoringStatus(const WebUIDataSource::GotDataCallback& callback) {
  bool is_monitoring;
  base::trace_event::TraceConfig config("", "");
  TracingController::GetInstance()->GetMonitoringStatus(
      &is_monitoring, &config);

  base::DictionaryValue monitoring_options;
  monitoring_options.SetBoolean("isMonitoring", is_monitoring);
  monitoring_options.SetString("categoryFilter",
                               config.ToCategoryFilterString());
  monitoring_options.SetBoolean("useSystemTracing", config.IsSystraceEnabled());
  monitoring_options.SetBoolean(
      "useContinuousTracing",
      config.GetTraceRecordMode() == base::trace_event::RECORD_CONTINUOUSLY);
  monitoring_options.SetBoolean("useSampling", config.IsSamplingEnabled());

  std::string monitoring_options_json;
  base::JSONWriter::Write(monitoring_options, &monitoring_options_json);

  base::RefCountedString* monitoring_options_base64 =
    new base::RefCountedString();
  base::Base64Encode(monitoring_options_json,
                     &monitoring_options_base64->data());
  callback.Run(monitoring_options_base64);
}

void TracingCallbackWrapperBase64(
    const WebUIDataSource::GotDataCallback& callback,
    base::RefCountedString* data) {
  base::RefCountedString* data_base64 = new base::RefCountedString();
  base::Base64Encode(data->data(), &data_base64->data());
  callback.Run(data_base64);
}

std::string GenerateMetadataJSON() {
  // Serialize metadata to json.
  scoped_ptr<base::DictionaryValue> metadata_dict(new base::DictionaryValue());
  metadata_dict->SetString("version", GetContentClient()->GetProduct());
  metadata_dict->SetString(
      "command_line",
      base::CommandLine::ForCurrentProcess()->GetCommandLineString());

  std::string results;
  if (base::JSONWriter::Write(*metadata_dict.get(), &results))
    return results;
  return std::string();
}

bool OnBeginJSONRequest(const std::string& path,
                        const WebUIDataSource::GotDataCallback& callback) {
  if (path == "json/categories") {
    return TracingController::GetInstance()->GetCategories(
        base::Bind(OnGotCategories, callback));
  }

  const char* beginRecordingPath = "json/begin_recording?";
  if (base::StartsWith(path, beginRecordingPath,
                       base::CompareCase::SENSITIVE)) {
    std::string data = path.substr(strlen(beginRecordingPath));
    return BeginRecording(data, callback);
  }
  if (path == "json/get_buffer_percent_full") {
    return TracingController::GetInstance()->GetTraceBufferUsage(
        base::Bind(OnTraceBufferUsageResult, callback));
  }
  if (path == "json/get_buffer_status") {
    return TracingController::GetInstance()->GetTraceBufferUsage(
        base::Bind(OnTraceBufferStatusResult, callback));
  }
  if (path == "json/end_recording_compressed") {
    scoped_refptr<TracingControllerImpl::TraceDataSink> data_sink =
        TracingController::CreateCompressedStringSink(
            TracingController::CreateCallbackEndpoint(
                base::Bind(TracingCallbackWrapperBase64, callback)));
    data_sink->SetMetadata(GenerateMetadataJSON());
    return TracingController::GetInstance()->DisableRecording(data_sink);
  }

  const char* enableMonitoringPath = "json/begin_monitoring?";
  if (path.find(enableMonitoringPath) == 0) {
    std::string data = path.substr(strlen(enableMonitoringPath));
    return EnableMonitoring(data, callback);
  }
  if (path == "json/end_monitoring") {
    return TracingController::GetInstance()->DisableMonitoring(
        base::Bind(OnMonitoringDisabled, callback));
  }
  if (path == "json/capture_monitoring_compressed") {
    scoped_refptr<TracingControllerImpl::TraceDataSink> data_sink =
        TracingController::CreateCompressedStringSink(
            TracingController::CreateCallbackEndpoint(
                base::Bind(TracingCallbackWrapperBase64, callback)));
    data_sink->SetMetadata(GenerateMetadataJSON());
    TracingController::GetInstance()->CaptureMonitoringSnapshot(data_sink);
    return true;
  }
  if (path == "json/get_monitoring_status") {
    GetMonitoringStatus(callback);
    return true;
  }

  LOG(ERROR) << "Unhandled request to " << path;
  return false;
}

bool OnTracingRequest(const std::string& path,
                      const WebUIDataSource::GotDataCallback& callback) {
  if (base::StartsWith(path, "json/", base::CompareCase::SENSITIVE)) {
    if (!OnBeginJSONRequest(path, callback)) {
      std::string error("##ERROR##");
      callback.Run(base::RefCountedString::TakeString(&error));
    }
    return true;
  }
  return false;
}

}  // namespace


////////////////////////////////////////////////////////////////////////////////
//
// TracingUI
//
////////////////////////////////////////////////////////////////////////////////

TracingUI::TracingUI(WebUI* web_ui)
    : WebUIController(web_ui),
      delegate_(GetContentClient()->browser()->GetTracingDelegate()),
      weak_factory_(this) {
  web_ui->RegisterMessageCallback(
      "doUpload",
      base::Bind(&TracingUI::DoUpload, base::Unretained(this)));
  web_ui->RegisterMessageCallback(
      "doUploadBase64",
      base::Bind(&TracingUI::DoUploadBase64Encoded, base::Unretained(this)));

  // Set up the chrome://tracing/ source.
  BrowserContext* browser_context =
      web_ui->GetWebContents()->GetBrowserContext();

  WebUIDataSource* source = WebUIDataSource::Create(kChromeUITracingHost);
  source->SetJsonPath("strings.js");
  source->SetDefaultResource(IDR_TRACING_HTML);
  source->AddResourcePath("tracing.js", IDR_TRACING_JS);
  source->SetRequestFilter(base::Bind(OnTracingRequest));
  WebUIDataSource::Add(browser_context, source);
  TracingControllerImpl::GetInstance()->RegisterTracingUI(this);
}

TracingUI::~TracingUI() {
  TracingControllerImpl::GetInstance()->UnregisterTracingUI(this);
}

void TracingUI::OnMonitoringStateChanged(bool is_monitoring) {
  web_ui()->CallJavascriptFunction(
      "onMonitoringStateChanged", base::FundamentalValue(is_monitoring));
}

void TracingUI::DoUploadBase64Encoded(const base::ListValue* args) {
  std::string file_contents_base64;
  if (!args || args->empty() || !args->GetString(0, &file_contents_base64)) {
    web_ui()->CallJavascriptFunction("onUploadError",
                                     base::StringValue("Missing data"));
    return;
  }

  std::string file_contents;
  base::Base64Decode(file_contents_base64, &file_contents);

  // doUploadBase64 is used to upload binary data which is assumed to already
  // be compressed.
  DoUploadInternal(file_contents, TraceUploader::UNCOMPRESSED_UPLOAD);
}

void TracingUI::DoUpload(const base::ListValue* args) {
  std::string file_contents;
  if (!args || args->empty() || !args->GetString(0, &file_contents)) {
    web_ui()->CallJavascriptFunction("onUploadError",
                                     base::StringValue("Missing data"));
    return;
  }

  DoUploadInternal(file_contents, TraceUploader::COMPRESSED_UPLOAD);
}

void TracingUI::DoUploadInternal(const std::string& file_contents,
                                 TraceUploader::UploadMode upload_mode) {
  if (!delegate_) {
    web_ui()->CallJavascriptFunction("onUploadError",
                                     base::StringValue("Not implemented"));
    return;
  }

  if (trace_uploader_) {
    web_ui()->CallJavascriptFunction("onUploadError",
                                     base::StringValue("Upload in progress"));
    return;
  }

  TraceUploader::UploadProgressCallback progress_callback =
      base::Bind(&TracingUI::OnTraceUploadProgress,
      weak_factory_.GetWeakPtr());
  TraceUploader::UploadDoneCallback done_callback =
      base::Bind(&TracingUI::OnTraceUploadComplete,
      weak_factory_.GetWeakPtr());

  trace_uploader_ = delegate_->GetTraceUploader(
      web_ui()->GetWebContents()->GetBrowserContext()->GetRequestContext());
  DCHECK(trace_uploader_);
  trace_uploader_->DoUpload(file_contents, upload_mode, nullptr,
                            progress_callback, done_callback);
  // TODO(mmandlis): Add support for stopping the upload in progress.
}

void TracingUI::OnTraceUploadProgress(int64 current, int64 total) {
  DCHECK(current <= total);
  int percent = (current / total) * 100;
  web_ui()->CallJavascriptFunction(
        "onUploadProgress",
        base::FundamentalValue(percent),
        base::StringValue(base::StringPrintf("%" PRId64, current)),
        base::StringValue(base::StringPrintf("%" PRId64, total)));
}

void TracingUI::OnTraceUploadComplete(bool success,
                                      const std::string& feedback) {
  if (success) {
    web_ui()->CallJavascriptFunction("onUploadComplete",
                                     base::StringValue(feedback));
  } else {
    web_ui()->CallJavascriptFunction("onUploadError",
                                     base::StringValue(feedback));
  }
  trace_uploader_.reset();
}

}  // namespace content
