/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "MetroApp.h"
#include "MetroWidget.h"
#include "mozilla/widget/AudioSession.h"
#include "nsIRunnable.h"
#include "MetroUtils.h"
#include "MetroAppShell.h"
#include "nsICommandLineRunner.h"
#include "FrameworkView.h"

using namespace ABI::Windows::ApplicationModel;
using namespace ABI::Windows::ApplicationModel::Core;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::System;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

// XXX move
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "runtimeobject.lib")

// Metro specific XRE methods we call from here on an
// appropriate thread.
extern nsresult XRE_metroStartup();
extern void XRE_metroShutdown();

#ifdef PR_LOGGING
extern PRLogModuleInfo* gWindowsLog;
#endif

namespace mozilla {
namespace widget {
namespace winrt {

ComPtr<FrameworkView> sFrameworkView;
ComPtr<MetroApp> sMetroApp;
ComPtr<ICoreApplication> sCoreApp;

////////////////////////////////////////////////////
// IFrameworkViewSource impl.

// Called after CoreApplication::Run(app)
HRESULT
MetroApp::CreateView(ABI::Windows::ApplicationModel::Core::IFrameworkView **aViewProvider)
{
  // This entry point is called on the metro main thread, but the thread won't be
  // recognized as such until after Initialize is called below. XPCOM has not gone
  // through startup at this point.

  LogFunction();

  sFrameworkView = Make<FrameworkView>(this);
  *aViewProvider = sFrameworkView.Get();
  return !sFrameworkView ? E_FAIL : S_OK;
}

////////////////////////////////////////////////////
// MetroApp impl.

// called after FrameworkView::Run() drops into the event dispatch loop
void
MetroApp::Initialize()
{
  HRESULT hr;
  LogThread();

  static bool xpcomInit;
  if (!xpcomInit) {
    xpcomInit = true;
    Log(L"XPCOM startup initialization began");
    nsresult rv = XRE_metroStartup();
    Log(L"XPCOM startup initialization complete");
    if (NS_FAILED(rv)) {
      Log(L"XPCOM startup initialization failed, bailing. rv=%X", rv);
      CoreExit();
      return;
    }
  }

  sFrameworkView->SetupContracts();

  hr = sCoreApp->add_Suspending(Callback<__FIEventHandler_1_Windows__CApplicationModel__CSuspendingEventArgs_t>(
    this, &MetroApp::OnSuspending).Get(), &mSuspendEvent);
  AssertHRESULT(hr);

  hr = sCoreApp->add_Resuming(Callback<__FIEventHandler_1_IInspectable_t>(
    this, &MetroApp::OnResuming).Get(), &mResumeEvent);
  AssertHRESULT(hr);

  mozilla::widget::StartAudioSession();
}

// Free all xpcom related resources before calling the xre shutdown call.
// Must be called on the metro main thread. Currently called from appshell.
void
MetroApp::ShutdownXPCOM()
{
  LogThread();

  mozilla::widget::StopAudioSession();

  sCoreApp->remove_Suspending(mSuspendEvent);
  sCoreApp->remove_Resuming(mResumeEvent);

  MetroApp::GetView()->ShutdownXPCOM();

  // Shut down xpcom
  XRE_metroShutdown();
}

// Request a shutdown of the application
void
MetroApp::CoreExit()
{
  HRESULT hr;
  ComPtr<ICoreApplicationExit> coreExit;
  HStringReference className(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication);
  hr = GetActivationFactory(className.Get(), coreExit.GetAddressOf());
  NS_ASSERTION(SUCCEEDED(hr), "Activation of ICoreApplicationExit");
  if (SUCCEEDED(hr)) {
    coreExit->Exit();
  }
}

// static
FrameworkView*
MetroApp::GetView()
{
  NS_ASSERTION(sFrameworkView, "view has not been created.");
  return sFrameworkView.Get();
}

////////////////////////////////////////////////////
// MetroApp events

HRESULT
MetroApp::OnSuspending(IInspectable* aSender, ISuspendingEventArgs* aArgs)
{
  LogThread();
  PostSuspendResumeProcessNotification(true);
  return S_OK;
}

HRESULT
MetroApp::OnResuming(IInspectable* aSender, IInspectable* aArgs)
{
  LogThread();
  PostSuspendResumeProcessNotification(false);
  return S_OK;
}

HRESULT
MetroApp::OnAsyncTileCreated(ABI::Windows::Foundation::IAsyncOperation<bool>* aOperation,
                             AsyncStatus aStatus)
{
  Log(L"Async operation status: %d", aStatus);
  return S_OK;
}

// static
void
MetroApp::SetBaseWidget(MetroWidget* aPtr)
{
  LogThread();
  NS_ASSERTION(aPtr, "setting null base widget?");
  aPtr->SetView(sFrameworkView.Get());
  sFrameworkView->SetWidget(aPtr);
}

// static
void
MetroApp::PostSuspendResumeProcessNotification(const bool aIsSuspend)
{
  static bool isSuspend = false;
  if (isSuspend == aIsSuspend) {
    return;
  }
  isSuspend = aIsSuspend;
  MetroUtils::FireObserver(aIsSuspend ? "suspend_process_notification" :
                                        "resume_process_notification");
}

// static
void
MetroApp::PostSleepWakeNotification(const bool aIsSleep)
{
  static bool isSleep = false;
  if (isSleep == aIsSleep) {
    return;
  }
  isSleep = aIsSleep;
  MetroUtils::FireObserver(aIsSleep ? "sleep_notification" :
                                      "wake_notification");
}

} } }

bool
XRE_MetroCoreApplicationRun()
{
  HRESULT hr;
  LogThread();

  using namespace mozilla::widget::winrt;

#ifdef PR_LOGGING
  if (!gWindowsLog) {
    gWindowsLog = PR_NewLogModule("nsWindow");
  }
#endif

  sMetroApp = Make<MetroApp>();

  HStringReference className(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication);
  hr = GetActivationFactory(className.Get(), sCoreApp.GetAddressOf());
  if (FAILED(hr)) {
    LogHRESULT(hr);
    return false;
  }

  sCoreApp->Run(sMetroApp.Get());

  Log(L"Exiting CoreApplication::Run");

  sFrameworkView = nullptr;
  sCoreApp = nullptr;
  sMetroApp = nullptr;

  return true;
}

