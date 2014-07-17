/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/AbortableProgressPromise.h"

#include "AbortableCallback.h"
#include "mozilla/dom/AbortableProgressPromiseBinding.h"
#include "mozilla/dom/NativeAbortableHandler.h"
#include "mozilla/ErrorResult.h"
#include "PromiseCallback.h"

namespace mozilla {
namespace dom {

NS_IMPL_ADDREF_INHERITED(AbortableProgressPromise, Promise)
NS_IMPL_RELEASE_INHERITED(AbortableProgressPromise, Promise)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(AbortableProgressPromise)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END_INHERITING(AbortablePromise)
NS_IMPL_CYCLE_COLLECTION_INHERITED(AbortableProgressPromise, AbortablePromise, mAbortableCallback)

AbortableProgressPromise::AbortableProgressPromise(nsIGlobalObject* aGlobal,
                                   NativeAbortableHandler* aAbortable)
  : AbortablePromise(aGlobal, aAbortable)
{
}

AbortableProgressPromise::AbortableProgressPromise(nsIGlobalObject* aGlobal)
  : AbortablePromise(aGlobal)
{
}

AbortableProgressPromise::~AbortableProgressPromise()
{
}

JSObject*
AbortableProgressPromise::WrapObject(JSContext* aCx)
{
  return AbortableProgressPromiseBinding::Wrap(aCx, this);
}

/* static */ already_AddRefed<AbortableProgressPromise>
AbortableProgressPromise::Constructor(const GlobalObject& aGlobal,
                                      ProgressPromiseInit& aInit,
                                      AbortCallback& aAbortCallback,
                                      ErrorResult& aRv)
{
  JSContext* cx = aGlobal.Context();

  nsCOMPtr<nsIGlobalObject> global;
  global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }

  nsRefPtr<AbortableProgressPromise> promise = new AbortableProgressPromise(global);
  JS::Rooted<JSObject*> resolveFunc(cx,
                                    CreateFunction(cx, aGlobal.Get(), promise,
                                    PromiseCallback::Resolve));
  if (!resolveFunc) {
    aRv.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }

  JS::Rooted<JSObject*> rejectFunc(cx,
                                   CreateFunction(cx, aGlobal.Get(), promise,
                                   PromiseCallback::Reject));
  if (!rejectFunc) {
    aRv.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }

  aInit.Call(resolveFunc, rejectFunc, aRv, CallbackObject::eRethrowExceptions);
  aRv.WouldReportJSException();

  if (aRv.IsJSException()) {
    JS::Rooted<JS::Value> value(cx);
    aRv.StealJSException(cx, &value);

    // we want the same behavior as this JS implementation:
    // function Promise(arg) { try { arg(a, b); } catch (e) { this.reject(e); }}
    if (!JS_WrapValue(cx, &value)) {
      aRv.Throw(NS_ERROR_UNEXPECTED);
      return nullptr;
    }

    promise->MaybeRejectInternal(cx, value);
  }

  promise->mAbortableCallback = new WrapperAbortableCallback(&aAbortCallback);
  return promise.forget();
}

already_AddRefed<AbortableProgressPromise>
AbortableProgressPromise::Progress(VoidAnyCallback& callback)
{
  return nsRefPtr<AbortableProgressPromise>(this).forget();
}

} // namespace dom
} // namespace mozilla
