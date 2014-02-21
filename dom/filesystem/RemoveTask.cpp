/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "RemoveTask.h"

#include "DOMError.h"
#include "js/Value.h"
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
                       bool aDeep)
  : FilesystemTaskBase(aFilesystem)
  , mDirRealPath(aDirPath)
  , mTargetFile(aTargetFile)
  , mTargetRealPath(aTargetPath)
  , mDeep(aDeep)
  , mReturnValue(false)
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  mPromise = new Promise(aFilesystem->GetWindow());
}

RemoveTask::RemoveTask(FilesystemBase* aFilesystem,
                       const FilesystemRemoveParams& aParam,
                       FilesystemRequestParent* aParent)
  : FilesystemTaskBase(aFilesystem, aParam, aParent)
  , mDeep(false)
  , mReturnValue(false)
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Only call from parent process!");
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");

  mDirRealPath = aParam.directory();

  mDeep = aParam.deep();

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
  param.deep() = mDeep;
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
  MOZ_ASSERT(!NS_IsMainThread(), "Only call on child thread!");

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

  rv = file->Remove(mDeep);
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

  AutoSafeJSContext cx;
  JSObject* global =
    FilesystemUtils::GetGetGlobalJSObject(filesystem->GetWindow());
  if (!global) {
    return;
  }

  JSAutoCompartment ac(cx, global);
  JS::Rooted<JSObject*> scopeObj(cx, global);
  JS::Rooted<JS::Value> val(cx);

  if (HasError()) {
    nsRefPtr<DOMError> domError = new DOMError(filesystem->GetWindow(),
      mErrorValue);
    if (!dom::WrapNewBindingObject(cx, scopeObj, domError, &val)) {
      return;
    }
    mPromise->MaybeReject(cx, val);
    return;
  }

  val.setBoolean(mReturnValue);
  mPromise->MaybeResolve(cx, val);
}

void
RemoveTask::GetPermissionAccessType(nsCString& aAccess) const
{
  aAccess.AssignLiteral("write");
}

} // namespace dom
} // namespace mozilla
