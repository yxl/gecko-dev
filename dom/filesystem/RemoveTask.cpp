/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "RemoveTask.h"

#include "DOMError.h"
#include "mozilla/dom/FileSystemBase.h"
#include "mozilla/dom/FileSystemUtils.h"
#include "mozilla/dom/Promise.h"
#include "nsIDOMFile.h"
#include "nsIFile.h"
#include "nsStringGlue.h"

namespace mozilla {
namespace dom {

RemoveTask::RemoveTask(FileSystemBase* aFileSystem,
                       const nsAString& aDirPath,
                       nsIDOMFile* aTargetFile,
                       const nsAString& aTargetPath,
                       bool aRecursive)
  : FileSystemTaskBase(aFileSystem)
  , mDirRealPath(aDirPath)
  , mTargetFile(aTargetFile)
  , mTargetRealPath(aTargetPath)
  , mRecursive(aRecursive)
  , mReturnValue(false)
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  if (!aFileSystem) {
    return;
  }
  nsCOMPtr<nsIGlobalObject> globalObject =
    do_QueryInterface(aFileSystem->GetWindow());
  if (!globalObject) {
    return;
  }
  mPromise = new Promise(globalObject);
}

RemoveTask::RemoveTask(FileSystemBase* aFileSystem,
                       const FileSystemRemoveParams& aParam,
                       FileSystemRequestParent* aParent)
  : FileSystemTaskBase(aFileSystem, aParam, aParent)
  , mRecursive(false)
  , mReturnValue(false)
{
  MOZ_ASSERT(FileSystemUtils::IsParentProcess(),
             "Only call from parent process!");
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");

  mDirRealPath = aParam.directory();

  mRecursive = aParam.recursive();

  const FileSystemPathOrFileValue& target = aParam.target();

  if (target.type() == FileSystemPathOrFileValue::TnsString) {
    mTargetRealPath = target;
    return;
  }

  BlobParent* bp = static_cast<BlobParent*>(static_cast<PBlobParent*>(target));
  nsCOMPtr<nsIDOMBlob> blob = bp->GetBlob();
  mTargetFile = do_QueryInterface(blob);
  MOZ_ASSERT(mTargetFile, "mTargetFile should not be null.");
}

RemoveTask::~RemoveTask()
{
  MOZ_ASSERT(!mPromise || NS_IsMainThread(),
             "mPromise should be released on main thread!");
}

already_AddRefed<Promise>
RemoveTask::GetPromise()
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  return nsRefPtr<Promise>(mPromise).forget();
}

FileSystemParams
RemoveTask::GetRequestParams(const nsString& aFileSystem) const
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  FileSystemRemoveParams param;
  param.filesystem() = aFileSystem;
  param.directory() = mDirRealPath;
  param.recursive() = mRecursive;
  if (mTargetFile) {
    BlobChild* actor
      = ContentChild::GetSingleton()->GetOrCreateActorForBlob(mTargetFile);
    if (actor) {
      param.target() = actor;
    }
  } else {
    param.target() = mTargetRealPath;
  }
  return param;
}

FileSystemResponseValue
RemoveTask::GetSuccessRequestResult() const
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  return FileSystemBooleanResponse(mReturnValue);
}

void
RemoveTask::SetSuccessRequestResult(const FileSystemResponseValue& aValue)
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  FileSystemBooleanResponse r = aValue;
  mReturnValue = r.success();
}

nsresult
RemoveTask::Work()
{
  MOZ_ASSERT(FileSystemUtils::IsParentProcess(),
             "Only call from parent process!");
  MOZ_ASSERT(!NS_IsMainThread(), "Only call on worker thread!");

  nsRefPtr<FileSystemBase> filesystem = do_QueryReferent(mFileSystem);
  if (!filesystem) {
    return NS_ERROR_FAILURE;
  }

  // Get the DOM path if a DOMFile is passed as the target.
  if (mTargetFile) {
    if (!filesystem->GetRealPath(mTargetFile, mTargetRealPath)) {
      return NS_ERROR_DOM_SECURITY_ERR;
    }
    if (!FileSystemUtils::IsDescendantPath(mDirRealPath, mTargetRealPath)) {
      return NS_ERROR_DOM_FILESYSTEM_NO_MODIFICATION_ALLOWED_ERR;
    }
  }

  nsCOMPtr<nsIFile> file = filesystem->GetLocalFile(mTargetRealPath);
  if (!file) {
    return NS_ERROR_DOM_FILESYSTEM_INVALID_PATH_ERR;
  }

  bool exists = false;
  nsresult rv = file->Exists(&exists);
  TASK_BASE_ENSURE_SUCCESS(rv);

  if (!exists) {
    mReturnValue = false;
    return NS_OK;
  }

  bool isFile = false;
  rv = file->IsFile(&isFile);
  TASK_BASE_ENSURE_SUCCESS(rv);

  if (isFile && !filesystem->IsSafeFile(file)) {
    return NS_ERROR_DOM_SECURITY_ERR;
  }

  rv = file->Remove(mRecursive);
  TASK_BASE_ENSURE_SUCCESS(rv);

  mReturnValue = true;

  return NS_OK;
}

void
RemoveTask::HandlerCallback()
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  nsRefPtr<FileSystemBase> filesystem = do_QueryReferent(mFileSystem);
  if (!filesystem) {
    mPromise = nullptr;
    return;
  }

  if (HasError()) {
    nsRefPtr<DOMError> domError = new DOMError(filesystem->GetWindow(),
      mErrorValue);
    mPromise->MaybeReject(domError);
    mPromise = nullptr;
    return;
  }

  mPromise->MaybeResolve(mReturnValue);
  mPromise = nullptr;
}

void
RemoveTask::GetPermissionAccessType(nsCString& aAccess) const
{
  aAccess.AssignLiteral("write");
}

} // namespace dom
} // namespace mozilla
