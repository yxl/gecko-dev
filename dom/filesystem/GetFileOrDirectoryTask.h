/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_GetFileOrDirectory_h__
#define mozilla_dom_GetFileOrDirectory_h__

#include "TaskBase.h"
#include "nsAutoPtr.h"

class nsString;

namespace mozilla {
namespace dom {

class FilesystemBase;
class FilesystemFile;
class FilesystemGetFileOrDirectoryParams;
class FilesystemWeakRef;
class Promise;

class GetFileOrDirectoryTask MOZ_FINAL : public TaskBase
{
public:
  // If aDirectoryOnly is set, we should ensure that the target is a directory.
  GetFileOrDirectoryTask(FilesystemBase* aFilesystem,
                         const nsAString& aTargetPath,
                         nsresult aErrorValue = NS_OK,
                         bool aDirectoryOnly = false);
  GetFileOrDirectoryTask(FilesystemBase* aFilesystem,
                         const FilesystemGetFileOrDirectoryParams& aParam,
                         FilesystemRequestParent* aParent);

  virtual ~GetFileOrDirectoryTask();

  already_AddRefed<Promise> GetPromise();

protected:
  virtual FilesystemParams GetRequestParams(const nsString& aFilesystem) const MOZ_OVERRIDE;

  virtual FilesystemResponseValue GetSuccessRequestResult() const MOZ_OVERRIDE;
  virtual void SetSuccessRequestResult(const FilesystemResponseValue& aValue) MOZ_OVERRIDE;

  virtual void Work() MOZ_OVERRIDE;
  virtual void HandlerCallback() MOZ_OVERRIDE;

private:
  nsRefPtr<Promise> mPromise;
  nsString mTargetRealPath;
  // If true, the task can only get a file.
  bool mDirectoryOnly;
  nsRefPtr<FilesystemFile> mTargetFile;

};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_GetFileOrDirectory_h__
