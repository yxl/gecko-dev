/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "GetFileOrDirectoryTask.h"
#include "Directory.h"
#include "FilesystemBase.h"
#include "FilesystemUtils.h"

#include "nsDOMFile.h"
#include "nsIFile.h"
#include "nsStringGlue.h"

#include "mozilla/dom/Promise.h"

namespace mozilla {
namespace dom {

GetFileOrDirectoryTask::GetFileOrDirectoryTask(
  FilesystemBase* aFilesystem,
  const nsAString& aTargetPath,
  nsresult aErrorValue /*= NS_OK*/)
  : TaskBase(aFilesystem)
  , mTargetRealPath(aTargetPath)
  , mIsDirectory(false)
{
  mPromise = new Promise(aFilesystem->GetWindow());
  SetError(aErrorValue);
}

GetFileOrDirectoryTask::GetFileOrDirectoryTask(
  FilesystemBase* aFilesystem,
  const FilesystemGetFileOrDirectoryParams& aParam,
  FilesystemRequestParent* aParent)
  : TaskBase(aFilesystem, aParam, aParent)
  , mIsDirectory(false)
{
  mTargetRealPath = aParam.realPath();
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
  return FilesystemGetFileOrDirectoryParams(aFilesystem, mTargetRealPath);
}

FilesystemResponseValue
GetFileOrDirectoryTask::GetSuccessRequestResult() const
{
  if (mIsDirectory) {
    return FilesystemDirectoryResponse(mTargetRealPath);
  }

  ContentParent* cp = static_cast<ContentParent*>(mRequestParent->Manager());
  BlobParent* actor = cp->GetOrCreateActorForBlob(mTargetFile);
  if (!actor) {
    return FilesystemErrorResponse(NS_ERROR_DOM_FILESYSTEM_UNKNOWN_ERR);
  }
  FilesystemFileResponse response;
  response.blobParent() = actor;
  return response;
}

void
GetFileOrDirectoryTask::SetSuccessRequestResult(const FilesystemResponseValue& aValue)
{
  switch (aValue.type()) {
    case FilesystemResponseValue::TFilesystemFileResponse: {
      FilesystemFileResponse r = aValue;
      BlobChild* actor = static_cast<BlobChild*>(r.blobChild());
      nsCOMPtr<nsIDOMBlob> blob = actor->GetBlob();
      mTargetFile = do_QueryInterface(blob);
      mIsDirectory = false;
      break;
    }
    case FilesystemResponseValue::TFilesystemDirectoryResponse: {
      FilesystemDirectoryResponse r = aValue;
      mTargetRealPath = r.realPath();
      mIsDirectory = true;
      break;
    }
    default: {
      NS_RUNTIMEABORT("not reached");
      break;
    }
  }
}

void
GetFileOrDirectoryTask::Work()
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Only call from parent process!");

  nsRefPtr<FilesystemBase> filesystem = mFilesystem->Get();
  if (!filesystem) {
    return;
  }

  // Whether we want to get the root directory.
  bool getRoot = mTargetRealPath.IsEmpty();

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
    if (!getRoot) {
      SetError(NS_ERROR_DOM_FILE_NOT_FOUND_ERR);
      return;
    }

    // If the root directory doesn't exit, create it.
    rv = file->Create(nsIFile::DIRECTORY_TYPE, 0777);
    if (NS_FAILED(rv)) {
      SetError(rv);
      return;
    }
  }

  // Get isDirectory.
  rv = file->IsDirectory(&mIsDirectory);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }

  if (!mIsDirectory) {
    // Check if the root is a directory.
    if (getRoot) {
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

    mTargetFile = new nsDOMFileFile(file);
  }
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
    JS::Value value;
    if (mIsDirectory) {
      nsRefPtr<Directory> dir = new Directory(filesystem, mTargetRealPath);
      value = FilesystemUtils::WrapperCacheObjectToJsval(cx,
        filesystem->GetWindow(), dir);
    } else {
      value = FilesystemUtils::InterfaceToJsval(cx, filesystem->GetWindow(),
        mTargetFile, &NS_GET_IID(nsIDOMFile));
    }
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
