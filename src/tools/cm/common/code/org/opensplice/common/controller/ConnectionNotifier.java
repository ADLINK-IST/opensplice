/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package org.opensplice.common.controller;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * ConnectionNotifier is a class that defines the interfaces for ConnectListener
 * and DisconnectListener, as well as manages the list of registered listeners
 * for both kinds of events. The lists of registered connection and
 * disconnection listeners are publicly visible for the purpose of
 * synchronization when using iterators.
 *
 * The lists themselves are instances of
 * {@link Collections#synchronizedList(List) SynchronizedList}, however when
 * using iterators over such lists, it is imperative that the user manually
 * synchronize on a given list when iterating over it:
 *
 * <pre>
 *    synchronized (ConnectionNotifier.connectListeners) {
 *        Iterator i = ConnectionNotifier.connectListeners.iterator(); // Must be in synchronized block
 *        while (i.hasNext())
 *            foo(i.next());
 *    }
 * <pre>
 * Failure to follow this advice may result in non-deterministic behavior.
 *
 * @see Collections#synchronizedList(List)
 */
public class ConnectionNotifier {

    /** the connect listeners. */
    public final static List<ConnectListener> connectListeners =
            Collections.synchronizedList(new ArrayList<ConnectListener>());
    /** the disconnect listeners. */
    public final static List<DisconnectListener> disconnectListeners =
            Collections.synchronizedList(new ArrayList<DisconnectListener>());

    /**
     * Add a connect listener, the listener will be notified upon a connect and
     * on add when already connected.
     *
     * @param listener
     *            the listener to add.
     * @param connected
     *            the current state of DDS domain connection at the time of this
     *            method call. If true, the supplied listener parameter will
     *            have its {@link ConnectListener#onConnect()} method called.
     */
    public static void addConnectListener(final ConnectListener listener, boolean connected) {
        connectListeners.add(listener);
        if (connected) {
            listener.onConnect();
        }
    }

    /**
     * Add a connect listener, the listener will be notified upon a connect.
     *
     * @param listener
     *            the listener to add.
     */
    public static void addConnectListener(final ConnectListener listener) {
        addConnectListener(listener, false);
    }

    /**
     * Add a disconnect listener, the listener will be notified upon a
     * disconnect.
     * @param listener
     *            the listener to add.
     * @param connected
     *            the current state of DDS domain connection at the time of this
     *            method call. If false, the supplied listener parameter will
     *            have its {@link DisconnectListener#onDisonnect()} method called.
     */
    public static void addDisconnectListener(final DisconnectListener listener, boolean connected) {
        disconnectListeners.add(listener);
        if (!connected) {
            listener.onDisconnect(!connected);
        }
    }

    /**
     * Add a disconnect listener, the listener will be notified upon a
     * disconnect.
     *
     * @param listener
     *            the listener to add.
     */
    public static void addDisconnectListener(final DisconnectListener listener) {
        addDisconnectListener(listener, true);
    }

    public static void removeConnectListener(final ConnectListener listener) {
        connectListeners.remove(listener);
    }

    public static void removeDisconnectListener(final DisconnectListener listener) {
        disconnectListeners.remove(listener);
    }

    /**
     * Public interface for a connect event.
     */
    public static interface ConnectListener {
        /** handle a connect event. */
        public void onConnect();
    }

    /**
     * Public interface for a disconnect event.
     */
    public static interface DisconnectListener {
        /** Handle a disconnect event. */
        void onDisconnect(boolean noConnection);
    }

}
