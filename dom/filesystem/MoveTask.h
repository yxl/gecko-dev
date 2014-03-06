/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_MoveTask_h
#define mozilla_dom_MoveTask_h

#include "mozilla/dom/FileSystemTaskBase.h"
#include "nsAutoPtr.h"

namespace mozilla {
namespace dom {

class AbortableProgressPromise;

class MoveTask MOZ_FINAL
  : public FileSystemTaskBase
{
public:
  MoveTask(FileSystemBase* aFileSystem,
           const nsAString& aDirPath,
           const nsAString& aSrcPath,
           nsIDOMFile* aSrcFile,
           const nsAString& aDestDirectory,
           const nsAString& aDestName,
           nsresult aErrorValue);
  MoveTask(FileSystemBase* aFileSystem,
           const FileSystemMoveParams& aParam,
           FileSystemRequestParent* aParent);

  virtual
  ~MoveTask();

  already_AddRefed<AbortableProgressPromise>
  GetPromise();

  virtual void
  GetPermissionAccessType(nsCString& aAccess) const MOZ_OVERRIDE;

protected:
  virtual FileSystemParams
  GetRequestParams(const nsString& aFileSystem) const MOZ_OVERRIDE;

  virtual FileSystemResponseValue
  GetSuccessRequestResult() const MOZ_OVERRIDE;

  virtual void
  SetSuccessRequestResult(const FileSystemResponseValue& aValue) MOZ_OVERRIDE;

  virtual nsresult
  Work() MOZ_OVERRIDE;

  virtual void
  HandlerCallback() MOZ_OVERRIDE;

  virtual bool
  RecvNotify(const FileSystemNotifyValue& aValue) MOZ_OVERRIDE;

private:

  void
  NotifyMoveProgress(const nsString& aValue);

  nsRefPtr<AbortableProgressPromise> mPromise;
  nsString mDirRealPath;
  nsString mSrcRealPath;
  nsCOMPtr<nsIDOMFile> mSrcFile;
  nsString mDestDirectory;
  nsString mDestName;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_MoveTask_h
