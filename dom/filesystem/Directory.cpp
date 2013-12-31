/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Directory.h"
#include "EventStream.h"

#include "nsStringGlue.h"

#include "mozilla/dom/AbortableProgressPromise.h"
#include "mozilla/dom/DirectoryBinding.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(Directory)
NS_IMPL_CYCLE_COLLECTING_ADDREF(Directory)
NS_IMPL_CYCLE_COLLECTING_RELEASE(Directory)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(Directory)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

Directory::Directory()
{
  SetIsDOMBinding();
}

Directory::~Directory()
{
}

nsPIDOMWindow*
Directory::GetParentObject() const
{
  // TODO
  return nullptr;
}

JSObject*
Directory::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aScope)
{
  return DirectoryBinding::Wrap(aCx, aScope, this);
}

void
Directory::GetName(nsString& aRetval) const
{
  aRetval.Truncate();
  // TODO
}

already_AddRefed<Promise>
Directory::CreateFile(const nsAString& aPath, const CreateFileOptions& aOptions)
{
  // TODO
  return nullptr;
}

already_AddRefed<Promise>
Directory::CreateDirectory(const nsAString& aPath)
{
  // TODO
  return nullptr;
}

already_AddRefed<Promise>
Directory::Get(const nsAString& aPath)
{
  // TODO
  return nullptr;
}

already_AddRefed<AbortableProgressPromise>
Directory::Move(const StringOrFileOrDirectory& aPath,
                const StringOrDirectoryOrDestinationDict& aDest)
{
  // TODO
  return nullptr;
}

already_AddRefed<Promise>
Directory::Remove(const StringOrFileOrDirectory& aPath)
{
  // TODO
  return nullptr;
}

already_AddRefed<Promise>
Directory::RemoveDeep(const StringOrFileOrDirectory& aPath)
{
  // TODO
  return nullptr;
}

already_AddRefed<Promise>
Directory::OpenRead(const StringOrFile& aPath)
{
  // TODO
  return nullptr;
}

already_AddRefed<Promise>
Directory::OpenWrite(const StringOrFile& aPath, const OpenWriteOptions& aOptions)
{
  // TODO
  return nullptr;
}

already_AddRefed<EventStream>
Directory::Enumerate(const Optional<nsAString >& aPath)
{
  // TODO
  return nullptr;
}

already_AddRefed<EventStream>
Directory::EnumerateDeep(const Optional<nsAString >& aPath)
{
  // TODO
  return nullptr;
}

} // namespace dom
} // namespace mozilla
