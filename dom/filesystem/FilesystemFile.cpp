/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FilesystemFile.h"
#include "Directory.h"
#include "FilesystemBase.h"
#include "FilesystemUtils.h"

#include "nsIFile.h"
#include "nsPIDOMWindow.h"

namespace mozilla {
namespace dom {

// static
bool
FilesystemFile::IsValidRelativePath(const nsString& aPath)
{
  if (aPath.IsEmpty()) {
    return true;
  }

  // Absolute path is not allowed.
  if (aPath.First() == kSeparatorChar) {
    return false;
  }

  static const nsString kCurrentDir = NS_LITERAL_STRING(".");
  static const nsString kParentDir = NS_LITERAL_STRING("..");

  // Split path and check each path component.
  int32_t begin = 0;
  int32_t end;
  for (begin = 0;
       (end = aPath.FindChar(kSeparatorChar, begin)) != kNotFound;
       begin = end + 1) {
    // The path containing empty components, such as "foo//bar", is invalid.
    if (begin == end) {
      return false;
    }
    // We don't allow paths, such as "../foo", "foo/./bar" and "foo/../bar",
    // to walk up the directory.
    nsDependentSubstring pathComponent = Substring(aPath, begin, end - begin);
    if (pathComponent.Equals(kCurrentDir) || pathComponent.Equals(kParentDir)) {
      return false;
    }
  }

  return true;
}

NS_IMPL_ISUPPORTS0(FilesystemFile)

FilesystemFile::FilesystemFile(const nsString& aPath, bool aIsDirectory)
  : mIsDirectory(aIsDirectory)
{
  // Remove the trailing "/".
  if (aPath.Last() == kSeparatorChar) {
    mPath = Substring(aPath, 0, aPath.Length() - 1);
  } else {
    mPath = aPath;
  }
}

FilesystemFile::~FilesystemFile()
{
}

void
FilesystemFile::GetName(nsAString& aName) const
{
  aName = Substring(mPath, mPath.RFindChar(kSeparatorChar) + 1);
}

JS::Value
FilesystemFile::ToJsValue(JSContext* cx, FilesystemBase* aFilesystem) const
{
  if (IsDirectory()) {
    nsRefPtr<Directory> dir =
      new Directory(aFilesystem, const_cast<FilesystemFile *>(this));
    return FilesystemUtils::WrapperCacheObjectToJsval(cx,
                                                      aFilesystem->GetWindow(),
                                                      dir);
  } else {
    nsCOMPtr<nsIFile> file = aFilesystem->GetLocalFile(mPath);
    if (NS_WARN_IF(!file)) {
      return JSVAL_NULL;
    }
    nsCOMPtr<nsIDOMFile> domFile = new nsDOMFileFile(file);
    return FilesystemUtils::InterfaceToJsval(cx,
                                             aFilesystem->GetWindow(),
                                             domFile,
                                             &NS_GET_IID(nsIDOMFile));
  }
}

} // namespace dom
} // namespace mozilla
