/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CreateDirectoryTask.h"

#include "DOMError.h"
#include "mozilla/dom/Directory.h"
#include "mozilla/dom/FilesystemBase.h"
#include "mozilla/dom/FilesystemUtils.h"
#include "mozilla/dom/Promise.h"
#include "nsIFile.h"
#include "nsStringGlue.h"

namespace mozilla {
namespace dom {

CreateDirectoryTask::CreateDirectoryTask(FilesystemBase* aFilesystem,
                                         const nsAString& aPath)
  : FilesystemTaskBase(aFilesystem)
  , mTargetRealPath(aPath)
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

CreateDirectoryTask::CreateDirectoryTask(
  FilesystemBase* aFilesystem,
  const FilesystemCreateDirectoryParams& aParam,
  FilesystemRequestParent* aParent)
  : FilesystemTaskBase(aFilesystem, aParam, aParent)
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Only call from parent process!");
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  mTargetRealPath = aParam.realPath();
}

CreateDirectoryTask::~CreateDirectoryTask()
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
}

already_AddRefed<Promise>
CreateDirectoryTask::GetPromise()
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  return nsRefPtr<Promise>(mPromise).forget();
}

FilesystemParams
CreateDirectoryTask::GetRequestParams(const nsString& aFilesystem) const
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  return FilesystemCreateDirectoryParams(aFilesystem, mTargetRealPath);
}

FilesystemResponseValue
CreateDirectoryTask::GetSuccessRequestResult() const
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  return FilesystemDirectoryResponse(mTargetRealPath);
}

void
CreateDirectoryTask::SetSuccessRequestResult(const FilesystemResponseValue& aValue)
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  FilesystemDirectoryResponse r = aValue;
  mTargetRealPath = r.realPath();
}

void
CreateDirectoryTask::Work()
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Only call from parent process!");
  MOZ_ASSERT(!NS_IsMainThread(), "Only call on child thread!");

  nsRefPtr<FilesystemBase> filesystem = do_QueryReferent(mFilesystem);
  if (!filesystem) {
    return;
  }

  nsCOMPtr<nsIFile> file = filesystem->GetLocalFile(mTargetRealPath);
  if (!file) {
    SetError(NS_ERROR_DOM_FILESYSTEM_INVALID_PATH_ERR);
    return;
  }

  bool ret;
  nsresult rv = file->Exists(&ret);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }

  if (ret) {
    SetError(NS_ERROR_DOM_FILESYSTEM_PATH_EXISTS_ERR);
    return;
  }

  rv = file->Create(nsIFile::DIRECTORY_TYPE, 0777);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }
}

void
CreateDirectoryTask::HandlerCallback()
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
  nsRefPtr<Directory> dir = new Directory(filesystem, mTargetRealPath);
  mPromise->MaybeResolve(dir);
}

void
CreateDirectoryTask::GetPermissionAccessType(nsCString& aAccess) const
{
  aAccess.AssignLiteral("create");
}

} // namespace dom
} // namespace mozilla
