/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CreateDirectoryTask.h"
#include "Directory.h"
#include "FilesystemBase.h"
#include "FilesystemUtils.h"

#include "nsIFile.h"
#include "nsStringGlue.h"

#include "mozilla/dom/Promise.h"

namespace mozilla {
namespace dom {

CreateDirectoryTask::CreateDirectoryTask(FilesystemBase* aFilesystem,
                                         const nsAString& aPath,
                                         nsresult aErrorValue /*= NS_OK*/)
  : TaskBase(aFilesystem)
  , mTargetRealPath(aPath)
{
  mPromise = new Promise(aFilesystem->GetWindow());
  SetError(aErrorValue);
}

CreateDirectoryTask::CreateDirectoryTask(
  FilesystemBase* aFilesystem,
  const FilesystemCreateDirectoryParams& aParam,
  FilesystemRequestParent* aParent) : TaskBase(aFilesystem, aParam, aParent)
{
  mTargetRealPath = aParam.realPath();
}

CreateDirectoryTask::~CreateDirectoryTask()
{
}

already_AddRefed<Promise>
CreateDirectoryTask::GetPromise()
{
  return nsRefPtr<Promise>(mPromise).forget();
}

FilesystemParams
CreateDirectoryTask::GetRequestParams(const nsString& aFilesystem) const
{
  return FilesystemCreateDirectoryParams(aFilesystem, mTargetRealPath);
}

FilesystemResponseValue
CreateDirectoryTask::GetSuccessRequestResult() const
{
  return FilesystemDirectoryResponse(mTargetRealPath);
}

void
CreateDirectoryTask::SetSuccessRequestResult(const FilesystemResponseValue& aValue)
{
  FilesystemDirectoryResponse r = aValue;
  mTargetRealPath = r.realPath();
}

void
CreateDirectoryTask::Work()
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Only call from parent process!");

  nsRefPtr<FilesystemBase> filesystem = mFilesystem->Get();
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
  nsRefPtr<FilesystemBase> filesystem = mFilesystem->Get();
  if (!filesystem) {
    return;
  }

  AutoSafeJSContext cx;

  if (!HasError()) {
    nsRefPtr<Directory> dir = new Directory(filesystem, mTargetRealPath);
    JS::Value value = FilesystemUtils::WrapperCacheObjectToJsval(cx,
      filesystem->GetWindow(), dir);
    MOZ_ASSERT(!JSVAL_IS_NULL(value));
    JS::Rooted<JS::Value> val(cx, value);
    mPromise->MaybeResolve(cx, val);
  } else {
    nsRefPtr<DOMError> domError = new DOMError(filesystem->GetWindow(),
      mErrorValue);
    JS::Value error = FilesystemUtils::WrapperCacheObjectToJsval(cx,
      filesystem->GetWindow(), domError);
    JS::Rooted<JS::Value> val(cx, error);
    mPromise->MaybeReject(cx, val);
  }
}

} // namespace dom
} // namespace mozilla
