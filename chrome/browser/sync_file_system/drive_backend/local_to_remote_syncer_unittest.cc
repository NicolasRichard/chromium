// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync_file_system/drive_backend/local_to_remote_syncer.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/scoped_temp_dir.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "base/thread_task_runner_handle.h"
#include "chrome/browser/drive/drive_api_util.h"
#include "chrome/browser/drive/drive_uploader.h"
#include "chrome/browser/drive/fake_drive_service.h"
#include "chrome/browser/sync_file_system/drive_backend/drive_backend_constants.h"
#include "chrome/browser/sync_file_system/drive_backend/drive_backend_test_util.h"
#include "chrome/browser/sync_file_system/drive_backend/fake_drive_service_helper.h"
#include "chrome/browser/sync_file_system/drive_backend/fake_drive_uploader.h"
#include "chrome/browser/sync_file_system/drive_backend/list_changes_task.h"
#include "chrome/browser/sync_file_system/drive_backend/metadata_database.h"
#include "chrome/browser/sync_file_system/drive_backend/metadata_database.pb.h"
#include "chrome/browser/sync_file_system/drive_backend/remote_to_local_syncer.h"
#include "chrome/browser/sync_file_system/drive_backend/sync_engine_context.h"
#include "chrome/browser/sync_file_system/drive_backend/sync_engine_initializer.h"
#include "chrome/browser/sync_file_system/drive_backend/sync_task_manager.h"
#include "chrome/browser/sync_file_system/drive_backend/sync_task_token.h"
#include "chrome/browser/sync_file_system/fake_remote_change_processor.h"
#include "chrome/browser/sync_file_system/sync_file_system_test_util.h"
#include "chrome/browser/sync_file_system/syncable_file_system_util.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "google_apis/drive/drive_api_parser.h"
#include "google_apis/drive/gdata_errorcode.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/leveldatabase/src/helpers/memenv/memenv.h"
#include "third_party/leveldatabase/src/include/leveldb/env.h"

namespace sync_file_system {
namespace drive_backend {

namespace {

storage::FileSystemURL URL(const GURL& origin, const std::string& path) {
  return CreateSyncableFileSystemURL(
      origin, base::FilePath::FromUTF8Unsafe(path));
}

const int kRetryLimit = 100;

}  // namespace

class LocalToRemoteSyncerTest : public testing::Test {
 public:
  LocalToRemoteSyncerTest()
      : thread_bundle_(content::TestBrowserThreadBundle::IO_MAINLOOP) {}
  virtual ~LocalToRemoteSyncerTest() {}

  virtual void SetUp() OVERRIDE {
    ASSERT_TRUE(database_dir_.CreateUniqueTempDir());
    in_memory_env_.reset(leveldb::NewMemEnv(leveldb::Env::Default()));

    scoped_ptr<FakeDriveServiceWrapper>
        fake_drive_service(new FakeDriveServiceWrapper);
    scoped_ptr<drive::DriveUploaderInterface>
        drive_uploader(new FakeDriveUploader(fake_drive_service.get()));
    fake_drive_helper_.reset(new FakeDriveServiceHelper(
        fake_drive_service.get(),
        drive_uploader.get(),
        kSyncRootFolderTitle));
    remote_change_processor_.reset(new FakeRemoteChangeProcessor);

    context_.reset(new SyncEngineContext(
        fake_drive_service.PassAs<drive::DriveServiceInterface>(),
        drive_uploader.Pass(),
        NULL,
        base::ThreadTaskRunnerHandle::Get(),
        base::ThreadTaskRunnerHandle::Get()));
    context_->SetRemoteChangeProcessor(remote_change_processor_.get());

    RegisterSyncableFileSystem();

    sync_task_manager_.reset(new SyncTaskManager(
        base::WeakPtr<SyncTaskManager::Client>(),
        10 /* maximum_background_task */,
        base::ThreadTaskRunnerHandle::Get()));
    sync_task_manager_->Initialize(SYNC_STATUS_OK);
  }

  virtual void TearDown() OVERRIDE {
    sync_task_manager_.reset();
    RevokeSyncableFileSystem();
    fake_drive_helper_.reset();
    context_.reset();
    base::RunLoop().RunUntilIdle();
  }

  void InitializeMetadataDatabase() {
    SyncEngineInitializer* initializer =
        new SyncEngineInitializer(context_.get(),
                                  database_dir_.path(),
                                  in_memory_env_.get());
    SyncStatusCode status = SYNC_STATUS_UNKNOWN;

    sync_task_manager_->ScheduleSyncTask(
        FROM_HERE,
        scoped_ptr<SyncTask>(initializer),
        SyncTaskManager::PRIORITY_MED,
        base::Bind(&LocalToRemoteSyncerTest::DidInitializeMetadataDatabase,
                   base::Unretained(this), initializer, &status));

    base::RunLoop().RunUntilIdle();
    EXPECT_EQ(SYNC_STATUS_OK, status);
  }

  void DidInitializeMetadataDatabase(SyncEngineInitializer* initializer,
                                     SyncStatusCode* status_out,
                                     SyncStatusCode status) {
    *status_out = status;
    context_->SetMetadataDatabase(initializer->PassMetadataDatabase());
  }

  void RegisterApp(const std::string& app_id,
                   const std::string& app_root_folder_id) {
    SyncStatusCode status = SYNC_STATUS_FAILED;
    context_->GetMetadataDatabase()->RegisterApp(
        app_id, app_root_folder_id, CreateResultReceiver(&status));
    base::RunLoop().RunUntilIdle();
    EXPECT_EQ(SYNC_STATUS_OK, status);
  }

  MetadataDatabase* GetMetadataDatabase() {
    return context_->GetMetadataDatabase();
  }

 protected:
  std::string CreateSyncRoot() {
    std::string sync_root_folder_id;
    EXPECT_EQ(google_apis::HTTP_CREATED,
              fake_drive_helper_->AddOrphanedFolder(
                  kSyncRootFolderTitle, &sync_root_folder_id));
    return sync_root_folder_id;
  }

  std::string CreateRemoteFolder(const std::string& parent_folder_id,
                                 const std::string& title) {
    std::string folder_id;
    EXPECT_EQ(google_apis::HTTP_CREATED,
              fake_drive_helper_->AddFolder(
                  parent_folder_id, title, &folder_id));
    return folder_id;
  }

  std::string CreateRemoteFile(const std::string& parent_folder_id,
                               const std::string& title,
                               const std::string& content) {
    std::string file_id;
    EXPECT_EQ(google_apis::HTTP_SUCCESS,
              fake_drive_helper_->AddFile(
                  parent_folder_id, title, content, &file_id));
    return file_id;
  }

  void DeleteResource(const std::string& file_id) {
    EXPECT_EQ(google_apis::HTTP_NO_CONTENT,
              fake_drive_helper_->DeleteResource(file_id));
  }

  SyncStatusCode RunLocalToRemoteSyncer(FileChange file_change,
                                        const storage::FileSystemURL& url) {
    SyncStatusCode status = SYNC_STATUS_UNKNOWN;
    base::FilePath local_path = base::FilePath::FromUTF8Unsafe("dummy");
    scoped_ptr<LocalToRemoteSyncer> syncer(new LocalToRemoteSyncer(
        context_.get(),
        SyncFileMetadata(file_change.file_type(), 0, base::Time()),
        file_change, local_path, url));
    syncer->RunPreflight(SyncTaskToken::CreateForTesting(
        CreateResultReceiver(&status)));
    base::RunLoop().RunUntilIdle();
    return status;
  }

  SyncStatusCode ListChanges() {
    SyncStatusCode status = SYNC_STATUS_UNKNOWN;
    sync_task_manager_->ScheduleSyncTask(
        FROM_HERE,
        scoped_ptr<SyncTask>(new ListChangesTask(context_.get())),
        SyncTaskManager::PRIORITY_MED,
        CreateResultReceiver(&status));
    base::RunLoop().RunUntilIdle();
    return status;
  }

  SyncStatusCode RunRemoteToLocalSyncer() {
    SyncStatusCode status = SYNC_STATUS_UNKNOWN;
    scoped_ptr<RemoteToLocalSyncer>
        syncer(new RemoteToLocalSyncer(context_.get()));
    syncer->RunPreflight(SyncTaskToken::CreateForTesting(
        CreateResultReceiver(&status)));
    base::RunLoop().RunUntilIdle();
    return status;
  }

  SyncStatusCode RunRemoteToLocalSyncerUntilIdle() {
    SyncStatusCode status;
    int retry_count = 0;
    do {
      if (retry_count++ > kRetryLimit)
        break;
      status = RunRemoteToLocalSyncer();
    } while (status == SYNC_STATUS_OK ||
             status == SYNC_STATUS_RETRY ||
             GetMetadataDatabase()->PromoteLowerPriorityTrackersToNormal());
    EXPECT_EQ(SYNC_STATUS_NO_CHANGE_TO_SYNC, status);
    return status;
  }

  ScopedVector<google_apis::ResourceEntry>
  GetResourceEntriesForParentAndTitle(const std::string& parent_folder_id,
                                      const std::string& title) {
    ScopedVector<google_apis::ResourceEntry> entries;
    EXPECT_EQ(google_apis::HTTP_SUCCESS,
              fake_drive_helper_->SearchByTitle(
                  parent_folder_id, title, &entries));
    return entries.Pass();
  }

  std::string GetFileIDForParentAndTitle(const std::string& parent_folder_id,
                                         const std::string& title) {
    ScopedVector<google_apis::ResourceEntry> entries =
        GetResourceEntriesForParentAndTitle(parent_folder_id, title);
    if (entries.size() != 1)
      return std::string();
    return entries[0]->resource_id();
  }

  void VerifyTitleUniqueness(
      const std::string& parent_folder_id,
      const std::string& title,
      google_apis::ResourceEntry::ResourceEntryKind kind) {
    ScopedVector<google_apis::ResourceEntry> entries;
    EXPECT_EQ(google_apis::HTTP_SUCCESS,
              fake_drive_helper_->SearchByTitle(
                  parent_folder_id, title, &entries));
    ASSERT_EQ(1u, entries.size());
    EXPECT_EQ(kind, entries[0]->kind());
  }

  void VerifyFileDeletion(const std::string& parent_folder_id,
                          const std::string& title) {
    ScopedVector<google_apis::ResourceEntry> entries;
    EXPECT_EQ(google_apis::HTTP_SUCCESS,
              fake_drive_helper_->SearchByTitle(
                  parent_folder_id, title, &entries));
    EXPECT_TRUE(entries.empty());
  }

 private:
  content::TestBrowserThreadBundle thread_bundle_;
  base::ScopedTempDir database_dir_;
  scoped_ptr<leveldb::Env> in_memory_env_;

  scoped_ptr<SyncEngineContext> context_;
  scoped_ptr<FakeDriveServiceHelper> fake_drive_helper_;
  scoped_ptr<FakeRemoteChangeProcessor> remote_change_processor_;
  scoped_ptr<SyncTaskManager> sync_task_manager_;

  DISALLOW_COPY_AND_ASSIGN(LocalToRemoteSyncerTest);
};

TEST_F(LocalToRemoteSyncerTest, CreateFile) {
  const GURL kOrigin("chrome-extension://example");
  const std::string sync_root = CreateSyncRoot();
  const std::string app_root = CreateRemoteFolder(sync_root, kOrigin.host());
  InitializeMetadataDatabase();
  RegisterApp(kOrigin.host(), app_root);

  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "file1")));
  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_DIRECTORY),
      URL(kOrigin, "folder")));
  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "folder/file2")));

  std::string folder_id = GetFileIDForParentAndTitle(app_root, "folder");
  ASSERT_FALSE(folder_id.empty());

  VerifyTitleUniqueness(
      app_root, "file1", google_apis::ResourceEntry::ENTRY_KIND_FILE);
  VerifyTitleUniqueness(
      app_root, "folder", google_apis::ResourceEntry::ENTRY_KIND_FOLDER);
  VerifyTitleUniqueness(
      folder_id, "file2", google_apis::ResourceEntry::ENTRY_KIND_FILE);
}

TEST_F(LocalToRemoteSyncerTest, CreateFileOnMissingPath) {
  const GURL kOrigin("chrome-extension://example");
  const std::string sync_root = CreateSyncRoot();
  const std::string app_root = CreateRemoteFolder(sync_root, kOrigin.host());
  InitializeMetadataDatabase();
  RegisterApp(kOrigin.host(), app_root);

  // Run the syncer 3 times to create missing folder1 and folder2.
  EXPECT_EQ(SYNC_STATUS_RETRY, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "folder1/folder2/file")));
  EXPECT_EQ(SYNC_STATUS_RETRY, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "folder1/folder2/file")));
  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "folder1/folder2/file")));

  std::string folder_id1 = GetFileIDForParentAndTitle(app_root, "folder1");
  ASSERT_FALSE(folder_id1.empty());
  std::string folder_id2 = GetFileIDForParentAndTitle(folder_id1, "folder2");
  ASSERT_FALSE(folder_id2.empty());

  VerifyTitleUniqueness(
      app_root, "folder1", google_apis::ResourceEntry::ENTRY_KIND_FOLDER);
  VerifyTitleUniqueness(
      folder_id1, "folder2", google_apis::ResourceEntry::ENTRY_KIND_FOLDER);
  VerifyTitleUniqueness(
      folder_id2, "file", google_apis::ResourceEntry::ENTRY_KIND_FILE);
}

TEST_F(LocalToRemoteSyncerTest, DeleteFile) {
  const GURL kOrigin("chrome-extension://example");
  const std::string sync_root = CreateSyncRoot();
  const std::string app_root = CreateRemoteFolder(sync_root, kOrigin.host());
  InitializeMetadataDatabase();
  RegisterApp(kOrigin.host(), app_root);

  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "file")));
  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_DIRECTORY),
      URL(kOrigin, "folder")));

  VerifyTitleUniqueness(
      app_root, "file", google_apis::ResourceEntry::ENTRY_KIND_FILE);
  VerifyTitleUniqueness(
      app_root, "folder", google_apis::ResourceEntry::ENTRY_KIND_FOLDER);

  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_DELETE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "file")));
  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_DELETE,
                 SYNC_FILE_TYPE_DIRECTORY),
      URL(kOrigin, "folder")));

  VerifyFileDeletion(app_root, "file");
  VerifyFileDeletion(app_root, "folder");
}

TEST_F(LocalToRemoteSyncerTest, Conflict_CreateFileOnFolder) {
  const GURL kOrigin("chrome-extension://example");
  const std::string sync_root = CreateSyncRoot();
  const std::string app_root = CreateRemoteFolder(sync_root, kOrigin.host());
  InitializeMetadataDatabase();
  RegisterApp(kOrigin.host(), app_root);

  CreateRemoteFolder(app_root, "foo");
  EXPECT_EQ(SYNC_STATUS_OK, ListChanges());
  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "foo")));

  // There should exist both file and folder on remote.
  ScopedVector<google_apis::ResourceEntry> entries =
      GetResourceEntriesForParentAndTitle(app_root, "foo");
  ASSERT_EQ(2u, entries.size());
  EXPECT_EQ(google_apis::ResourceEntry::ENTRY_KIND_FOLDER, entries[0]->kind());
  EXPECT_EQ(google_apis::ResourceEntry::ENTRY_KIND_FILE, entries[1]->kind());
}

TEST_F(LocalToRemoteSyncerTest, Conflict_CreateFolderOnFile) {
  const GURL kOrigin("chrome-extension://example");
  const std::string sync_root = CreateSyncRoot();
  const std::string app_root = CreateRemoteFolder(sync_root, kOrigin.host());
  InitializeMetadataDatabase();
  RegisterApp(kOrigin.host(), app_root);

  CreateRemoteFile(app_root, "foo", "data");
  EXPECT_EQ(SYNC_STATUS_OK, ListChanges());

  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_DIRECTORY),
      URL(kOrigin, "foo")));

  // There should exist both file and folder on remote.
  ScopedVector<google_apis::ResourceEntry> entries =
      GetResourceEntriesForParentAndTitle(app_root, "foo");
  ASSERT_EQ(2u, entries.size());
  EXPECT_EQ(google_apis::ResourceEntry::ENTRY_KIND_FILE, entries[0]->kind());
  EXPECT_EQ(google_apis::ResourceEntry::ENTRY_KIND_FOLDER, entries[1]->kind());
}

TEST_F(LocalToRemoteSyncerTest, Conflict_CreateFileOnFile) {
  const GURL kOrigin("chrome-extension://example");
  const std::string sync_root = CreateSyncRoot();
  const std::string app_root = CreateRemoteFolder(sync_root, kOrigin.host());
  InitializeMetadataDatabase();
  RegisterApp(kOrigin.host(), app_root);

  CreateRemoteFile(app_root, "foo", "data");
  EXPECT_EQ(SYNC_STATUS_OK, ListChanges());

  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "foo")));

  // There should exist both files on remote.
  ScopedVector<google_apis::ResourceEntry> entries =
      GetResourceEntriesForParentAndTitle(app_root, "foo");
  ASSERT_EQ(2u, entries.size());
  EXPECT_EQ(google_apis::ResourceEntry::ENTRY_KIND_FILE, entries[0]->kind());
  EXPECT_EQ(google_apis::ResourceEntry::ENTRY_KIND_FILE, entries[1]->kind());
}

TEST_F(LocalToRemoteSyncerTest, Conflict_UpdateDeleteOnFile) {
  const GURL kOrigin("chrome-extension://example");
  const std::string sync_root = CreateSyncRoot();
  const std::string app_root = CreateRemoteFolder(sync_root, kOrigin.host());
  InitializeMetadataDatabase();
  RegisterApp(kOrigin.host(), app_root);

  const std::string file_id = CreateRemoteFile(app_root, "foo", "data");
  EXPECT_EQ(SYNC_STATUS_OK, ListChanges());
  EXPECT_EQ(SYNC_STATUS_NO_CHANGE_TO_SYNC,
            RunRemoteToLocalSyncerUntilIdle());

  DeleteResource(file_id);

  EXPECT_EQ(SYNC_STATUS_FILE_BUSY, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "foo")));
  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "foo")));

  ScopedVector<google_apis::ResourceEntry> entries =
      GetResourceEntriesForParentAndTitle(app_root, "foo");
  ASSERT_EQ(1u, entries.size());
  EXPECT_EQ(google_apis::ResourceEntry::ENTRY_KIND_FILE, entries[0]->kind());
  EXPECT_TRUE(!entries[0]->deleted());
  EXPECT_NE(file_id, entries[0]->resource_id());
}

TEST_F(LocalToRemoteSyncerTest, Conflict_CreateDeleteOnFile) {
  const GURL kOrigin("chrome-extension://example");
  const std::string sync_root = CreateSyncRoot();
  const std::string app_root = CreateRemoteFolder(sync_root, kOrigin.host());
  InitializeMetadataDatabase();
  RegisterApp(kOrigin.host(), app_root);

  const std::string file_id = CreateRemoteFile(app_root, "foo", "data");
  EXPECT_EQ(SYNC_STATUS_OK, ListChanges());
  EXPECT_EQ(SYNC_STATUS_NO_CHANGE_TO_SYNC,
            RunRemoteToLocalSyncerUntilIdle());

  DeleteResource(file_id);

  EXPECT_EQ(SYNC_STATUS_OK, ListChanges());

  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_FILE),
      URL(kOrigin, "foo")));

  ScopedVector<google_apis::ResourceEntry> entries =
      GetResourceEntriesForParentAndTitle(app_root, "foo");
  ASSERT_EQ(1u, entries.size());
  EXPECT_EQ(google_apis::ResourceEntry::ENTRY_KIND_FILE, entries[0]->kind());
  EXPECT_TRUE(!entries[0]->deleted());
  EXPECT_NE(file_id, entries[0]->resource_id());
}

TEST_F(LocalToRemoteSyncerTest, Conflict_CreateFolderOnFolder) {
  const GURL kOrigin("chrome-extension://example");
  const std::string sync_root = CreateSyncRoot();
  const std::string app_root = CreateRemoteFolder(sync_root, kOrigin.host());
  InitializeMetadataDatabase();
  RegisterApp(kOrigin.host(), app_root);

  const std::string folder_id = CreateRemoteFolder(app_root, "foo");

  EXPECT_EQ(SYNC_STATUS_OK, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_DIRECTORY),
      URL(kOrigin, "foo")));

  ScopedVector<google_apis::ResourceEntry> entries =
      GetResourceEntriesForParentAndTitle(app_root, "foo");
  ASSERT_EQ(2u, entries.size());
  EXPECT_EQ(google_apis::ResourceEntry::ENTRY_KIND_FOLDER, entries[0]->kind());
  EXPECT_EQ(google_apis::ResourceEntry::ENTRY_KIND_FOLDER, entries[1]->kind());
  EXPECT_TRUE(!entries[0]->deleted());
  EXPECT_TRUE(!entries[1]->deleted());
  EXPECT_TRUE(folder_id == entries[0]->resource_id() ||
              folder_id == entries[1]->resource_id());

  TrackerIDSet trackers;
  EXPECT_TRUE(GetMetadataDatabase()->FindTrackersByFileID(
      folder_id, &trackers));
  EXPECT_EQ(1u, trackers.size());
  ASSERT_TRUE(trackers.has_active());
}

TEST_F(LocalToRemoteSyncerTest, AppRootDeletion) {
  const GURL kOrigin("chrome-extension://example");
  const std::string sync_root = CreateSyncRoot();
  const std::string app_root = CreateRemoteFolder(sync_root, kOrigin.host());
  InitializeMetadataDatabase();
  RegisterApp(kOrigin.host(), app_root);

  DeleteResource(app_root);
  EXPECT_EQ(SYNC_STATUS_OK, ListChanges());
  EXPECT_EQ(SYNC_STATUS_NO_CHANGE_TO_SYNC,
            RunRemoteToLocalSyncerUntilIdle());

  EXPECT_EQ(SYNC_STATUS_UNKNOWN_ORIGIN, RunLocalToRemoteSyncer(
      FileChange(FileChange::FILE_CHANGE_ADD_OR_UPDATE,
                 SYNC_FILE_TYPE_DIRECTORY),
      URL(kOrigin, "foo")));

  // SyncEngine will re-register the app and resurrect the app root later.
}

}  // namespace drive_backend
}  // namespace sync_file_system
