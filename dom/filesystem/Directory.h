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
class OpenWriteOptions;
class Promise;
class StringOrDirectoryOrDestinationDict;
class StringOrFileOrDirectory;
class StringOrFile;

class Directory MOZ_FINAL : public nsISupports,
                            public nsWrapperCache
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(Directory)

public:
  Directory();
  ~Directory();

  // ========= Begin WebIDL bindings. ===========

  nsPIDOMWindow* GetParentObject() const;

  virtual JSObject*
  WrapObject(JSContext* aCx, JS::Handle<JSObject*> aScope) MOZ_OVERRIDE;

  void GetName(nsString& retval) const;

  already_AddRefed<Promise> CreateFile(const nsAString& path, const CreateFileOptions& options);

  already_AddRefed<Promise> CreateDirectory(const nsAString& path);

  already_AddRefed<Promise> Get(const nsAString& aPath);

  already_AddRefed<AbortableProgressPromise> Move(const StringOrFileOrDirectory& path, const StringOrDirectoryOrDestinationDict& dest);

  already_AddRefed<Promise> Remove(const StringOrFileOrDirectory& path);

  already_AddRefed<Promise> RemoveDeep(const StringOrFileOrDirectory& path);

  already_AddRefed<Promise> OpenRead(const StringOrFile& path);

  already_AddRefed<Promise> OpenWrite(const StringOrFile& path, const OpenWriteOptions& options);

  // Mark this as resultNotAddRefed to return raw pointers
  already_AddRefed<EventStream> Enumerate(const Optional<nsAString >& path);

  // Mark this as resultNotAddRefed to return raw pointers
  already_AddRefed<EventStream> EnumerateDeep(const Optional<nsAString >& path);

  // =========== End WebIDL bindings.============
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_Directory_h__
