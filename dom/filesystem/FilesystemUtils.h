/* -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 40 -*- */
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FilesystemUtils_h__
#define mozilla_dom_FilesystemUtils_h__

#include "js/Value.h"
#include "nsStringGlue.h"

class nsPIDOMWindow;
class nsISupports;
class nsWrapperCache;

struct nsID;
typedef nsID nsIID;

namespace mozilla {
namespace dom {

/*
 * This class is for error handling.
 * All methods in this class are static.
 */
class FilesystemUtils
{
public:
  static JS::Value WrapperCacheObjectToJsval(JSContext* cx,
                                             nsPIDOMWindow* aWindow,
                                             nsWrapperCache* aObject);

  static JS::Value InterfaceToJsval(JSContext* cx,
                                    nsPIDOMWindow* aWindow,
                                    nsISupports* aObject,
                                    const nsIID* aIID);

  /*
   * Convert the path separator to "/".
   */
  static void LocalPathToNormalizedPath(const nsAString& aLocal,
                                        nsAString& aNorm);

  /*
   * Convert the normalized path separator "/" to the system dependent path
   * separator, which is "/" on Mac and Linux, and "\" on Windows.
   */
  static void NormalizedPathToLocalPath(const nsAString& aNorm,
                                        nsAString& aLocal);

  static bool IsParentProcess();
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_FilesystemUtils_h__
