/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_DeviceStorageFilesystem_h__
#define mozilla_dom_DeviceStorageFilesystem_h__

#include "FilesystemBase.h"
#include "nsString.h"

namespace mozilla {
namespace dom {

class DeviceStorageFilesystem : public FilesystemBase
{
public:
  DeviceStorageFilesystem(const nsAString& aStorageType,
                          nsPIDOMWindow* aWindow = nullptr);

  // Overrides FilesystemBase
  virtual nsPIDOMWindow* GetWindow() const MOZ_OVERRIDE;
  virtual already_AddRefed<nsIFile> GetLocalFile(const nsAString& aRealPath) const MOZ_OVERRIDE;
  virtual void GetRealPath(nsIDOMFile *aFile, nsAString& aRealPath) const MOZ_OVERRIDE;
  virtual const nsAString& GetRootName() const MOZ_OVERRIDE;
private:
  virtual ~DeviceStorageFilesystem();

  nsString mStorageType;
  nsString mStorageName;
  nsString mLocalRootPath;
  nsPIDOMWindow* mWindow;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_DeviceStorageFilesystem_h__
