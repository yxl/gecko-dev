/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsNetUtil.h" // Stream transport service.
#include "mozilla/dom/ContentChild.h"
#include "mozilla/unused.h"
#include "nsDOMFile.h"
#include "FileSystemNotifyBase.h"

using mozilla::MonitorAutoLock;

namespace mozilla {
namespace dom {

FileSystemNotifyBase::FileSystemNotifyBase(FileSystemTaskBase* aTask,
                                           FileSystemRequestParent* aParent,
                                           const nsString& aNotifyString)
  : mRequestParent(aParent)
  , mTask(aTask)
  , mNotifyString(aNotifyString)
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
}

FileSystemNotifyBase::FileSystemNotifyBase(FileSystemTaskBase* aTask,
                                           FileSystemRequestParent* aParent,
                                           nsRefPtr<DOMFileImpl>& aNotifyFileImpl)
  : mRequestParent(aParent)
  , mTask(aTask)
  , mNotifyFileImpl(aNotifyFileImpl)
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
}

FileSystemNotifyBase::~FileSystemNotifyBase()
{
}

NS_IMETHODIMP
FileSystemNotifyBase::Run()
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  if (mNotifyFileImpl) {
    FileSystemFileResponse r;
    nsRefPtr<DOMFile> aFile = new DOMFile(mNotifyFileImpl);
    
    // Load the lazy dom file data from the parent before sending to the child.
    nsString mimeType;
    aFile->GetType(mimeType);
    uint64_t fileSize;
    aFile->GetSize(&fileSize);
    uint64_t lastModifiedDate;
    aFile->GetMozLastModifiedDate(&lastModifiedDate);
    ContentParent* cp = static_cast<ContentParent*>(mRequestParent->Manager());
    r.blobParent() = cp->GetOrCreateActorForBlob(aFile);
    if (mRequestParent) {
      if (mRequestParent->IsRunning())
        unused << mRequestParent->SendNotify(r);
    } else {
      mTask->HandlerNotify(r);
    }
  } else {
    FileSystemDirectoryResponse r;
    r.realPath() = mNotifyString;
    if (mRequestParent) {
      if (mRequestParent->IsRunning())
        unused << mRequestParent->SendNotify(r);
    } else {
      mTask->HandlerNotify(r);
    }
    
  }
  return NS_OK;
}

} // namespace dom
} // namespace mozilla
