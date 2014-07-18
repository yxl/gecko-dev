/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_AbortableProgressPromise_h__
#define mozilla_dom_AbortableProgressPromise_h__

#include "mozilla/dom/AbortablePromise.h"

struct JSContext;

namespace mozilla {
namespace dom {

class ProgressPromiseInit;
class VoidAnyCallback;

class AbortableProgressPromise  MOZ_FINAL
  : public AbortablePromise
{
  friend class ProgressCallbackTask;
  friend class WorkerProgressCallbackTask;
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(AbortableProgressPromise, AbortablePromise)

public:
  // Constructor used to create native AbortableProgressPromise with C++.
  AbortableProgressPromise(nsIGlobalObject* aGlobal,
                   NativeAbortableHandler* aAbortable);

  void
  NotifyProgress(const Optional<JS::Handle<JS::Value>>& aValue);

  virtual JSObject*
  WrapObject(JSContext* aCx) MOZ_OVERRIDE;

  static already_AddRefed<AbortableProgressPromise>
  Constructor(const GlobalObject& aGlobal, ProgressPromiseInit& aInit,
              AbortCallback& aAbortCallback, ErrorResult& aRv);

  already_AddRefed<AbortableProgressPromise>
  Progress(VoidAnyCallback& aCallback);

private:
  // Static methods for the ProgressPromiseInit functions.
  static bool
  JSProgressCallback(JSContext *aCx, unsigned aArgc, JS::Value *aVp);

  // Constructor used to create AbortableProgressPromise for JavaScript. It
  // should be called by the static AbortableProgressPromise::Constructor.
  AbortableProgressPromise(nsIGlobalObject* aGlobal);

  virtual
  ~AbortableProgressPromise();

  JSObject*
  CreateProgressFunction(JSContext* aCx, JSObject* aParent);

  void
  NotifyProgressInternal(const Optional<JS::Handle<JS::Value>>& aValue);

  nsTArray<nsRefPtr<VoidAnyCallback> > mProgressCallbacks;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_AbortableProgressPromise_h__
