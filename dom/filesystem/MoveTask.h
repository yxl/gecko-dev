/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_MoveTask_h__
#define mozilla_dom_MoveTask_h__

#include "TaskBase.h"

#include "nsAutoPtr.h"

class nsString;

namespace mozilla {
namespace dom {

class FilesystemBase;
class FilesystemCreateDirectoryParams;
class FilesystemFile;
class FilesystemWeakRef;
class AbortableProgressPromise;

class MoveTask MOZ_FINAL : public TaskBase
{
public:
  MoveTask(FilesystemBase* aFilesystem,
           const nsAString& aSrcPath,
           const nsAString& aDestPath,
           nsresult aErrorValue = NS_OK);
  MoveTask(FilesystemBase* aFilesystem,
           const FilesystemMoveParams& aParam,
           FilesystemRequestParent* aParent);

  virtual ~MoveTask();

  already_AddRefed<AbortableProgressPromise> GetPromise();

protected:
  virtual FilesystemParams GetRequestParams(const nsString& aFilesystem) const MOZ_OVERRIDE;

  virtual FilesystemResponseValue GetSuccessRequestResult() const MOZ_OVERRIDE;
  virtual void SetSuccessRequestResult(const FilesystemResponseValue& aValue) MOZ_OVERRIDE;

  virtual void Work() MOZ_OVERRIDE;
  virtual void HandlerCallback() MOZ_OVERRIDE;

private:
  nsRefPtr<AbortableProgressPromise> mPromise;
  nsString mSrcRealPath;
  nsString mDestRealPath;
  bool mReturnValue;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_MoveTask_h__
