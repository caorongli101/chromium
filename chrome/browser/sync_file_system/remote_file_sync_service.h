// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SYNC_FILE_SYSTEM_REMOTE_FILE_SYNC_SERVICE_H_
#define CHROME_BROWSER_SYNC_FILE_SYSTEM_REMOTE_FILE_SYNC_SERVICE_H_

#include "base/basictypes.h"
#include "webkit/fileapi/syncable/sync_callbacks.h"

class GURL;

namespace sync_file_system {

class RemoteChangeProcessor;
class LocalChangeProcessor;

// This class represents a backing service of the sync filesystem.
// Owned by SyncFileSystemService.
class RemoteFileSyncService {
 public:
  class Observer {
   public:
    Observer() {}
    virtual ~Observer() {}

    // This is called when there're one or more remote changes available.
    // |pending_changes_hint| indicates the pending queue length to help sync
    // scheduling but the value may not be accurately reflect the real-time
    // value.
    virtual void OnRemoteChangeAvailable(int64 pending_changes_hint) = 0;
   private:
    DISALLOW_COPY_AND_ASSIGN(Observer);
  };

  RemoteFileSyncService() {}
  virtual ~RemoteFileSyncService() {}

  virtual void AddObserver(Observer* observer) = 0;
  virtual void RemoveObserver(Observer* observer) = 0;

  // Registers |origin| to track remote side changes for the |origin|.
  virtual void RegisterOriginForTrackingChanges(const GURL& origin) = 0;

  // Unregisters |origin| to track remote side changes for the |origin|.
  virtual void UnregisterOriginForTrackingChanges(const GURL& origin) = 0;

  // Called by the sync engine to process one remote change.
  // After a change is processed |callback| will be called (to return
  // the control to the sync engine).
  virtual void ProcessRemoteChange(
      RemoteChangeProcessor* processor,
      const fileapi::SyncCompletionCallback& callback) = 0;

  // Returns a LocalChangeProcessor that applies a local change to the remote
  // storage backed by this service.
  virtual LocalChangeProcessor* GetLocalChangeProcessor() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(RemoteFileSyncService);
};

}  // namespace sync_file_system

#endif  // CHROME_BROWSER_SYNC_FILE_SYSTEM_REMOTE_FILE_SYNC_SERVICE_H_
