/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_Directory_h__
#define mozilla_dom_Directory_h__

#include "mozilla/Attributes.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsAutoPtr.h"
#include "nsDOMFile.h"

namespace mozilla {
namespace dom {

class AbortableProgressPromise;
class CreateFileOptions;
class EventStream;
class FilesystemBase;
class FilesystemWeakRef;
class OpenWriteOptions;
class Promise;
class StringOrDirectoryOrDestinationDict;
class StringOrFileOrDirectory;
class StringOrFile;

class Directory MOZ_FINAL
  : public nsISupports
  , public nsWrapperCache
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(Directory)

public:
  static already_AddRefed<Promise>
  GetRoot(FilesystemBase* aFilesystem);

  Directory(FilesystemBase* aFilesystem, const nsAString& aPath);
  ~Directory();

  // ========= Begin WebIDL bindings. ===========

  nsPIDOMWindow*
  GetParentObject() const;

  virtual JSObject*
  WrapObject(JSContext* aCx, JS::Handle<JSObject*> aScope) MOZ_OVERRIDE;

  void
  GetName(nsString& aRetval) const;

  already_AddRefed<Promise>
  CreateFile(const nsAString& aPath, const CreateFileOptions& aOptions);

  already_AddRefed<Promise>
  CreateDirectory(const nsAString& aPath);

  already_AddRefed<Promise>
  Get(const nsAString& aPath);

  already_AddRefed<AbortableProgressPromise>
  Move(const StringOrFileOrDirectory& aPath,
       const StringOrDirectoryOrDestinationDict& aDest);

  already_AddRefed<Promise>
  Remove(const StringOrFileOrDirectory& aPath);

  already_AddRefed<Promise>
  RemoveDeep(const StringOrFileOrDirectory& aPath);

  already_AddRefed<Promise>
  OpenRead(const StringOrFile& aPath);

  already_AddRefed<Promise>
  OpenWrite(const StringOrFile& aPath, const OpenWriteOptions& aOptions);

  already_AddRefed<EventStream>
  Enumerate(const Optional<nsAString >& aPath);

  already_AddRefed<EventStream>
  EnumerateDeep(const Optional<nsAString >& aPath);

  // =========== End WebIDL bindings.============
private:
  static bool
  IsValidRelativePath(const nsString& aPath);

  /*
   * Convert relative DOM path to the absolute real path.
   * @return true if succeed. false if the DOM path is invalid.
   */
  bool
  DOMPathToRealPath(const nsAString& aPath, nsAString& aRealPath) const;

  /*
   * Get the absolute real path of a sub-directory. If the given directory is
   * not a sub-directory, return false and empty path.
   */
  bool
  GetSubDirectoryRealPath(const Directory& sub, nsAString& aRealPath);

  nsAutoPtr<FilesystemWeakRef> mFilesystem;
  nsString mPath;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_Directory_h__
