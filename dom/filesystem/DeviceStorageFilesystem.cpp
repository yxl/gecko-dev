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
#include "nsIDOMFile.h"
#include "nsIFile.h"
#include "nsPIDOMWindow.h"

namespace mozilla {
namespace dom {

DeviceStorageFilesystem::DeviceStorageFilesystem(
  const nsAString& aStorageType,
  const nsAString& aStorageName,
  nsPIDOMWindow* aWindow /* = nullptr */)
  : mWindow(aWindow)
{
  mStorageType = aStorageType;
  mStorageName = aStorageName;

  // Generate the string representation of the file system.
  mString.AppendLiteral("devicestorage-");
  mString.Append(mStorageType);
  mString.AppendLiteral("-");
  mString.Append(mStorageName);

  if (!FilesystemUtils::IsParentProcess()) {
    return;
  }

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
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Should be on parent process!");
  nsAutoString localPath;
  FilesystemUtils::NormalizedPathToLocalPath(aRealPath, localPath);
  localPath = mLocalRootPath +
              NS_LITERAL_STRING(FILESYSTEM_DOM_PATH_SEPARATOR) +
              localPath;
  nsCOMPtr<nsIFile> file;
  nsresult rv = NS_NewLocalFile(localPath, false, getter_AddRefs(file));
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return nullptr;
  }
  return file.forget();
}

bool
DeviceStorageFilesystem::GetRealPath(nsIDOMFile *aFile, nsAString& aRealPath) const
{
  MOZ_ASSERT(FilesystemUtils::IsParentProcess(),
             "Should be on parent process!");
  MOZ_ASSERT(aFile, "aFile Should not be null.");

  aRealPath.Truncate();

  nsAutoString localPath;
  if (NS_FAILED(aFile->GetMozFullPathInternal(localPath))) {
    return false;
  }

  if (!StringBeginsWith(localPath, mLocalRootPath)) {
    return false;
  }

  localPath  = Substring(localPath, mLocalRootPath.Length());
  FilesystemUtils::LocalPathToNormalizedPath(localPath, aRealPath);

  return true;
}

const nsAString&
DeviceStorageFilesystem::GetRootName() const
{
  return mStorageName;
}

} // namespace dom
} // namespace mozilla
