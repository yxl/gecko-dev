/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/AbortablePromise.h"

#include "mozilla/dom/AbortablePromiseBinding.h"
#include "mozilla/dom/PromiseNativeAbortCallback.h"
#include "mozilla/ErrorResult.h"
#include "PromiseCallback.h"

namespace mozilla {
namespace dom {

NS_IMPL_ISUPPORTS0(NativeAbortableHandler)

NS_IMPL_ADDREF_INHERITED(AbortablePromise, Promise)
NS_IMPL_RELEASE_INHERITED(AbortablePromise, Promise)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(AbortablePromise)
NS_INTERFACE_MAP_END_INHERITING(Promise)
NS_IMPL_CYCLE_COLLECTION_INHERITED(AbortablePromise, Promise,
                                   mJSAbortCallback, mNativeAbortCallback)

AbortablePromise::AbortablePromise(nsIGlobalObject* aGlobal,
                                   PromiseNativeAbortCallback& aAbortCallback)
  : Promise(aGlobal)
  , mNativeAbortCallback(&aAbortCallback)
{
}

AbortablePromise::AbortablePromise(nsIGlobalObject* aGlobal)
  : Promise(aGlobal)
{
}

AbortablePromise::~AbortablePromise()
{
}

JSObject*
AbortablePromise::WrapObject(JSContext* aCx)
{
  return AbortablePromiseBinding::Wrap(aCx, this);
}

/* static */ already_AddRefed<AbortablePromise>
AbortablePromise::Constructor(const GlobalObject& aGlobal, PromiseInit& aInit,
                              AbortCallback& aAbortCallback, ErrorResult& aRv)
{
  nsCOMPtr<nsIGlobalObject> global;
  global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }

  nsRefPtr<AbortablePromise> promise = new AbortablePromise(global);
  promise->CreateWrapper(global, aRv);
  if (aRv.Failed()) {
    return nullptr;
  }

  promise->CallInitFunction(aGlobal, aInit, aRv);
  if (aRv.Failed()) {
    return nullptr;
  }

  promise->mJSAbortCallback = &aAbortCallback;

  return promise.forget();
}

void
AbortablePromise::Abort()
{
  if (IsPending()) {
    return;
  }
  MaybeReject(NS_ERROR_ABORT);

  // This class processes the promise's abortable callbacks.
  class AbortCallbackTask MOZ_FINAL : public nsRunnable
  {
  public:
    AbortCallbackTask(AbortablePromise* aPromise)
      : mPromise(aPromise)
    {
      MOZ_ASSERT(aPromise);
      MOZ_COUNT_CTOR(AbortCallbackTask);
    }

  protected:
    ~AbortCallbackTask()
    {
      NS_ASSERT_OWNINGTHREAD(AbortCallbackTask);
      MOZ_COUNT_DTOR(AbortCallbackTask);
    }

  public:
    NS_IMETHOD
    Run() MOZ_OVERRIDE
    {
      NS_ASSERT_OWNINGTHREAD(AbortCallbackTask);
      mPromise->DoAbort();
      return NS_OK;
    }

  private:
    nsRefPtr<AbortablePromise> mPromise;
    NS_DECL_OWNINGTHREAD
  };

  nsRefPtr<AbortCallbackTask> task = new AbortCallbackTask(this);
  Promise::DispatchToMainOrWorkerThread(task);
}

void
AbortablePromise::DoAbort()
{
  if (mJSAbortCallback) {
    ErrorResult rv;
    mJSAbortCallback->Call(rv);
    return;
  }
  mNativeAbortCallback->Call();
}

} // namespace dom
} // namespace mozilla
