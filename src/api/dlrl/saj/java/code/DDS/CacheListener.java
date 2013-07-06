/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package DDS;

/**
 * <P>The CacheListener is a callback interface that can be implemented by the application
 * if it wants to be notified of incoming changes to the Cache when it is set in the 
 * updates_enabled mode.</P> 
 * <P>It offers callback methods that signal the beginning of an update round and the end of an 
 * update round. It also offers callback methods that signal a change to the update mode.</P>
 */
public interface CacheListener {

    /**
     * Invoked when a new update round is started on a Cache in the updates_enabled mode.
     */
	void on_begin_updates ();

	/**
	 * Invoked when an update round ends on a Cache in the updates_enabled mode.
	 */
	void on_end_updates ();

	/**
	 * Invoked when the updates_enabled mode is switched off. 
	 */
	void on_updates_disabled ();

	/**
	 * Invoked when the updates_enabled mode is switched on.
	 */
	void on_updates_enabled ();
} // interface CacheListener
