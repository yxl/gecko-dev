/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FilesystemRequestParent_h
#define mozilla_dom_FilesystemRequestParent_h

#include "mozilla/dom/PFilesystemRequestParent.h"
#include "mozilla/dom/ContentChild.h"
#include "mozilla/dom/ContentParent.h"

namespace mozilla {
namespace dom {

class FilesystemBase;

class FilesystemRequestParent
  : public PFilesystemRequestParent
{
  NS_INLINE_DECL_THREADSAFE_REFCOUNTING(FilesystemRequestParent)
public:
  FilesystemRequestParent();

  virtual
  ~FilesystemRequestParent();

  bool
  IsRunning()
  {
    return state() == PFilesystemRequest::__Start;
  }

  bool
  Dispatch(ContentParent* aParent, const FilesystemParams& aParams);
private:
  nsRefPtr<FilesystemBase> mFilesystem;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_FilesystemRequestParent_h
