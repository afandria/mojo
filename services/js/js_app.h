// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_JS_JS_APP_H_
#define SERVICES_JS_JS_APP_H_

#include "gin/public/isolate_holder.h"
#include "gin/shell_runner.h"
#include "mojo/application/content_handler_factory.h"
#include "mojo/public/cpp/system/message_pipe.h"
#include "mojo/public/interfaces/application/application.mojom.h"
#include "mojo/public/interfaces/application/shell.mojom.h"
#include "services/js/js_app_runner_delegate.h"
#include "v8/include/v8.h"

namespace js {

class JSApp;
class JSAppMessageLoopObservers;
class ApplicationDelegateImpl;

// Each JavaScript app started by the content handler runs on its own thread
// and in its own V8 isolate. This class represents one running JS app.

class JSApp : public mojo::InterfaceImpl<mojo::Application>,
              public mojo::ContentHandlerFactory::HandledApplicationHolder {
 public:
  JSApp(mojo::ShellPtr shell, mojo::URLResponsePtr response);
  virtual ~JSApp();

  // This method delegates to shell_->ConnectToApplication().
  void ConnectToApplication(const std::string& application_url,
                            v8::Handle<v8::Value> service_provider);
  void Quit();

 private:
  static const char kMainModuleName[];

  void OnAppLoaded(std::string url, v8::Handle<v8::Value> module);

  // Application methods:
  void AcceptConnection(const mojo::String& requestor_url,
                        mojo::ServiceProviderPtr provider) override;
  void Initialize(mojo::Array<mojo::String> args) override;

  void CallAppInstanceMethod(
      const std::string& name, int argc, v8::Handle<v8::Value> argv[]);

  void QuitInternal();

  mojo::ShellPtr shell_;
  JSAppRunnerDelegate runner_delegate_;
  gin::IsolateHolder isolate_holder_;
  scoped_ptr<gin::ShellRunner> shell_runner_;
  v8::Persistent<v8::Object> app_instance_;
  scoped_ptr<JSAppMessageLoopObservers> message_loop_observers_;

  DISALLOW_COPY_AND_ASSIGN(JSApp);
};

}  // namespace js

#endif  // SERVICES_JS_JS_APP_H_
