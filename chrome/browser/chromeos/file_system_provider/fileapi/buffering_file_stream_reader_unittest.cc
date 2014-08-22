// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/run_loop.h"
#include "chrome/browser/chromeos/file_system_provider/fileapi/buffering_file_stream_reader.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chromeos {
namespace file_system_provider {
namespace {

// Size of the fake file in bytes.
const int kFileSize = 1024;

// Size of the preloading buffer in bytes.
const int kBufferSize = 8;

// Number of bytes requested per BufferingFileStreamReader::Read().
const int kChunkSize = 3;

// Pushes a value to the passed log vector.
template <typename T>
void LogValue(std::vector<T>* log, T value) {
  log->push_back(value);
}

// Fake internal file stream reader.
class FakeFileStreamReader : public storage::FileStreamReader {
 public:
  FakeFileStreamReader(std::vector<int>* log, net::Error return_error)
      : log_(log), return_error_(return_error) {}
  virtual ~FakeFileStreamReader() {}

  // storage::FileStreamReader overrides.
  virtual int Read(net::IOBuffer* buf,
                   int buf_len,
                   const net::CompletionCallback& callback) OVERRIDE {
    DCHECK(log_);
    log_->push_back(buf_len);

    if (return_error_ != net::OK) {
      base::MessageLoopProxy::current()->PostTask(
          FROM_HERE, base::Bind(callback, return_error_));
      return net::ERR_IO_PENDING;
    }

    const std::string fake_data('X', buf_len);
    memcpy(buf->data(), fake_data.c_str(), buf_len);

    base::MessageLoopProxy::current()->PostTask(FROM_HERE,
                                                base::Bind(callback, buf_len));
    return net::ERR_IO_PENDING;
  }

  virtual int64 GetLength(
      const net::Int64CompletionCallback& callback) OVERRIDE {
    DCHECK_EQ(net::OK, return_error_);
    base::MessageLoopProxy::current()->PostTask(
        FROM_HERE, base::Bind(callback, kFileSize));
    return net::ERR_IO_PENDING;
  }

 private:
  std::vector<int>* log_;  // Not owned.
  net::Error return_error_;
  DISALLOW_COPY_AND_ASSIGN(FakeFileStreamReader);
};

}  // namespace

class FileSystemProviderBufferingFileStreamReaderTest : public testing::Test {
 protected:
  FileSystemProviderBufferingFileStreamReaderTest() {}
  virtual ~FileSystemProviderBufferingFileStreamReaderTest() {}

  content::TestBrowserThreadBundle thread_bundle_;
};

TEST_F(FileSystemProviderBufferingFileStreamReaderTest, Read) {
  std::vector<int> inner_read_log;
  BufferingFileStreamReader reader(
      scoped_ptr<storage::FileStreamReader>(
          new FakeFileStreamReader(&inner_read_log, net::OK)),
      kBufferSize);

  // For the first read, the internal file stream reader is fired, as there is
  // no data in the preloading buffer.
  {
    scoped_refptr<net::IOBuffer> buffer(new net::IOBuffer(kChunkSize));
    std::vector<int> read_log;
    const int result =
        reader.Read(buffer, kChunkSize, base::Bind(&LogValue<int>, &read_log));
    base::RunLoop().RunUntilIdle();

    EXPECT_EQ(net::ERR_IO_PENDING, result);
    ASSERT_EQ(1u, inner_read_log.size());
    EXPECT_EQ(kBufferSize, inner_read_log[0]);
    ASSERT_EQ(1u, read_log.size());
    EXPECT_EQ(kChunkSize, read_log[0]);
  }

  // Second read should return data from the preloading buffer, without calling
  // the internal file stream reader.
  {
    inner_read_log.clear();
    scoped_refptr<net::IOBuffer> buffer(new net::IOBuffer(kChunkSize));
    std::vector<int> read_log;
    const int result =
        reader.Read(buffer, kChunkSize, base::Bind(&LogValue<int>, &read_log));
    base::RunLoop().RunUntilIdle();

    EXPECT_EQ(kChunkSize, result);
    EXPECT_EQ(0u, inner_read_log.size());
    // Results returned synchronously, so no new read result events.
    EXPECT_EQ(0u, read_log.size());
  }

  // Third read should return partial result from the preloading buffer. It is
  // valid to return less bytes than requested.
  {
    inner_read_log.clear();
    scoped_refptr<net::IOBuffer> buffer(new net::IOBuffer(kChunkSize));
    std::vector<int> read_log;
    const int result =
        reader.Read(buffer, kChunkSize, base::Bind(&LogValue<int>, &read_log));
    base::RunLoop().RunUntilIdle();

    EXPECT_EQ(kBufferSize - 2 * kChunkSize, result);
    EXPECT_EQ(0u, inner_read_log.size());
    // Results returned synchronously, so no new read result events.
    EXPECT_EQ(0u, read_log.size());
  }

  // The preloading buffer is now empty, so reading should invoke the internal
  // file stream reader.
  {
    inner_read_log.clear();
    scoped_refptr<net::IOBuffer> buffer(new net::IOBuffer(kChunkSize));
    std::vector<int> read_log;
    const int result =
        reader.Read(buffer, kChunkSize, base::Bind(&LogValue<int>, &read_log));
    base::RunLoop().RunUntilIdle();

    EXPECT_EQ(net::ERR_IO_PENDING, result);
    ASSERT_EQ(1u, inner_read_log.size());
    EXPECT_EQ(kBufferSize, inner_read_log[0]);
    ASSERT_EQ(1u, read_log.size());
    EXPECT_EQ(kChunkSize, read_log[0]);
  }
}

TEST_F(FileSystemProviderBufferingFileStreamReaderTest, Read_Directly) {
  std::vector<int> inner_read_log;
  BufferingFileStreamReader reader(
      scoped_ptr<storage::FileStreamReader>(
          new FakeFileStreamReader(&inner_read_log, net::OK)),
      kBufferSize);

  // First read couple of bytes, so the internal buffer is filled out.
  {
    scoped_refptr<net::IOBuffer> buffer(new net::IOBuffer(kChunkSize));
    std::vector<int> read_log;
    const int result =
        reader.Read(buffer, kChunkSize, base::Bind(&LogValue<int>, &read_log));
    base::RunLoop().RunUntilIdle();

    EXPECT_EQ(net::ERR_IO_PENDING, result);
    ASSERT_EQ(1u, inner_read_log.size());
    EXPECT_EQ(kBufferSize, inner_read_log[0]);
    ASSERT_EQ(1u, read_log.size());
    EXPECT_EQ(kChunkSize, read_log[0]);
  }

  const int read_bytes = kBufferSize * 2;
  ASSERT_GT(kFileSize, read_bytes);

  // Reading more than the internal buffer size would cause fetching only
  // as much as available in the internal buffer.
  {
    inner_read_log.clear();
    scoped_refptr<net::IOBuffer> buffer(new net::IOBuffer(read_bytes));
    std::vector<int> read_log;
    const int result =
        reader.Read(buffer, read_bytes, base::Bind(&LogValue<int>, &read_log));
    base::RunLoop().RunUntilIdle();

    EXPECT_EQ(kBufferSize - kChunkSize, result);
    EXPECT_EQ(0u, inner_read_log.size());
    EXPECT_EQ(0u, read_log.size());
  }

  // The internal buffer is clean. Fetching more than the internal buffer size
  // would cause fetching data directly from the inner reader, with skipping
  // the internal buffer.
  {
    inner_read_log.clear();
    scoped_refptr<net::IOBuffer> buffer(new net::IOBuffer(read_bytes));
    std::vector<int> read_log;
    const int result =
        reader.Read(buffer, read_bytes, base::Bind(&LogValue<int>, &read_log));
    base::RunLoop().RunUntilIdle();

    EXPECT_EQ(net::ERR_IO_PENDING, result);
    ASSERT_EQ(1u, inner_read_log.size());
    EXPECT_EQ(read_bytes, inner_read_log[0]);
    ASSERT_EQ(1u, read_log.size());
    EXPECT_EQ(read_bytes, read_log[0]);
  }
}

TEST_F(FileSystemProviderBufferingFileStreamReaderTest,
       Read_MoreThanBufferSize) {
  std::vector<int> inner_read_log;
  BufferingFileStreamReader reader(
      scoped_ptr<storage::FileStreamReader>(
          new FakeFileStreamReader(&inner_read_log, net::OK)),
      kBufferSize);
  // First read couple of bytes, so the internal buffer is filled out.
  {
    scoped_refptr<net::IOBuffer> buffer(new net::IOBuffer(kChunkSize));
    std::vector<int> read_log;
    const int result =
        reader.Read(buffer, kChunkSize, base::Bind(&LogValue<int>, &read_log));
    base::RunLoop().RunUntilIdle();

    EXPECT_EQ(net::ERR_IO_PENDING, result);
    ASSERT_EQ(1u, inner_read_log.size());
    EXPECT_EQ(kBufferSize, inner_read_log[0]);
    ASSERT_EQ(1u, read_log.size());
    EXPECT_EQ(kChunkSize, read_log[0]);
  }

  // Returning less than requested number of bytes is valid, and should not
  // fail.
  {
    inner_read_log.clear();
    const int chunk_size = 20;
    ASSERT_LT(kBufferSize, chunk_size);
    scoped_refptr<net::IOBuffer> buffer(new net::IOBuffer(chunk_size));
    std::vector<int> read_log;
    const int result =
        reader.Read(buffer, chunk_size, base::Bind(&LogValue<int>, &read_log));
    base::RunLoop().RunUntilIdle();

    EXPECT_EQ(5, result);
    EXPECT_EQ(0u, inner_read_log.size());
    EXPECT_EQ(0u, read_log.size());
  }
}

TEST_F(FileSystemProviderBufferingFileStreamReaderTest, Read_WithError) {
  std::vector<int> inner_read_log;
  BufferingFileStreamReader reader(
      scoped_ptr<storage::FileStreamReader>(
          new FakeFileStreamReader(&inner_read_log, net::ERR_ACCESS_DENIED)),
      kBufferSize);

  scoped_refptr<net::IOBuffer> buffer(new net::IOBuffer(kChunkSize));
  std::vector<int> read_log;
  const int result =
      reader.Read(buffer, kChunkSize, base::Bind(&LogValue<int>, &read_log));
  base::RunLoop().RunUntilIdle();

  EXPECT_EQ(net::ERR_IO_PENDING, result);
  ASSERT_EQ(1u, inner_read_log.size());
  EXPECT_EQ(kBufferSize, inner_read_log[0]);
  ASSERT_EQ(1u, read_log.size());
  EXPECT_EQ(net::ERR_ACCESS_DENIED, read_log[0]);
}

TEST_F(FileSystemProviderBufferingFileStreamReaderTest, GetLength) {
  BufferingFileStreamReader reader(scoped_ptr<storage::FileStreamReader>(
                                       new FakeFileStreamReader(NULL, net::OK)),
                                   kBufferSize);

  std::vector<int64> get_length_log;
  const int64 result =
      reader.GetLength(base::Bind(&LogValue<int64>, &get_length_log));
  base::RunLoop().RunUntilIdle();

  EXPECT_EQ(net::ERR_IO_PENDING, result);
  ASSERT_EQ(1u, get_length_log.size());
  EXPECT_EQ(kFileSize, get_length_log[0]);
}

}  // namespace file_system_provider
}  // namespace chromeos
