/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_RemoveTask_h
#define mozilla_dom_RemoveTask_h

#include "mozilla/dom/FilesystemTaskBase.h"
#include "nsAutoPtr.h"

class nsCString;
class nsIDOMFile;
class nsString;

namespace mozilla {
namespace dom {

class Promise;

class RemoveTask MOZ_FINAL
  : public FilesystemTaskBase
{
public:
  RemoveTask(FilesystemBase* aFilesystem,
             const nsAString& aDirPath,
             nsIDOMFile* aTargetFile,
             const nsAString& aTargetPath,
             bool aRecursive);
  RemoveTask(FilesystemBase* aFilesystem,
             const FilesystemRemoveParams& aParam,
             FilesystemRequestParent* aParent);

  virtual
  ~RemoveTask();

  already_AddRefed<Promise>
  GetPromise();

  virtual void
  GetPermissionAccessType(nsCString& aAccess) const MOZ_OVERRIDE;

protected:
  virtual FilesystemParams
  GetRequestParams(const nsString& aFilesystem) const MOZ_OVERRIDE;

  virtual FilesystemResponseValue
  GetSuccessRequestResult() const MOZ_OVERRIDE;

  virtual void
  SetSuccessRequestResult(const FilesystemResponseValue& aValue) MOZ_OVERRIDE;

  virtual void
  Work() MOZ_OVERRIDE;

  virtual void
  HandlerCallback() MOZ_OVERRIDE;

private:
  nsRefPtr<Promise> mPromise;
  nsString mDirRealPath;
  nsCOMPtr<nsIDOMFile> mTargetFile;
  nsString mTargetRealPath;
  bool mRecursive;
  bool mReturnValue;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_RemoveTask_h
