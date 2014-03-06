/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/AbortableProgressPromise.h"

#include "mozilla/dom/AbortableProgressPromiseBinding.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_INHERITED_0(AbortableProgressPromise, Promise)
NS_IMPL_ADDREF_INHERITED(AbortableProgressPromise, Promise)
NS_IMPL_RELEASE_INHERITED(AbortableProgressPromise, Promise)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(AbortableProgressPromise)
NS_INTERFACE_MAP_END_INHERITING(Promise)

AbortableProgressPromise::AbortableProgressPromise(nsIGlobalObject* aGlobal)
  : Promise(aGlobal)
{
}

AbortableProgressPromise::~AbortableProgressPromise()
{
}

JSObject*
AbortableProgressPromise::WrapObject(JSContext* aCx,
                                     JS::Handle<JSObject*> aScope)
{
  return AbortableProgressPromiseBinding::Wrap(aCx, aScope, this);
}

void
AbortableProgressPromise::Abort()
{
  // TODO
}

// Mark this as resultNotAddRefed to return raw pointers
already_AddRefed<AbortableProgressPromise>
AbortableProgressPromise::Progress(VoidAnyCallback& aCallback)
{

  NS_ADDREF_THIS();
  return this;
}

void
AbortableProgressPromise::NotifyProgress(JSContext* aCx,
                                         JS::Handle<JS::Value> aValue)
{
  NotifyProgressInternal(aCx, aValue);
}

void
AbortableProgressPromise::NotifyProgressInternal(JSContext* aCx,
                                                 JS::Handle<JS::Value> aValue)
{

}

} // namespace dom
} // namespace mozilla
