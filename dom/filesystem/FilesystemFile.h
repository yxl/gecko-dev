/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FilesystemFile_h__
#define mozilla_dom_FilesystemFile_h__

#include "js/Value.h"
#include "nsISupportsUtils.h"
#include "nsString.h"

class nsIDOMFile;
class nsPIDOMWindow;

namespace mozilla {
namespace dom {

class FilesystemBase;

class FilesystemFile MOZ_FINAL : public nsISupports
{
  NS_DECL_THREADSAFE_ISUPPORTS
public:
  static bool IsValidRelativePath(const nsString& aPath);

  FilesystemFile(const nsString& aPath, bool aIsDirectory);
  virtual ~FilesystemFile();

  // The trailing "/" is not included.
  const nsString& GetPath() const { return mPath; }
  bool IsDirectory() const { return mIsDirectory; }
  void GetName(nsAString& aName) const;

  JS::Value ToJsValue(JSContext* cx, FilesystemBase* aFilesystem) const;
private:
  static const PRUnichar kSeparatorChar = PRUnichar('/');

  nsString mPath;
  bool mIsDirectory;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_FilesystemFile_h__
