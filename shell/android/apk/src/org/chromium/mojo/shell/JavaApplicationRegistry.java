// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.mojo.shell;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.mojo.application.ApplicationConnection;
import org.chromium.mojo.application.ApplicationDelegate;
import org.chromium.mojo.application.ApplicationRunner;
import org.chromium.mojo.application.ServiceFactoryBinder;
import org.chromium.mojo.system.MessagePipeHandle;
import org.chromium.mojo.system.impl.CoreImpl;
import org.chromium.mojom.mojo.Shell;

import java.util.HashMap;
import java.util.Map;

/**
 * Utility class that allows to register java application bundled with the shell.
 * <p>
 * To add a new application, add your registerService() call in the create() method of this class
 * with the URL of the application to register and then ServiceFactoryBinder for the registered
 * service.
 */
@JNINamespace("shell")
public class JavaApplicationRegistry {
    private final Map<String, ApplicationDelegate> mApplicationDelegateMap = new HashMap<>();

    private static final class ApplicationRunnable implements Runnable {
        private final ApplicationDelegate mApplicationDelegate;
        private final MessagePipeHandle mApplicationRequestHandle;

        private ApplicationRunnable(ApplicationDelegate applicationDelegate,
                MessagePipeHandle applicationRequestHandle) {
            mApplicationDelegate = applicationDelegate;
            mApplicationRequestHandle = applicationRequestHandle;
        }

        /**
         * @see Runnable#run()
         */
        @Override
        public void run() {
            ApplicationRunner.run(
                    mApplicationDelegate, CoreImpl.getInstance(), mApplicationRequestHandle);
        }
    }

    private JavaApplicationRegistry() {}

    private void registerApplicationDelegate(String url, ApplicationDelegate applicationDelegate) {
        mApplicationDelegateMap.put(url, applicationDelegate);
    }

    @CalledByNative
    private String[] getApplications() {
        return mApplicationDelegateMap.keySet().toArray(new String[mApplicationDelegateMap.size()]);
    }

    @CalledByNative
    private void startApplication(String url, int applicationRequestHandle) {
        MessagePipeHandle messagePipeHandle = CoreImpl.getInstance()
                                                      .acquireNativeHandle(applicationRequestHandle)
                                                      .toMessagePipeHandle();
        ApplicationDelegate applicationDelegate = mApplicationDelegateMap.get(url);
        if (applicationDelegate != null) {
            new Thread(new ApplicationRunnable(applicationDelegate, messagePipeHandle)).start();
        } else {
            messagePipeHandle.close();
        }
    }

    @CalledByNative
    private static JavaApplicationRegistry create() {
        JavaApplicationRegistry registry = new JavaApplicationRegistry();
        // Register services.
        registry.registerApplicationDelegate("mojo:keyboard",
                new ServiceProviderFactoryApplicationDelegate(new KeyboardFactory()));
        registry.registerApplicationDelegate(
                "mojo:intent_receiver", new ServiceProviderFactoryApplicationDelegate(
                                                IntentReceiverRegistry.getInstance()));
        registry.registerApplicationDelegate("mojo:nfc", new NfcApplicationDelegate());
        registry.registerApplicationDelegate("mojo:sharing", new SharingApplicationDelegate());
        registry.registerApplicationDelegate(
                "mojo:native_viewport_support", new NativeViewportSupportApplicationDelegate());
        return registry;
    }

    static class ServiceProviderFactoryApplicationDelegate implements ApplicationDelegate {
        private final ServiceFactoryBinder<?> mBinder;

        ServiceProviderFactoryApplicationDelegate(ServiceFactoryBinder<?> binder) {
            mBinder = binder;
        }

        /**
         * @see ApplicationDelegate#initialize(Shell, String[], String)
         */
        @Override
        public void initialize(Shell shell, String[] args, String url) {}

        /**
         * @see ApplicationDelegate#configureIncomingConnection(ApplicationConnection)
         */
        @Override
        public boolean configureIncomingConnection(ApplicationConnection connection) {
            connection.addService(mBinder);
            return true;
        }

        /**
         * @see ApplicationDelegate#quit()
         */
        @Override
        public void quit() {}
    }
}
