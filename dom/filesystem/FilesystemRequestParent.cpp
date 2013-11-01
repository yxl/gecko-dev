/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "FilesystemRequestParent.h"
#include "FilesystemBase.h"
#include "CreateDirectoryTask.h"
#include "GetFileOrDirectoryTask.h"

namespace mozilla {
namespace dom {

NS_IMPL_ADDREF(FilesystemRequestParent)
NS_IMPL_RELEASE(FilesystemRequestParent)

FilesystemRequestParent::FilesystemRequestParent(
  const FilesystemParams& aParams)
  : mParams(aParams)
{
}

FilesystemRequestParent::~FilesystemRequestParent()
{
}

void
FilesystemRequestParent::Dispatch()
{
  nsRefPtr<TaskBase> task;
  switch (mParams.type()) {

    case FilesystemParams::TFilesystemCreateDirectoryParams: {
      FilesystemCreateDirectoryParams& p = mParams;
      mFilesystem = FilesystemBase::FromString(p.filesystem());
      MOZ_ASSERT(mFilesystem, "Filesystem should not be null.");
      task = new CreateDirectoryTask(mFilesystem, p, this);
      break;
    }

    case FilesystemParams::TFilesystemGetFileOrDirectoryParams: {
      FilesystemGetFileOrDirectoryParams& p = mParams;
      mFilesystem = FilesystemBase::FromString(p.filesystem());
      MOZ_ASSERT(mFilesystem, "Filesystem should not be null.");
      task = new GetFileOrDirectoryTask(mFilesystem, p, this);
      break;
    }

    default: {
      NS_RUNTIMEABORT("not reached");
      break;
    }

  }
}

} // namespace dom
} // namespace mozilla
