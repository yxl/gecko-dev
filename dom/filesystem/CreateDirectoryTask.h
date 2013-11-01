/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_CreateDirectoryTask_h__
#define mozilla_dom_CreateDirectoryTask_h__

#include "TaskBase.h"

#include "nsAutoPtr.h"

class nsString;

namespace mozilla {
namespace dom {

class FilesystemBase;
class FilesystemCreateDirectoryParams;
class FilesystemFile;
class FilesystemWeakRef;
class Promise;

class CreateDirectoryTask MOZ_FINAL : public TaskBase
{
public:
  CreateDirectoryTask(FilesystemBase* aFilesystem,
                      const nsAString& aPath,
                      nsresult aErrorValue = NS_OK);
  CreateDirectoryTask(FilesystemBase* aFilesystem,
                      const FilesystemCreateDirectoryParams& aParam,
                      FilesystemRequestParent* aParent);

  virtual ~CreateDirectoryTask();

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
  nsRefPtr<FilesystemFile> mTargetFile;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_CreateDirectoryTask_h__
