// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/ssl/ssl_client_session_cache_openssl.h"

#include <openssl/ssl.h>

#include "base/strings/string_number_conversions.h"
#include "base/test/simple_test_clock.h"
#include "net/ssl/scoped_openssl_types.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace net {

// Test basic insertion and lookup operations.
TEST(SSLClientSessionCacheOpenSSLTest, Basic) {
  SSLClientSessionCacheOpenSSL::Config config;
  SSLClientSessionCacheOpenSSL cache(config);

  ScopedSSL_SESSION session1(SSL_SESSION_new());
  ScopedSSL_SESSION session2(SSL_SESSION_new());
  ScopedSSL_SESSION session3(SSL_SESSION_new());
  EXPECT_EQ(1, session1->references);
  EXPECT_EQ(1, session2->references);
  EXPECT_EQ(1, session3->references);

  EXPECT_EQ(nullptr, cache.Lookup("key1"));
  EXPECT_EQ(nullptr, cache.Lookup("key2"));
  EXPECT_EQ(0u, cache.size());

  cache.Insert("key1", session1.get());
  EXPECT_EQ(session1.get(), cache.Lookup("key1"));
  EXPECT_EQ(nullptr, cache.Lookup("key2"));
  EXPECT_EQ(1u, cache.size());

  cache.Insert("key2", session2.get());
  EXPECT_EQ(session1.get(), cache.Lookup("key1"));
  EXPECT_EQ(session2.get(), cache.Lookup("key2"));
  EXPECT_EQ(2u, cache.size());

  EXPECT_EQ(2, session1->references);
  EXPECT_EQ(2, session2->references);

  cache.Insert("key1", session3.get());
  EXPECT_EQ(session3.get(), cache.Lookup("key1"));
  EXPECT_EQ(session2.get(), cache.Lookup("key2"));
  EXPECT_EQ(2u, cache.size());

  EXPECT_EQ(1, session1->references);
  EXPECT_EQ(2, session2->references);
  EXPECT_EQ(2, session3->references);

  cache.Flush();
  EXPECT_EQ(nullptr, cache.Lookup("key1"));
  EXPECT_EQ(nullptr, cache.Lookup("key2"));
  EXPECT_EQ(nullptr, cache.Lookup("key3"));
  EXPECT_EQ(0u, cache.size());

  EXPECT_EQ(1, session1->references);
  EXPECT_EQ(1, session2->references);
  EXPECT_EQ(1, session3->references);
}

// Test that a session may be inserted at two different keys. This should never
// be necessary, but the API doesn't prohibit it.
TEST(SSLClientSessionCacheOpenSSLTest, DoubleInsert) {
  SSLClientSessionCacheOpenSSL::Config config;
  SSLClientSessionCacheOpenSSL cache(config);

  ScopedSSL_SESSION session(SSL_SESSION_new());
  EXPECT_EQ(1, session->references);

  EXPECT_EQ(nullptr, cache.Lookup("key1"));
  EXPECT_EQ(nullptr, cache.Lookup("key2"));
  EXPECT_EQ(0u, cache.size());

  cache.Insert("key1", session.get());
  EXPECT_EQ(session.get(), cache.Lookup("key1"));
  EXPECT_EQ(nullptr, cache.Lookup("key2"));
  EXPECT_EQ(1u, cache.size());

  EXPECT_EQ(2, session->references);

  cache.Insert("key2", session.get());
  EXPECT_EQ(session.get(), cache.Lookup("key1"));
  EXPECT_EQ(session.get(), cache.Lookup("key2"));
  EXPECT_EQ(2u, cache.size());

  EXPECT_EQ(3, session->references);

  cache.Flush();
  EXPECT_EQ(nullptr, cache.Lookup("key1"));
  EXPECT_EQ(nullptr, cache.Lookup("key2"));
  EXPECT_EQ(0u, cache.size());

  EXPECT_EQ(1, session->references);
}

// Tests that the session cache's size is correctly bounded.
TEST(SSLClientSessionCacheOpenSSLTest, MaxEntries) {
  SSLClientSessionCacheOpenSSL::Config config;
  config.max_entries = 3;
  SSLClientSessionCacheOpenSSL cache(config);

  ScopedSSL_SESSION session1(SSL_SESSION_new());
  ScopedSSL_SESSION session2(SSL_SESSION_new());
  ScopedSSL_SESSION session3(SSL_SESSION_new());
  ScopedSSL_SESSION session4(SSL_SESSION_new());

  // Insert three entries.
  cache.Insert("key1", session1.get());
  cache.Insert("key2", session2.get());
  cache.Insert("key3", session3.get());
  EXPECT_EQ(session1.get(), cache.Lookup("key1"));
  EXPECT_EQ(session2.get(), cache.Lookup("key2"));
  EXPECT_EQ(session3.get(), cache.Lookup("key3"));
  EXPECT_EQ(3u, cache.size());

  // On insertion of a fourth, the first is removed.
  cache.Insert("key4", session4.get());
  EXPECT_EQ(nullptr, cache.Lookup("key1"));
  EXPECT_EQ(session4.get(), cache.Lookup("key4"));
  EXPECT_EQ(session3.get(), cache.Lookup("key3"));
  EXPECT_EQ(session2.get(), cache.Lookup("key2"));
  EXPECT_EQ(3u, cache.size());

  // Despite being newest, the next to be removed is session4 as it was accessed
  // least. recently.
  cache.Insert("key1", session1.get());
  EXPECT_EQ(session1.get(), cache.Lookup("key1"));
  EXPECT_EQ(session2.get(), cache.Lookup("key2"));
  EXPECT_EQ(session3.get(), cache.Lookup("key3"));
  EXPECT_EQ(nullptr, cache.Lookup("key4"));
  EXPECT_EQ(3u, cache.size());
}

// Tests that session expiration works properly.
TEST(SSLClientSessionCacheOpenSSLTest, Expiration) {
  const size_t kNumEntries = 20;
  const size_t kExpirationCheckCount = 10;
  const base::TimeDelta kTimeout = base::TimeDelta::FromSeconds(1000);

  SSLClientSessionCacheOpenSSL::Config config;
  config.expiration_check_count = kExpirationCheckCount;
  config.timeout = kTimeout;
  SSLClientSessionCacheOpenSSL cache(config);
  base::SimpleTestClock* clock = new base::SimpleTestClock;
  cache.SetClockForTesting(make_scoped_ptr(clock));

  // Add |kNumEntries - 1| entries.
  for (size_t i = 0; i < kNumEntries - 1; i++) {
    ScopedSSL_SESSION session(SSL_SESSION_new());
    cache.Insert(base::SizeTToString(i), session.get());
  }
  EXPECT_EQ(kNumEntries - 1, cache.size());

  // Expire all the previous entries and insert one more entry.
  clock->Advance(kTimeout * 2);
  ScopedSSL_SESSION session(SSL_SESSION_new());
  cache.Insert("key", session.get());

  // All entries are still in the cache.
  EXPECT_EQ(kNumEntries, cache.size());

  // Perform one fewer lookup than needed to trigger the expiration check. This
  // shall not expire any session.
  for (size_t i = 0; i < kExpirationCheckCount - 1; i++)
    cache.Lookup("key");

  // All entries are still in the cache.
  EXPECT_EQ(kNumEntries, cache.size());

  // Perform one more lookup. This will expire all sessions but the last one.
  cache.Lookup("key");
  EXPECT_EQ(1u, cache.size());
  EXPECT_EQ(session.get(), cache.Lookup("key"));
  for (size_t i = 0; i < kNumEntries - 1; i++) {
    SCOPED_TRACE(i);
    EXPECT_EQ(nullptr, cache.Lookup(base::SizeTToString(i)));
  }
}

// Tests that Lookup performs an expiration check before returning a cached
// session.
TEST(SSLClientSessionCacheOpenSSLTest, LookupExpirationCheck) {
  // kExpirationCheckCount is set to a suitably large number so the automated
  // pruning never triggers.
  const size_t kExpirationCheckCount = 1000;
  const base::TimeDelta kTimeout = base::TimeDelta::FromSeconds(1000);

  SSLClientSessionCacheOpenSSL::Config config;
  config.expiration_check_count = kExpirationCheckCount;
  config.timeout = kTimeout;
  SSLClientSessionCacheOpenSSL cache(config);
  base::SimpleTestClock* clock = new base::SimpleTestClock;
  cache.SetClockForTesting(make_scoped_ptr(clock));

  // Insert an entry into the session cache.
  ScopedSSL_SESSION session(SSL_SESSION_new());
  cache.Insert("key", session.get());
  EXPECT_EQ(session.get(), cache.Lookup("key"));
  EXPECT_EQ(1u, cache.size());

  // Expire the session.
  clock->Advance(kTimeout * 2);

  // The entry has not been removed yet.
  EXPECT_EQ(1u, cache.size());

  // But it will not be returned on lookup and gets pruned at that point.
  EXPECT_EQ(nullptr, cache.Lookup("key"));
  EXPECT_EQ(0u, cache.size());

  // Sessions also are treated as expired if the clock rewinds.
  cache.Insert("key", session.get());
  EXPECT_EQ(session.get(), cache.Lookup("key"));
  EXPECT_EQ(1u, cache.size());

  clock->Advance(-kTimeout * 2);

  EXPECT_EQ(nullptr, cache.Lookup("key"));
  EXPECT_EQ(0u, cache.size());
}

}  // namespace net
