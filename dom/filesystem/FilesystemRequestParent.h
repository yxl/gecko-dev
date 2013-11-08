/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FilesystemRequestParent_h__
#define mozilla_dom_FilesystemRequestParent_h__

#include "mozilla/dom/PFilesystemRequestParent.h"
#include "mozilla/dom/ContentChild.h"
#include "mozilla/dom/ContentParent.h"

namespace mozilla {
namespace dom {

class FilesystemBase;

class FilesystemRequestParent : public PFilesystemRequestParent
{
public:
  FilesystemRequestParent(const FilesystemParams& aParams);
  ~FilesystemRequestParent();

  NS_IMETHOD_(nsrefcnt) AddRef();
  NS_IMETHOD_(nsrefcnt) Release();

  bool IsRunning() { return state() == PFilesystemRequest::__Start; }

  void Dispatch();

  virtual bool RecvAbort() MOZ_OVERRIDE;
private:
  ThreadSafeAutoRefCnt mRefCnt;
  NS_DECL_OWNINGTHREAD
  FilesystemParams mParams;
  nsCOMPtr<FilesystemBase> mFilesystem;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_FilesystemRequestParent_h__
