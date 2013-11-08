/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "MoveTask.h"
#include "Directory.h"
#include "FilesystemBase.h"
#include "FilesystemUtils.h"

#include "nsIFile.h"
#include "nsStringGlue.h"

#include "mozilla/dom/AbortableProgressPromise.h"

namespace mozilla {
namespace dom {

MoveTask::MoveTask(FilesystemBase* aFilesystem,
                   const nsAString& aSrcPath,
                   nsIDOMFile* aSrcFile,
                   const nsAString& aDestDirectory,
                   const nsAString& aDestName,
                   nsresult aErrorValue)
  : TaskBase(aFilesystem)
  , mSrcRealPath(aSrcPath)
  , mSrcFile(aSrcFile)
  , mDestDirectory(aDestDirectory)
  , mDestName(aDestName)
  , mReturnValue(false)
{
  mPromise = new AbortableProgressPromise(aFilesystem->GetWindow());
  SetError(aErrorValue);
}

MoveTask::MoveTask(FilesystemBase* aFilesystem,
                   const FilesystemMoveParams& aParam,
                   FilesystemRequestParent* aParent)
  : TaskBase(aFilesystem, aParam, aParent)
  , mReturnValue(false)
{
  const FilesystemSourceParam& src = aParam.src();
  if (src.type() == FilesystemSourceParam::TnsString) {
    mSrcRealPath = src;
  } else if (src.type() == FilesystemSourceParam::TPBlobParent) {
    BlobParent* bp = static_cast<BlobParent*>(static_cast<PBlobParent*>(src));
    nsCOMPtr<nsIDOMBlob> blob = bp->GetBlob();
    MOZ_ASSERT(blob, "blob should not be null.");
    mSrcFile = do_QueryInterface(blob);
  } else {
    SetError(NS_ERROR_DOM_FILESYSTEM_UNKNOWN_ERR);
  }

  mDestDirectory = aParam.destDirectory();
  mDestName = aParam.destName();
}

MoveTask::~MoveTask()
{

}

already_AddRefed<AbortableProgressPromise>
MoveTask::GetPromise()
{
  return nsRefPtr<AbortableProgressPromise>(mPromise).forget();
}

FilesystemParams
MoveTask::GetRequestParams(const nsString& aFilesystem) const
{
  FilesystemMoveParams param;
  param.filesystem() = aFilesystem;
  FilesystemSourceParam src;
  if (mSrcFile) {
    BlobChild* actor
      = ContentChild::GetSingleton()->GetOrCreateActorForBlob(mSrcFile);
    if (actor) {
      src = actor;
    }
  } else {
    src = mSrcRealPath;
  }
  param.destDirectory() = mDestDirectory;
  param.destName() = mDestName;
  return param;
}

FilesystemResponseValue
MoveTask::GetSuccessRequestResult() const
{
  return FilesystemBooleanResponse(mReturnValue);
}

void
MoveTask::SetSuccessRequestResult(const FilesystemResponseValue& aValue)
{
  FilesystemBooleanResponse r = aValue;
  mReturnValue = r.success();
}

void
MoveTask::Work()
{
  nsRefPtr<FilesystemBase> filesystem = mFilesystem->Get();
  if (!filesystem) {
    return;
  }

  // Get the file path if a DOMFile is passed as the source.
  if (mSrcFile) {
    if (!filesystem->GetRealPath(mSrcFile, mSrcRealPath)) {
      SetError(NS_ERROR_DOM_FILESYSTEM_NO_MODIFICATION_ALLOWED_ERR);
      return;
    }
  }

  // Create nsIFile object from the source path.
  nsCOMPtr<nsIFile> srcFile = filesystem->GetLocalFile(mSrcRealPath);
  if (!srcFile) {
    SetError(NS_ERROR_DOM_FILESYSTEM_INVALID_PATH_ERR);
    return;
  }

  bool exists;

  // Check if the source entry exits.
  nsresult rv = srcFile->Exists(&exists);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }
  if (!exists) {
    SetError(NS_ERROR_DOM_FILE_NOT_FOUND_ERR);
    return;
  }

  // Check the source entry type.
  bool isDirectory;
  rv = srcFile->IsDirectory(&isDirectory);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }
  bool isFile;
  rv = srcFile->IsFile(&isFile);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }
  if (!isDirectory && !isFile) {
    // Neither directory or file.
    SetError(NS_ERROR_DOM_FILESYSTEM_TYPE_MISMATCH_ERR);
    return;
  }

  // If the destination name is not passed, use the source name.
  if (mDestName.IsEmpty()) {
    mDestName = Substring(mSrcRealPath,
      mSrcRealPath.RFindChar(FilesystemUtils::kSeparatorChar) + 1);
  }

  // Get the destination path.
  nsString destRealPath;
  destRealPath = mDestDirectory +
                 NS_LITERAL_STRING(FILESYSTEM_DOM_PATH_SEPARATOR) +
                 mDestName;

  // Creaet nsIFile object from the destination path.
  nsCOMPtr<nsIFile> destFile = filesystem->GetLocalFile(destRealPath);
  if (!destFile) {
    SetError(NS_ERROR_DOM_FILESYSTEM_INVALID_PATH_ERR);
    return;
  }

  // Check if destination exists.

  rv = destFile->Exists(&exists);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }
  if (exists) {
    SetError(NS_ERROR_DOM_FILESYSTEM_PATH_EXISTS_ERR);
    return;
  }

  // Move the entry.

  nsCOMPtr<nsIFile> destParent;
  rv = destFile->GetParent(getter_AddRefs(destParent));
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }
  nsString destName;
  rv = destFile->GetLeafName(destName);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }
  rv = srcFile->MoveTo(destParent, destName);
  if (NS_FAILED(rv)) {
    SetError(rv);
    return;
  }
  mReturnValue = true;
}

void
MoveTask::HandlerCallback()
{
  nsRefPtr<FilesystemBase> filesystem = mFilesystem->Get();
  if (!filesystem) {
    return;
  }

  AutoSafeJSContext cx;

  if (!HasError()) {
    JS::Value value = JS::UndefinedValue();
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
