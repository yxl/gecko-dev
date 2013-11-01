/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FilesystemUtils.h"
#include "nsContentUtils.h"
#include "nsCxPusher.h"
#include "nsIGlobalObject.h"
#include "nsPIDOMWindow.h"
#include "nsWrapperCache.h"
#include "nsXULAppAPI.h"

namespace mozilla {
namespace dom {

// static
JS::Value
FilesystemUtils::WrapperCacheObjectToJsval(JSContext* cx,
                                           nsPIDOMWindow* aWindow,
                                           nsWrapperCache* aObject)
{
  nsCOMPtr<nsIGlobalObject> globalObject = do_QueryInterface(aWindow);
  if (NS_WARN_IF(!globalObject)) {
    return JSVAL_NULL;
  }

  JS::Rooted<JSObject*> global(cx, globalObject->GetGlobalJSObject());

  return OBJECT_TO_JSVAL(aObject->WrapObject(cx, global));
}

// static
JS::Value FilesystemUtils::InterfaceToJsval(JSContext* cx,
                                            nsPIDOMWindow* aWindow,
                                            nsISupports* aObject,
                                            const nsIID* aIID)
{
  nsCOMPtr<nsIGlobalObject> globalObject = do_QueryInterface(aWindow);
  if (NS_WARN_IF(!globalObject)) {
    return JSVAL_NULL;
  }

  JS::Rooted<JSObject*> scopeObj(cx, globalObject->GetGlobalJSObject());
  if (NS_WARN_IF(!scopeObj)) {
    return JSVAL_NULL;
  }

  JSAutoCompartment ac(cx, scopeObj);

  JS::Rooted<JS::Value> someJsVal(cx);
  nsresult rv =
    nsContentUtils::WrapNative(cx, scopeObj, aObject, aIID, &someJsVal);
  if (NS_FAILED(rv)) {
    return JSVAL_NULL;
  }

  return someJsVal;
}

// static
void
FilesystemUtils::LocalPathToNormalizedPath(const nsAString& aLocal, nsAString& aNorm)
{
  nsString result;
  result = aLocal;
#if defined(XP_WIN)
  PRUnichar* cur = result.BeginWriting();
  PRUnichar* end = result.EndWriting();
  for (; cur < end; ++cur) {
    if (PRUnichar('\\') == *cur)
      *cur = PRUnichar('/');
  }
#endif
  aNorm = result;
}

// static
void
FilesystemUtils::NormalizedPathToLocalPath(const nsAString& aNorm, nsAString& aLocal)
{
  nsString result;
  result = aNorm;
#if defined(XP_WIN)
  PRUnichar* cur = result.BeginWriting();
  PRUnichar* end = result.EndWriting();
  for (; cur < end; ++cur) {
    if (PRUnichar('/') == *cur)
      *cur = PRUnichar('\\');
  }
#endif
  aLocal = result;
}

// static
bool
FilesystemUtils::IsParentProcess()
{
  return XRE_GetProcessType() == GeckoProcessType_Default;
}

} // namespace dom
} // namespace mozilla
