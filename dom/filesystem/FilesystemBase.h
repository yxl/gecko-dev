/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FilesystemBase_h__
#define mozilla_dom_FilesystemBase_h__

#include "nsWeakReference.h"
#include "nsAutoPtr.h"
#include "nsString.h"

class nsPIDOMWindow; // You need |#include "nsPIDOMWindow.h"| in CPP files.

namespace mozilla {
namespace dom {

/*
 * To make FilesystemBase as a weak reference, so that before the child window
 * is closed and the FilesystemBase is destroyed, we don't need to notify the
 * TaskBase instances, which hold the FilesystemBase reference, to cancel and
 * wait until the instances finish.
 */
class FilesystemBase : public nsSupportsWeakReference
{
  NS_DECL_THREADSAFE_ISUPPORTS
public:
  static already_AddRefed<FilesystemBase> FromString(const nsAString& aString);

  FilesystemBase();

  const nsString& ToString() const { return mString; }

  virtual nsPIDOMWindow* GetWindow() const;

  /*
   * Create nsIFile object with the given real path (absolute DOM path).
   */
  virtual already_AddRefed<nsIFile> GetLocalFile(const nsAString& aRealPath) const = 0;

  /*
   * Get the virtual name of the root directory. This name will be exposed to
   * the content page.
   */
  virtual const nsAString& GetRootName() const = 0;
protected:
  virtual ~FilesystemBase();

  // The string representation of the file system.
  nsString mString;
};

/*
 * Helper class for holding and accessing a weak reference of FilesystemBase.
 */
class FilesystemWeakRef
{
public:
  FilesystemWeakRef(FilesystemBase* aFilesystem);
  ~FilesystemWeakRef();

  already_AddRefed<FilesystemBase> Get();
private:
  nsWeakPtr mFilesystem;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_FilesystemBase_h__
