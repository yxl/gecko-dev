/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

callback VoidAnyCallback = void (optional any value);

/* TODO We use object instead Function.  There is an open issue on WebIDL to
 * have different types for "platform-provided function" and "user-provided
 * function"; for now, we just use "object".
 *
 * The first two arguments are the same as those of PromiseInit. They are
 * callbacks that can be used to reject or resolve the promise.
 * The 3rd argument can be used to notify about progress and has the signature
 * of VoidAnyCallback.  
 */
callback ProgressPromiseInit = void (object resolve, object reject, object progress);

[Constructor(ProgressPromiseInit init, AbortCallback abortCallback),
 CheckPermissions="device-storage:apps device-storage:sdcard device-storage:music device-storage:pictures device-storage:videos"]
interface AbortableProgressPromise : AbortablePromise {
  /*
   * Set progress notification callback to be informed about progress.
   * @return This promise.
   */
  AbortableProgressPromise progress(VoidAnyCallback progressHandler);
};
