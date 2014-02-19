/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/FilesystemTaskBase.h"

#include "nsNetUtil.h" // Stream transport service.
#include "mozilla/dom/ContentChild.h"
#include "mozilla/dom/FilesystemBase.h"
#include "mozilla/dom/FilesystemRequestParent.h"
#include "mozilla/dom/FilesystemUtils.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/PContent.h"
#include "mozilla/unused.h"

namespace mozilla {
namespace dom {

FilesystemTaskBase::FilesystemTaskBase(FilesystemBase* aFilesystem)
  : mErrorValue(NS_OK)
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  MOZ_ASSERT(aFilesystem, "aFilesystem should not be null.");
  mFilesystem = do_GetWeakReference(aFilesystem);
}

FilesystemTaskBase::FilesystemTaskBase(FilesystemBase* aFilesystem,
                   const FilesystemParams& aParam,
                   FilesystemRequestParent* aParent)
  : mErrorValue(NS_OK)
  , mRequestParent(aParent)
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Only call from parent process!");
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  MOZ_ASSERT(aFilesystem, "aFilesystem should not be null.");
  mFilesystem = do_GetWeakReference(aFilesystem);
}

FilesystemTaskBase::~FilesystemTaskBase()
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
}

void
FilesystemTaskBase::Start()
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");

  if (HasError()) {
    HandlerCallback();
    return;
  }

  if (FilesystemUtils::IsParentProcess()) {
    // Run in parent process.
    // Start worker thread.
    nsCOMPtr<nsIEventTarget> target
      = do_GetService(NS_STREAMTRANSPORTSERVICE_CONTRACTID);
    NS_ASSERTION(target, "Must have stream transport service.");
    target->Dispatch(this, NS_DISPATCH_NORMAL);
    return;
  }

  // Run in child process.
  nsRefPtr<FilesystemBase> filesystem = do_QueryReferent(mFilesystem);
  if (!filesystem) {
    return;
  }

  // Retain a reference so the task object isn't deleted without IPDL's
  // knowledge. The reference will be released by
  // mozilla::dom::ContentChild::DeallocPFilesystemRequestChild.
  AddRef();
  ContentChild::GetSingleton()->SendPFilesystemRequestConstructor(this,
    GetRequestParams(filesystem->ToString()));
}

NS_IMETHODIMP
FilesystemTaskBase::Run()
{
  if (!NS_IsMainThread()) {
    // Run worker thread tasks
    Work();
    // Dispatch itself to main thread
    NS_DispatchToMainThread(this);
    return NS_OK;
  }

  // Run main thread tasks
  HandleResult();
  return NS_OK;
}

void
FilesystemTaskBase::HandleResult()
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Only call from parent process!");
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  nsRefPtr<FilesystemBase> filesystem = do_QueryReferent(mFilesystem);
  if (!filesystem) {
    return;
  }
  if (mRequestParent && mRequestParent->IsRunning()) {
    unused << mRequestParent->Send__delete__(mRequestParent,
      GetRequestResult());
  } else {
    HandlerCallback();
  }
}

FilesystemResponseValue
FilesystemTaskBase::GetRequestResult() const
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Only call from parent process!");
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  if (HasError()) {
    return FilesystemErrorResponse(mErrorValue);
  } else {
    return GetSuccessRequestResult();
  }
}

void
FilesystemTaskBase::SetRequestResult(const FilesystemResponseValue& aValue)
{
  MOZ_ASSERT(!FilesystemUtils::IsParentProcess(),
             "Only call from child process!");
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  if (aValue.type() == FilesystemResponseValue::TFilesystemErrorResponse) {
    FilesystemErrorResponse r = aValue;
    mErrorValue = r.error();
  } else {
    SetSuccessRequestResult(aValue);
  }
}

bool
FilesystemTaskBase::Recv__delete__(const FilesystemResponseValue& aValue)
{
  SetRequestResult(aValue);
  HandlerCallback();
  return true;
}

void
FilesystemTaskBase::SetError(const nsresult& aErrorValue)
{
  uint16_t module = NS_ERROR_GET_MODULE(aErrorValue);
  if (module == NS_ERROR_MODULE_DOM_FILESYSTEM ||
      module == NS_ERROR_MODULE_DOM_FILE) {
    mErrorValue = aErrorValue;
    return;
  }

  switch (aErrorValue) {
    case NS_OK:
      mErrorValue = NS_OK;
      return;

    case NS_ERROR_FILE_INVALID_PATH:
    case NS_ERROR_FILE_UNRECOGNIZED_PATH:
      mErrorValue = NS_ERROR_DOM_FILESYSTEM_INVALID_PATH_ERR;
      return;

    case NS_ERROR_FILE_DESTINATION_NOT_DIR:
      mErrorValue = NS_ERROR_DOM_FILESYSTEM_INVALID_MODIFICATION_ERR;
      return;

    case NS_ERROR_FILE_ACCESS_DENIED:
    case NS_ERROR_FILE_DIR_NOT_EMPTY:
      mErrorValue = NS_ERROR_DOM_FILESYSTEM_NO_MODIFICATION_ALLOWED_ERR;
      return;

    case NS_ERROR_FILE_TARGET_DOES_NOT_EXIST:
    case NS_ERROR_NOT_AVAILABLE:
      mErrorValue = NS_ERROR_DOM_FILE_NOT_FOUND_ERR;
      return;

    case NS_ERROR_FILE_ALREADY_EXISTS:
      mErrorValue = NS_ERROR_DOM_FILESYSTEM_PATH_EXISTS_ERR;
      return;

    case NS_ERROR_FILE_NOT_DIRECTORY:
      mErrorValue = NS_ERROR_DOM_FILESYSTEM_TYPE_MISMATCH_ERR;
      return;

    case NS_ERROR_UNEXPECTED:
    default:
      mErrorValue = NS_ERROR_DOM_FILESYSTEM_UNKNOWN_ERR;
      return;
  }
}

} // namespace dom
} // namespace mozilla
