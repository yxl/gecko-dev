/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/FilesystemBase.h"

#include "DeviceStorageFilesystem.h"
#include "nsCharSeparatedTokenizer.h"

namespace mozilla {
namespace dom {

NS_IMPL_ISUPPORTS1(FilesystemBase, nsISupportsWeakReference)

// static
already_AddRefed<FilesystemBase>
FilesystemBase::FromString(const nsAString& aString)
{
  if (StringBeginsWith(aString, NS_LITERAL_STRING("devicestorage-"))) {
    // The string representation of devicestorage file system is of the format:
    // devicestorage-StorageType-StorageName

    nsCharSeparatedTokenizer tokenizer(aString, char16_t('-'));
    tokenizer.nextToken();

    nsString storageType;
    if (tokenizer.hasMoreTokens()) {
      storageType = tokenizer.nextToken();
    }

    nsString storageName;
    if (tokenizer.hasMoreTokens()) {
      storageName = tokenizer.nextToken();
    }

    nsCOMPtr<DeviceStorageFilesystem> f =
      new DeviceStorageFilesystem(storageType, storageName);
    return f.forget();
  }
  return nullptr;
}

FilesystemBase::FilesystemBase()
  : mIsTesting(false)
{
}

FilesystemBase::~FilesystemBase()
{
}

nsPIDOMWindow*
FilesystemBase::GetWindow() const
{
  return nullptr;
}

bool
FilesystemBase::IsSafeFile(nsIFile* aFile) const
{
  return true;
}

bool
FilesystemBase::IsSafeFile(nsIDOMFile* aFile) const
{
  return true;
}

bool
FilesystemBase::IsSafeDirectory(Directory* aDir) const
{
  return true;
}

} // namespace dom
} // namespace mozilla
