/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Directory.h"
#include "CreateDirectoryTask.h"
#include "EventStream.h"
#include "FilesystemBase.h"
#include "FilesystemFile.h"
#include "FilesystemUtils.h"
#include "GetFileOrDirectoryTask.h"
#include "MoveTask.h"

#include "nsStringGlue.h"

#include "mozilla/dom/AbortableProgressPromise.h"
#include "mozilla/dom/DirectoryBinding.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/UnionTypes.h"

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
  nsRefPtr<GetFileOrDirectoryTask> task = new GetFileOrDirectoryTask(
    aFilesystem, NS_LITERAL_STRING("/"), NS_OK, true);
  return task->GetPromise();
}

Directory::Directory(FilesystemBase* aFilesystem,
                     FilesystemFile* aFile)
  : mFilesystem(new FilesystemWeakRef(aFilesystem))
  , mFile(aFile)
{
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
Directory::GetName(nsString& retval) const
{
  mFile->GetName(retval);
}

already_AddRefed<Promise>
Directory::CreateFile(const nsAString& path, const CreateFileOptions& options)
{
  return nullptr;
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
  nsRefPtr<CreateDirectoryTask> task =
    new CreateDirectoryTask(fs, realPath, error);
  return task->GetPromise();
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
  nsRefPtr<GetFileOrDirectoryTask> task =
    new GetFileOrDirectoryTask(fs, realPath, error);
  return task->GetPromise();
}

already_AddRefed<AbortableProgressPromise>
Directory::Move(const StringOrFileOrDirectory& path, const StringOrDirectoryOrDestinationDict& dest)
{
  nsresult error = NS_OK;
  nsAutoString srcPath;
  nsAutoString destPath;

  nsRefPtr<FilesystemBase> fs = mFilesystem->Get();

  // Check and get the source path
  if (path.IsString()) {
    if (!DOMPathToRealPath(path.GetAsString(), srcPath)) {
      error = NS_ERROR_DOM_FILESYSTEM_INVALID_PATH_ERR;
      goto parameters_check_done;
    }
  } else if (path.IsFile()) {
    nsCOMPtr<nsIDOMFile> file = path.GetAsFile();
    fs->GetRealPath(file, srcPath);
  } else {
    Directory& dir = path.GetAsDirectory();
    srcPath = dir.mFile->GetPath();
  }
  if (!IsDescendantRealPath(srcPath)) {
    error = NS_ERROR_DOM_FILESYSTEM_NO_MODIFICATION_ALLOWED_ERR;
    goto parameters_check_done;
  }

  // Check and get the destination path.
  if (dest.IsString()) {
    DOMPathToRealPath(dest.GetAsString(), destPath);
  } else if (dest.IsDirectory()) {
    Directory& dir = dest.GetAsDirectory();
    // Use the source file name as the destination file name.
    nsString fileName;
    fileName = Substring(srcPath, srcPath.RFindChar(PRUnichar('/')) + 1);
    dir.DOMPathToRealPath(fileName, destPath);
  } else {
    const DestinationDict& dict = dest.GetAsDestinationDict();
    if (dict.mDir.WasPassed() && dict.mName.WasPassed()) {
      Directory& dir = dict.mDir.Value();
      dir.DOMPathToRealPath(dict.mName.Value(), destPath);
    } else {
      error = NS_ERROR_DOM_FILESYSTEM_INVALID_PARAMETERS_ERR;
      goto parameters_check_done;
    }
  }
  if (destPath.IsEmpty()) {
    error = NS_ERROR_DOM_FILESYSTEM_INVALID_PATH_ERR;
  } else if (StringBeginsWith(destPath + NS_LITERAL_STRING("/"), srcPath)) {
    // Cannot move a directory into its descendant or move an entry to itself.
    error = NS_ERROR_DOM_FILESYSTEM_INVALID_MODIFICATION_ERR;
  }

parameters_check_done:

  nsRefPtr<MoveTask> task = new MoveTask(fs, srcPath, destPath, error);
  return task->GetPromise();
}

already_AddRefed<Promise>
Directory::Remove(const StringOrFileOrDirectory& path)
{
  return nullptr;
}

already_AddRefed<Promise>
Directory::RemoveDeep(const StringOrFileOrDirectory& path)
{
  return nullptr;
}

already_AddRefed<Promise>
Directory::OpenRead(const StringOrFile& path)
{
  return nullptr;
}

already_AddRefed<Promise>
Directory::OpenWrite(const StringOrFile& path, const OpenWriteOptions& options)
{
  return nullptr;
}

already_AddRefed<EventStream>
Directory::Enumerate(const Optional<nsAString >& path)
{
  return nullptr;
}

already_AddRefed<EventStream>
Directory::EnumerateDeep(const Optional<nsAString >& path)
{
  return nullptr;
}

bool
Directory::DOMPathToRealPath(const nsAString& aPath, nsAString& aRealPath)
{
  nsString relativePath;

  // Normalize the DOM path to remove the leading "./" and the trailing "/"
  if (StringBeginsWith(aPath, NS_LITERAL_STRING("./"))) {
    relativePath = Substring(aPath, 2);
  } else {
    relativePath = aPath;
  }
  if (relativePath.IsEmpty()) {
    return false;
  }
  if (StringEndsWith(relativePath, NS_LITERAL_STRING("/"))) {
    relativePath = Substring(relativePath, 0, relativePath.Length() - 1);
  }

  if (!FilesystemFile::IsValidRelativePath(relativePath)) {
    return false;
  }

  aRealPath = mFile->GetPath() + NS_LITERAL_STRING("/") + relativePath;

  return true;
}

bool
Directory::IsDescendantRealPath(const nsAString& aRealPath)
{
  const nsString& root = mFile->GetPath();
  if (StringBeginsWith(aRealPath, root) && aRealPath.Length() > root.Length()) {
    return true;
  }
  return false;
}

} // namespace dom
} // namespace mozilla
