/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_AbortableProgressPromise_h__
#define mozilla_dom_AbortableProgressPromise_h__

#include "nsAutoPtr.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/Promise.h"

struct JSContext;

namespace mozilla {
namespace dom {

class VoidAnyCallback;

class AbortableProgressPromise MOZ_FINAL
  : public Promise
{
  virtual
  ~AbortableProgressPromise();

public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(AbortableProgressPromise, Promise)

public:
  AbortableProgressPromise(nsIGlobalObject* aGlobal);

  virtual JSObject*
  WrapObject(JSContext* aCx, JS::Handle<JSObject*> aScope) MOZ_OVERRIDE;

  void
  Abort();

  already_AddRefed<AbortableProgressPromise>
  Progress(VoidAnyCallback& aCallback);

public:
  void
  NotifyProgress(JSContext* aCx, JS::Handle<JS::Value> aValue);

private:
  void
  NotifyProgressInternal(JSContext* aCx, JS::Handle<JS::Value> aValue);
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_AbortableProgressPromise_h__
