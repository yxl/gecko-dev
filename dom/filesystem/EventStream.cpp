/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "EventStream.h"
#include "mozilla/dom/DirectoryBinding.h"
#include "mozilla/dom/Promise.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(EventStream)
NS_IMPL_CYCLE_COLLECTING_ADDREF(EventStream)
NS_IMPL_CYCLE_COLLECTING_RELEASE(EventStream)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(EventStream)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

EventStream::EventStream()
{
  SetIsDOMBinding();
}

EventStream::~EventStream()
{
}

EventStream*
EventStream::GetParentObject() const
{
  return nullptr;
}

JSObject*
EventStream::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aScope)
{
  return EventStreamBinding::Wrap(aCx, aScope, this);
}

already_AddRefed<Promise>
EventStream::ForEach(ForEachCallback& callback)
{
  return nullptr;
}

} // namespace dom
} // namespace mozilla
