/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "FilesystemPermissionRequest.h"

#include "mozilla/dom/FilesystemBase.h"
#include "mozilla/dom/FilesystemTaskBase.h"
#include "mozilla/dom/FilesystemUtils.h"
#include "mozilla/dom/TabChild.h"
#include "nsIDocument.h"
#include "nsPIDOMWindow.h"

namespace mozilla {
namespace dom {

NS_IMPL_ISUPPORTS2(FilesystemPermissionRequest, nsIRunnable, nsIContentPermissionRequest)

// static
void
FilesystemPermissionRequest::RequestForTask(FilesystemTaskBase* aTask)
{
  MOZ_ASSERT(aTask, "aTask should not be null!");
  MOZ_ASSERT(NS_IsMainThread());
  nsRefPtr<FilesystemPermissionRequest> request =
    new FilesystemPermissionRequest(aTask);
  NS_DispatchToCurrentThread(request);
}

FilesystemPermissionRequest::FilesystemPermissionRequest(
  FilesystemTaskBase* aTask)
  : mTask(aTask)
{
  MOZ_ASSERT(mTask, "aTask should not be null!");
  MOZ_ASSERT(NS_IsMainThread());

  mTask->GetPermissionAccessType(mPermissionAccess);

  nsRefPtr<FilesystemBase> filesystem = mTask->GetFilesystem();
  if (!filesystem) {
    return;
  }

  mPermissionType = filesystem->GetPermission();

  mWindow = filesystem->GetWindow();
  if (!mWindow) {
    return;
  }

  nsCOMPtr<nsIDocument> doc = mWindow->GetDoc();
  if (!doc) {
    return;
  }

  mPrincipal = doc->NodePrincipal();
}

FilesystemPermissionRequest::~FilesystemPermissionRequest()
{
}

bool
FilesystemPermissionRequest::Recv__delete__(const bool& allow)
{
  if (allow) {
    Allow();
  } else {
    Cancel();
  }
  return true;
}

void
FilesystemPermissionRequest::IPDLRelease()
{
  Release();
}

NS_IMETHODIMP
FilesystemPermissionRequest::GetTypes(nsIArray** aTypes)
{
  return CreatePermissionArray(mPermissionType, mPermissionAccess, aTypes);
}

NS_IMETHODIMP
FilesystemPermissionRequest::GetPrincipal(nsIPrincipal** aRequestingPrincipal)
{
  NS_IF_ADDREF(*aRequestingPrincipal = mPrincipal);
  return NS_OK;
}

NS_IMETHODIMP
FilesystemPermissionRequest::GetWindow(nsIDOMWindow** aRequestingWindow)
{
  NS_IF_ADDREF(*aRequestingWindow = mWindow);
  return NS_OK;
}

NS_IMETHODIMP
FilesystemPermissionRequest::GetElement(nsIDOMElement** aRequestingElement)
{
  *aRequestingElement = nullptr;
  return NS_OK;
}

NS_IMETHODIMP
FilesystemPermissionRequest::Cancel()
{
  MOZ_ASSERT(NS_IsMainThread());
  mTask->SetError(NS_ERROR_DOM_SECURITY_ERR);
  mTask->Start();
  return NS_OK;
}

NS_IMETHODIMP
FilesystemPermissionRequest::Allow()
{
  MOZ_ASSERT(NS_IsMainThread());
  mTask->Start();
  return NS_OK;
}

NS_IMETHODIMP
FilesystemPermissionRequest::Run()
{
  MOZ_ASSERT(NS_IsMainThread());

  nsRefPtr<FilesystemBase> filesystem = mTask->GetFilesystem();
  if (!filesystem) {
    Cancel();
    return NS_OK;
  }

  if (filesystem->IsTesting()) {
    Allow();
    return NS_OK;
  }

  if (FilesystemUtils::IsParentProcess()) {
    nsCOMPtr<nsIContentPermissionPrompt> prompt
      = do_CreateInstance(NS_CONTENT_PERMISSION_PROMPT_CONTRACTID);
    if (!prompt || NS_FAILED(prompt->Prompt(this))) {
      Cancel();
    }
    return NS_OK;
  }

  if (!mWindow) {
    Cancel();
    return NS_OK;
  }

  // because owner implements nsITabChild, we can assume that it is
  // the one and only TabChild.
  TabChild* child = TabChild::GetFrom(mWindow->GetDocShell());
  if (!child) {
    Cancel();
    return NS_OK;
  }

  // Retain a reference so the object isn't deleted without IPDL's
  // knowledge. Corresponding release occurs in
  // DeallocPContentPermissionRequest.
  AddRef();

  nsTArray<PermissionRequest> permArray;
  permArray.AppendElement(PermissionRequest(mPermissionType, mPermissionAccess));
  child->SendPContentPermissionRequestConstructor(
    this, permArray, IPC::Principal(mPrincipal));

  Sendprompt();
  return NS_OK;
}

} /* namespace dom */
} /* namespace mozilla */
