/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/DeviceStorageFilesystem.h"

#include "DeviceStorage.h"
#include "mozilla/dom/FilesystemUtils.h"
#include "nsCOMPtr.h"
#include "nsDebug.h"
#include "nsIFile.h"
#include "nsPIDOMWindow.h"

namespace mozilla {
namespace dom {

DeviceStorageFilesystem::DeviceStorageFilesystem(
  const nsAString& aStorageType,
  const nsAString& aStorageName)
  : mDeviceStorage(nullptr)
{
  mStorageType = aStorageType;
  mStorageName = aStorageName;

  // Generate the string representation of the file system.
  mString.AppendLiteral("devicestorage-");
  mString.Append(mStorageType);
  mString.AppendLiteral("-");
  mString.Append(mStorageName);

  // Get the local path of the file system root.
  // Since the child process is not allowed to access the file system, we only
  // do this from the parent process.
  if (!FilesystemUtils::IsParentProcess()) {
    return;
  }
  nsCOMPtr<nsIFile> rootFile;
  DeviceStorageFile::GetRootDirectoryForType(aStorageType,
                                             aStorageName,
                                             getter_AddRefs(rootFile));

  NS_WARN_IF(!rootFile || NS_FAILED(rootFile->GetPath(mLocalRootPath)));
}

DeviceStorageFilesystem::~DeviceStorageFilesystem()
{
}

void
DeviceStorageFilesystem::SetDeviceStorage(nsDOMDeviceStorage* aDeviceStorage)
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  mDeviceStorage = aDeviceStorage;
}

nsPIDOMWindow*
DeviceStorageFilesystem::GetWindow() const
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  if (!mDeviceStorage) {
    return nullptr;
  }
  return mDeviceStorage->GetOwner();
}

already_AddRefed<nsIFile>
DeviceStorageFilesystem::GetLocalFile(const nsAString& aRealPath) const
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Should be on parent process!");
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
