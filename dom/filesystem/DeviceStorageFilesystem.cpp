/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "DeviceStorageFilesystem.h"
#include "FilesystemUtils.h"

#include "DeviceStorage.h"
#include "nsCOMPtr.h"
#include "nsDebug.h"
#include "nsDeviceStorage.h"
#include "nsIFile.h"
#include "nsPIDOMWindow.h"

namespace mozilla {
namespace dom {

DeviceStorageFilesystem::DeviceStorageFilesystem(const nsAString& aStorageType,
  nsPIDOMWindow* aWindow /* = nullptr */)
  : mWindow(aWindow)
{
  mString = NS_LITERAL_STRING("devicestorage-") + mStorageType;
  mStorageType = aStorageType;
  if (!DeviceStorageTypeChecker::IsVolumeBased(mStorageType)) {
    // The storage name will be the empty string
    mStorageName.Truncate();
  } else {
    nsDOMDeviceStorage::GetDefaultStorageName(mStorageType, mStorageName);
  }

  if (!FilesystemUtils::IsParentProcess()) {
    return;
  }
  nsCOMPtr<nsIFile> rootFile;
  DeviceStorageFile::GetRootDirectoryForType(aStorageType,
                                             mStorageName,
                                             getter_AddRefs(rootFile));

  NS_WARN_IF_FALSE(NS_SUCCEEDED(rootFile->GetPath(mLocalRootPath)),
                   "Failed to get file system root path.");
}

DeviceStorageFilesystem::~DeviceStorageFilesystem()
{
}

nsPIDOMWindow*
DeviceStorageFilesystem::GetWindow() const
{
  return mWindow;
}

already_AddRefed<nsIFile>
DeviceStorageFilesystem::GetLocalFile(const nsAString& aRealPath) const
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(), "Should be on chrome process!");
  nsAutoString localPath;
  FilesystemUtils::NormalizedPathToLocalPath(aRealPath, localPath);
  localPath = mLocalRootPath + localPath;
  nsCOMPtr<nsIFile> file;
  nsresult rv = NS_NewLocalFile(localPath, false, getter_AddRefs(file));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return nullptr;
  }
  return file.forget();
}

const nsAString&
DeviceStorageFilesystem::GetRootName() const
{
  return mStorageName;
}

} // namespace dom
} // namespace mozilla
