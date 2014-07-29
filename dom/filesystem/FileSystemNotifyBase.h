/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FileSystemNotifyBase_h
#define mozilla_dom_FileSystemNotifyBase_h

#include "mozilla/Attributes.h"
#include "mozilla/dom/ipc/Blob.h"
#include "mozilla/dom/FileSystemRequestParent.h"

namespace mozilla {
namespace dom {

class DOMFileImpl;

class FileSystemNotifyBase
  : public nsRunnable
{
public:
  nsRefPtr<FileSystemRequestParent> mRequestParent;

  /*
   * To create a task to handle the page content request.
   */
  FileSystemNotifyBase(FileSystemRequestParent* aParent,
             const nsString& aNotifyString);

  /*
   * To create a task to handle the page content request.
   */
  FileSystemNotifyBase(FileSystemRequestParent* aParent,
             nsRefPtr<DOMFileImpl>& aNotifyFileImpl);

  virtual
  ~FileSystemNotifyBase();

  NS_DECL_NSIRUNNABLE
protected:

private:
  nsString mNotifyString;
  nsRefPtr<DOMFileImpl> mNotifyFileImpl;
  
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_FileSystemNotifyBase_h
