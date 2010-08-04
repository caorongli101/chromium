// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/cros/update_library.h"

#include "base/message_loop.h"
#include "base/string_util.h"
#include "chrome/browser/chrome_thread.h"
#include "chrome/browser/chromeos/cros/cros_library.h"

namespace chromeos {

class UpdateLibraryImpl : public UpdateLibrary {
 public:
  UpdateLibraryImpl()
    : status_connection_(NULL) {
    if (CrosLibrary::Get()->EnsureLoaded()) {
      Init();
    }
  }

  ~UpdateLibraryImpl() {
    if (status_connection_) {
      DisconnectUpdateProgress(status_connection_);
    }
  }

  void AddObserver(Observer* observer) {
    observers_.AddObserver(observer);
  }

  void RemoveObserver(Observer* observer) {
    observers_.RemoveObserver(observer);
  }

  bool CheckForUpdate() {
    if (!CrosLibrary::Get()->EnsureLoaded())
      return false;

    return InitiateUpdateCheck();
  }

  bool RebootAfterUpdate() {
    if (!CrosLibrary::Get()->EnsureLoaded())
      return false;

    return RebootIfUpdated();
  }

  const UpdateLibrary::Status& status() const {
    return status_;
  }

 private:
  static void ChangedHandler(void* object,
      const UpdateProgress& status) {
    UpdateLibraryImpl* updater = static_cast<UpdateLibraryImpl*>(object);
    updater->UpdateStatus(Status(status));
  }

  void Init() {
    status_connection_ = MonitorUpdateStatus(&ChangedHandler, this);
  }

  void UpdateStatus(const Status& status) {
    // Make sure we run on UI thread.
    if (!ChromeThread::CurrentlyOn(ChromeThread::UI)) {
      ChromeThread::PostTask(
          ChromeThread::UI, FROM_HERE,
          NewRunnableMethod(this, &UpdateLibraryImpl::UpdateStatus, status));
      return;
    }

    status_ = status;
    FOR_EACH_OBSERVER(Observer, observers_, UpdateStatusChanged(this));
  }

  ObserverList<Observer> observers_;

  // A reference to the update api, to allow callbacks when the update
  // status changes.
  UpdateStatusConnection status_connection_;

  // The latest power status.
  Status status_;

  DISALLOW_COPY_AND_ASSIGN(UpdateLibraryImpl);
};

class UpdateLibraryStubImpl : public UpdateLibrary {
 public:
  UpdateLibraryStubImpl() {}
  ~UpdateLibraryStubImpl() {}
  void AddObserver(Observer* observer) {}
  void RemoveObserver(Observer* observer) {}
  bool CheckForUpdate() { return false; }
  bool RebootAfterUpdate() { return false; }
  const UpdateLibrary::Status& status() const {
    return status_;
  }

 private:
  Status status_;

  DISALLOW_COPY_AND_ASSIGN(UpdateLibraryStubImpl);
};

// static
UpdateLibrary* UpdateLibrary::GetImpl(bool stub) {
  if (stub)
    return new UpdateLibraryStubImpl();
  else
    return new UpdateLibraryImpl();
}

}  // namespace chromeos

// Allows InvokeLater without adding refcounting. This class is a Singleton and
// won't be deleted until it's last InvokeLater is run.
DISABLE_RUNNABLE_METHOD_REFCOUNT(chromeos::UpdateLibraryImpl);

