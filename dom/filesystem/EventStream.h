/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_EventStream_h__
#define mozilla_dom_EventStream_h__

#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsAutoPtr.h"

struct JSContext;

namespace mozilla {
namespace dom {

class Promise;
class ForEachCallback;

class EventStream MOZ_FINAL : public nsISupports /* Change nativeOwnership in the binding configuration if you don't want this */,
                              public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(EventStream)

public:
  EventStream();

  ~EventStream();

  // TODO: return something sensible here, and change the return type
  EventStream* GetParentObject() const;

  virtual JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aScope) MOZ_OVERRIDE;

  // Mark this as resultNotAddRefed to return raw pointers
  already_AddRefed<Promise> ForEach(ForEachCallback& callback);
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_EventStream_h__
