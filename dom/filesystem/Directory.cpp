/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Directory.h"
#include "FilesystemBase.h"
#include "FilesystemUtils.h"
#include "TaskBase.h"

#include "nsCharSeparatedTokenizer.h"
#include "nsStringGlue.h"

#include "mozilla/dom/DirectoryBinding.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(Directory)
NS_IMPL_CYCLE_COLLECTING_ADDREF(Directory)
NS_IMPL_CYCLE_COLLECTING_RELEASE(Directory)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(Directory)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

// static
already_AddRefed<Promise>
Directory::GetRoot(FilesystemBase* aFilesystem)
{
  return TaskBase::StartGetFileOrDirectoryTask(
    aFilesystem, EmptyString(), NS_OK, true);
}

Directory::Directory(FilesystemBase* aFilesystem,
                     const nsAString& aPath)
  : mFilesystem(new FilesystemWeakRef(aFilesystem))
  , mPath(aPath)
{
  // Remove the trailing "/".
  mPath.Trim(FILESYSTEM_DOM_PATH_SEPARATOR, false, true);

  SetIsDOMBinding();
}

Directory::~Directory()
{
}

nsPIDOMWindow*
Directory::GetParentObject() const
{
  nsRefPtr<FilesystemBase> fs = mFilesystem->Get();
  if (!fs) {
    return nullptr;
  }
  return fs->GetWindow();
}

JSObject*
Directory::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aScope)
{
  return DirectoryBinding::Wrap(aCx, aScope, this);
}

void
Directory::GetName(nsString& aRetval) const
{
  aRetval.Truncate();

  nsRefPtr<FilesystemBase> fs = mFilesystem->Get();
  if (mPath.IsEmpty() && fs) {
    aRetval = fs->GetRootName();
    return;
  }

  aRetval = Substring(mPath,
                      mPath.RFindChar(FilesystemUtils::kSeparatorChar) + 1);
}

already_AddRefed<Promise>
Directory::CreateDirectory(const nsAString& aPath)
{
  nsresult error = NS_OK;
  nsString realPath;
  if (!DOMPathToRealPath(aPath, realPath)) {
    error = NS_ERROR_DOM_FILESYSTEM_INVALID_PATH_ERR;
  }
  nsRefPtr<FilesystemBase> fs = mFilesystem->Get();
  return TaskBase::StartCreateDirectoryTask(fs, realPath, error);
}

already_AddRefed<Promise>
Directory::Get(const nsAString& aPath)
{
  nsresult error = NS_OK;
  nsString realPath;
  if (!DOMPathToRealPath(aPath, realPath)) {
    error = NS_ERROR_DOM_FILESYSTEM_INVALID_PATH_ERR;
  }
  nsRefPtr<FilesystemBase> fs = mFilesystem->Get();
  return TaskBase::StartGetFileOrDirectoryTask(fs, realPath, error);
}

bool
Directory::DOMPathToRealPath(const nsAString& aPath, nsAString& aRealPath) const
{
  aRealPath.Truncate();

  nsString relativePath;
  relativePath = aPath;

  // Trim white spaces.
  static const char kWhitespace[] = "\b\t\r\n ";
  relativePath.Trim(kWhitespace);

  // Remove the leading "./".
  if (StringBeginsWith(relativePath, NS_LITERAL_STRING("./"))) {
    relativePath = Substring(relativePath, 2);
  } else {
    relativePath = relativePath;
  }

  // Remove trailing "/".
  relativePath.Trim(FILESYSTEM_DOM_PATH_SEPARATOR, false, true);

  // We don't allow empty relative path to access the root.
  if (relativePath.IsEmpty()) {
    return false;
  }

  if (!IsValidRelativePath(relativePath)) {
    return false;
  }

  aRealPath = mPath + NS_LITERAL_STRING(FILESYSTEM_DOM_PATH_SEPARATOR) +
    relativePath;

  return true;
}

// static
bool
Directory::IsValidRelativePath(const nsString& aPath)
{
  if (aPath.IsEmpty()) {
    return true;
  }

  // Absolute path is not allowed.
  if (aPath.First() == FilesystemUtils::kSeparatorChar) {
    return false;
  }

  static const nsString kCurrentDir = NS_LITERAL_STRING(".");
  static const nsString kParentDir = NS_LITERAL_STRING("..");

  // Split path and check each path component.
  nsCharSeparatedTokenizer tokenizer(aPath, FilesystemUtils::kSeparatorChar);
  while (tokenizer.hasMoreTokens()) {
    nsDependentSubstring pathComponent = tokenizer.nextToken();
    // The path containing empty components, such as "foo//bar", is invalid.
    // We don't allow paths, such as "../foo", "foo/./bar" and "foo/../bar",
    // to walk up the directory.
    if (pathComponent.IsEmpty() ||
        pathComponent.Equals(kCurrentDir) ||
        pathComponent.Equals(kParentDir)) {
      return false;
    }
  }

  return true;
}

} // namespace dom
} // namespace mozilla
