/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "mozilla/dom/FilesystemRequestParent.h"

#include "CreateDirectoryTask.h"
#include "GetFileOrDirectoryTask.h"

#include "mozilla/AppProcessChecker.h"
#include "mozilla/dom/FilesystemBase.h"

namespace mozilla {
namespace dom {

FilesystemRequestParent::FilesystemRequestParent()
{
}

FilesystemRequestParent::~FilesystemRequestParent()
{
}

bool
FilesystemRequestParent::Dispatch(ContentParent* aParent,
                                  const FilesystemParams& aParams)
{
  MOZ_ASSERT(aParent, "aParent should not be null.");
  nsRefPtr<FilesystemTaskBase> task;
  switch (aParams.type()) {

    case FilesystemParams::TFilesystemCreateDirectoryParams: {
      const FilesystemCreateDirectoryParams& p = aParams;
      mFilesystem = FilesystemBase::FromString(p.filesystem());
      task = new CreateDirectoryTask(mFilesystem, p, this);
      break;
    }

    case FilesystemParams::TFilesystemGetFileOrDirectoryParams: {
      const FilesystemGetFileOrDirectoryParams& p = aParams;
      mFilesystem = FilesystemBase::FromString(p.filesystem());
      task  = new GetFileOrDirectoryTask(mFilesystem, p, this);
      break;
    }

    default: {
      NS_RUNTIMEABORT("not reached");
      break;
    }
  }

  if (NS_WARN_IF(!task || !mFilesystem)) {
    // Should never reach here.
    return false;
  }

  if (!mFilesystem->IsTesting()) {
    // Check the content process permission.

    nsCString access;
    task->GetPermissionAccessType(access);

    nsAutoCString permissionName;
    permissionName = mFilesystem->GetPermission();
    permissionName.AppendLiteral("-");
    permissionName.Append(access);

    if (!AssertAppProcessPermission(aParent, permissionName.get())) {
      return false;
    }
  }

  task->Start();
  return true;
}

} // namespace dom
} // namespace mozilla
