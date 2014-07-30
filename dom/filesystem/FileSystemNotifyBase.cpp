/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nsNetUtil.h" // Stream transport service.
#include "mozilla/dom/ContentChild.h"
#include "mozilla/unused.h"
#include "nsDOMFile.h"
#include "FileSystemNotifyBase.h"
#include "FileSystemTaskBase.h"

using mozilla::MonitorAutoLock;

namespace mozilla {
namespace dom {

FileSystemNotifyBase::FileSystemNotifyBase(FileSystemTaskBase* aTask)
  : mTask(aTask)
{
}

FileSystemNotifyBase::~FileSystemNotifyBase()
{
}

NS_IMETHODIMP
FileSystemNotifyBase::Run()
{
  MOZ_ASSERT(NS_IsMainThread(), "Only call on main thread!");
  if (mTask) {
    mTask->HandlerNotify();
  }
  return NS_OK;
}

} // namespace dom
} // namespace mozilla
