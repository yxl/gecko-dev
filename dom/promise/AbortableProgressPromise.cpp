/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/AbortableProgressPromise.h"

#include "AbortableCallback.h"
#include "mozilla/dom/AbortableProgressPromiseBinding.h"
#include "mozilla/ErrorResult.h"
#include "PromiseCallback.h"
#include "WorkerPrivate.h"
#include "WorkerRunnable.h"

namespace mozilla {
namespace dom {

using namespace workers;

class ProgressCallbackTask MOZ_FINAL
  : public nsRunnable
{
public:
  ProgressCallbackTask(AbortableProgressPromise* aPromise,
                       const Optional<JS::Handle<JS::Value> >& aValue)
    : mPromise(aPromise)
  {
    MOZ_ASSERT(aPromise);
    MOZ_COUNT_CTOR(PromiseTask);
    if (aValue.WasPassed()) {
      mValue.Value() = aValue.Value();
    }
  }

  NS_IMETHOD Run()
  {
    Optional<JS::Handle<JS::Value> > value;
    if (mValue.WasPassed()) {
      value.Value() = mValue.Value();
    }
    mPromise->NotifyProgressInternal(value);
    return NS_OK;
  }

private:
  ~ProgressCallbackTask()
  {
    MOZ_COUNT_DTOR(PromiseTask);
  }

  nsRefPtr<AbortableProgressPromise> mPromise;
  Optional<JS::PersistentRooted<JS::Value> > mValue;
};

class WorkerProgressCallbackTask MOZ_FINAL
  : public WorkerSameThreadRunnable
{
public:
  WorkerProgressCallbackTask(WorkerPrivate* aWorkerPrivate,
                             AbortableProgressPromise* aPromise,
                             const Optional<JS::Handle<JS::Value>>& aValue)
    : WorkerSameThreadRunnable(aWorkerPrivate)
    , mPromise(aPromise)
  {
    MOZ_ASSERT(aPromise);
    MOZ_COUNT_CTOR(WorkerPromiseTask);
    if (aValue.WasPassed()) {
      mValue.Value() = aValue.Value();
    }
  }

  bool
  WorkerRun(JSContext* aCx, WorkerPrivate* aWorkerPrivate)
  {
    Optional<JS::Handle<JS::Value> > value;
    if (mValue.WasPassed()) {
      value.Value() = mValue.Value();
    }
    mPromise->NotifyProgressInternal(value);
    return true;
  }

private:
  ~WorkerProgressCallbackTask()
  {
    MOZ_COUNT_DTOR(WorkerPromiseTask);
  }

  nsRefPtr<AbortableProgressPromise> mPromise;
  Optional<JS::PersistentRooted<JS::Value> > mValue;
};

NS_IMPL_ADDREF_INHERITED(AbortableProgressPromise, Promise)
NS_IMPL_RELEASE_INHERITED(AbortableProgressPromise, Promise)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(AbortableProgressPromise)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END_INHERITING(AbortablePromise)
NS_IMPL_CYCLE_COLLECTION_INHERITED(AbortableProgressPromise, AbortablePromise, mProgressCallbacks)

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

void
AbortableProgressPromise::NotifyProgress(
  const Optional<JS::Handle<JS::Value>>& aValue)
{
  if (mState != Pending || mProgressCallbacks.Length() == 0) {
    return;
  }
  if (MOZ_LIKELY(NS_IsMainThread())) {
    nsRefPtr<ProgressCallbackTask> task =
      new ProgressCallbackTask(this, aValue);
    NS_DispatchToCurrentThread(task);
  } else {
    WorkerPrivate* worker = GetCurrentThreadWorkerPrivate();
    MOZ_ASSERT(worker);
    nsRefPtr<WorkerProgressCallbackTask> task =
      new WorkerProgressCallbackTask(worker, this, aValue);
    task->Dispatch(worker->GetJSContext());
  }
}

void
AbortableProgressPromise::NotifyProgressInternal(
  const Optional<JS::Handle<JS::Value>>& aValue)
{
  if (mState != Pending || mProgressCallbacks.Length() == 0) {
    return;
  }

  ErrorResult rv;
  size_t n = mProgressCallbacks.Length();
  for (uint32_t i = 0; i < n; ++i) {
    mProgressCallbacks[i]->Call(aValue, rv);
  }
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

  JS::Rooted<JSObject*> progressFunc(cx,
    promise->CreateProgressFunction(cx, aGlobal.Get()));

  if (!progressFunc) {
    aRv.Throw(NS_ERROR_UNEXPECTED);
    return nullptr;
  }

  aInit.Call(resolveFunc, rejectFunc, progressFunc, aRv,
             CallbackObject::eRethrowExceptions);
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
AbortableProgressPromise::Progress(VoidAnyCallback& aCallback)
{
  mProgressCallbacks.AppendElement(&aCallback);
  return nsRefPtr<AbortableProgressPromise>(this).forget();
}

JSObject*
AbortableProgressPromise::CreateProgressFunction(JSContext* aCx, JSObject* aParent)
{
  JSFunction* func = js::NewFunctionWithReserved(aCx,
                                                 JSProgressCallback,
                                                 1 /* nargs */, 0 /* flags */,
                                                 aParent, nullptr);

  if (!func) {
    return nullptr;
  }

  JS::Rooted<JSObject*> obj(aCx, JS_GetFunctionObject(func));

  JS::Rooted<JS::Value> promiseObj(aCx);
  if (!dom::WrapNewBindingObject(aCx, this, &promiseObj)) {
    return nullptr;
  }

  js::SetFunctionNativeReserved(obj, 0, promiseObj);

  return obj;
}

/* static */ bool
AbortableProgressPromise::JSProgressCallback(JSContext* aCx, unsigned aArgc, JS::Value* aVp)
{
  JS::CallArgs args = CallArgsFromVp(aArgc, aVp);

  JS::Rooted<JS::Value> v(aCx, js::GetFunctionNativeReserved(&args.callee(),0));
  MOZ_ASSERT(v.isObject());

  AbortableProgressPromise* promise;
  if (NS_FAILED(UNWRAP_OBJECT(AbortableProgressPromise, &v.toObject(), promise))) {
    return Throw(aCx, NS_ERROR_UNEXPECTED);
  }
  Optional<JS::Handle<JS::Value>> value;
  if (args.length() > 0) {
    value.Value() = args.get(0);
  }
  promise->NotifyProgress(value);

  return true;
}

} // namespace dom
} // namespace mozilla
