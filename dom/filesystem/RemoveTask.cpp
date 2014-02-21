/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "RemoveTask.h"

#include "DOMError.h"
#include "mozilla/dom/FilesystemBase.h"
#include "mozilla/dom/FilesystemUtils.h"
#include "mozilla/dom/Promise.h"
#include "nsIDOMFile.h"
#include "nsIFile.h"
#include "nsStringGlue.h"

namespace mozilla {
namespace dom {

RemoveTask::RemoveTask(FilesystemBase* aFilesystem,
                       const nsAString& aDirPath,
                       nsIDOMFile* aTargetFile,
                       const nsAString& aTargetPath,
                       bool aRecursive)
  : FilesystemTaskBase(aFilesystem)
  , mDirRealPath(aDirPath)
  , mTargetFile(aTargetFile)
  , mTargetRealPath(aTargetPath)
  , mRecursive(aRecursive)
  , mReturnValue(false)
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  if (!aFilesystem) {
    return;
  }
  nsCOMPtr<nsIGlobalObject> globalObject =
    do_QueryInterface(aFilesystem->GetWindow());
  if (!globalObject) {
    return;
  }
  mPromise = new Promise(globalObject);
}

RemoveTask::RemoveTask(FilesystemBase* aFilesystem,
                       const FilesystemRemoveParams& aParam,
                       FilesystemRequestParent* aParent)
  : FilesystemTaskBase(aFilesystem, aParam, aParent)
  , mRecursive(false)
  , mReturnValue(false)
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Only call from parent process!");
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");

  mDirRealPath = aParam.directory();

  mRecursive = aParam.recursive();

  const FilesystemPathOrFileValue& target = aParam.target();

  if (target.type() == FilesystemPathOrFileValue::TnsString) {
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
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
}

already_AddRefed<Promise>
RemoveTask::GetPromise()
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  return nsRefPtr<Promise>(mPromise).forget();
}

FilesystemParams
RemoveTask::GetRequestParams(const nsString& aFilesystem) const
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  FilesystemRemoveParams param;
  param.filesystem() = aFilesystem;
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

FilesystemResponseValue
RemoveTask::GetSuccessRequestResult() const
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  return FilesystemBooleanResponse(mReturnValue);
}

void
RemoveTask::SetSuccessRequestResult(const FilesystemResponseValue& aValue)
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  FilesystemBooleanResponse r = aValue;
  mReturnValue = r.success();
}

void
RemoveTask::Work()
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Only call from parent process!");
  MOZ_ASSERT(!NS_IsMainThread(), "Only call on worker thread!");

  nsRefPtr<FilesystemBase> filesystem = do_QueryReferent(mFilesystem);
  if (!filesystem) {
    return;
  }

  // Get the DOM path if a DOMFile is passed as the target.
  if (mTargetFile) {
    if (!filesystem->GetRealPath(mTargetFile, mTargetRealPath)) {
      SetError(NS_ERROR_DOM_SECURITY_ERR);
      return;
    }
    if (!FilesystemUtils::IsDescendantPath(mDirRealPath, mTargetRealPath)) {
      SetError(NS_ERROR_DOM_FILESYSTEM_NO_MODIFICATION_ALLOWED_ERR);
      return;
    }
  }

  nsCOMPtr<nsIFile> file = filesystem->GetLocalFile(mTargetRealPath);
  if (!file) {
    SetError(NS_ERROR_DOM_FILESYSTEM_INVALID_PATH_ERR);
    return;
  }

  bool exists = false;
  nsresult rv = file->Exists(&exists);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }

  if (!exists) {
    mReturnValue = false;
    return;
  }

  bool isFile = false;
  rv = file->IsFile(&isFile);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }

  if (isFile && !filesystem->IsSafeFile(file)) {
    SetError(NS_ERROR_DOM_SECURITY_ERR);
    return;
  }

  rv = file->Remove(mRecursive);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }

  mReturnValue = true;
}

void
RemoveTask::HandlerCallback()
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  nsRefPtr<FilesystemBase> filesystem = do_QueryReferent(mFilesystem);
  if (!filesystem) {
    return;
  }

  if (HasError()) {
    nsRefPtr<DOMError> domError = new DOMError(filesystem->GetWindow(),
      mErrorValue);
    mPromise->MaybeReject(domError);
    return;
  }

  mPromise->MaybeResolve(mReturnValue);
}

void
RemoveTask::GetPermissionAccessType(nsCString& aAccess) const
{
  aAccess.AssignLiteral("write");
}

} // namespace dom
} // namespace mozilla
