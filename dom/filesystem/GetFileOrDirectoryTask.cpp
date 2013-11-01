/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "GetFileOrDirectoryTask.h"
#include "Directory.h"
#include "FilesystemBase.h"
#include "FilesystemUtils.h"
#include "FilesystemFile.h"

#include "nsIFile.h"
#include "nsStringGlue.h"

#include "mozilla/dom/Promise.h"

namespace mozilla {
namespace dom {

GetFileOrDirectoryTask::GetFileOrDirectoryTask(
  FilesystemBase* aFilesystem,
  const nsAString& aTargetPath,
  nsresult aErrorValue /*= NS_OK*/,
  bool aDirectoryOnly /*= false*/)
  : TaskBase(aFilesystem)
  , mTargetRealPath(aTargetPath)
  , mDirectoryOnly(aDirectoryOnly)
{
  mPromise = new Promise(aFilesystem->GetWindow());
  if (aErrorValue == NS_OK) {
    Start();
  } else {
    SetError(aErrorValue);
    HandlerCallback();
  }
}

GetFileOrDirectoryTask::GetFileOrDirectoryTask(
  FilesystemBase* aFilesystem,
  const FilesystemGetFileOrDirectoryParams& aParam,
  FilesystemRequestParent* aParent) : TaskBase(aFilesystem, aParam, aParent)
{
  mTargetRealPath = aParam.realPath();
  mDirectoryOnly = aParam.directoryOnly();
  Start();
}

GetFileOrDirectoryTask::~GetFileOrDirectoryTask()
{
}

already_AddRefed<Promise>
GetFileOrDirectoryTask::GetPromise()
{
  return nsRefPtr<Promise>(mPromise).forget();
}

FilesystemParams
GetFileOrDirectoryTask::GetRequestParams(const nsString& aFilesystem) const
{
  return FilesystemGetFileOrDirectoryParams(aFilesystem, mTargetRealPath,
                                            mDirectoryOnly);
}

FilesystemResponseValue
GetFileOrDirectoryTask::GetSuccessRequestResult() const
{
  return FilesystemFileResponse(mTargetFile->GetPath(), mTargetFile->IsDirectory());
}

void
GetFileOrDirectoryTask::SetSuccessRequestResult(const FilesystemResponseValue& aValue)
{
  FilesystemFileResponse r = aValue;
  mTargetFile = new FilesystemFile(r.realPath(), r.isDirectory());
}

void
GetFileOrDirectoryTask::Work()
{
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

  if (!ret) {
    SetError(NS_ERROR_DOM_FILE_NOT_FOUND_ERR);
    return;
  }

  bool isDirectory = false;

  // Get isDirectory.
  rv = file->IsDirectory(&isDirectory);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }

  if (!isDirectory) {
    if (mDirectoryOnly) {
      SetError(NS_ERROR_DOM_FILESYSTEM_TYPE_MISMATCH_ERR);
      return;
    }

    // Get isFile
    rv = file->IsFile(&ret);
    if (NS_FAILED(rv)) {
      SetError(rv);
      return;
    }
    if (!ret) {
      // Neither directory or file.
      SetError(NS_ERROR_DOM_FILESYSTEM_TYPE_MISMATCH_ERR);
      return;
    }
  }

  mTargetFile = new FilesystemFile(mTargetRealPath, isDirectory);
}

void
GetFileOrDirectoryTask::HandlerCallback()
{
  nsRefPtr<FilesystemBase> filesystem = mFilesystem->Get();
  if (!filesystem) {
    return;
  }

  AutoSafeJSContext cx;

  if (!HasError()) {
    JS::Value value = mTargetFile->ToJsValue(cx, filesystem);
    NS_WARN_IF(JSVAL_IS_NULL(value));
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
