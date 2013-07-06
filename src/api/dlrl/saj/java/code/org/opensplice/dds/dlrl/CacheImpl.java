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
package org.opensplice.dds.dlrl;

/**
 * Prismtech implementation of the {@link DDS.Cache} interface.
 */
public final class CacheImpl implements DDS.Cache {

    private long admin;

    /* see DDS.Cache for javadoc */
    public final DDS.CacheUsage cache_usage () throws DDS.AlreadyDeleted{
        return DDS.CacheUsage.from_int(jniCacheUsage());
    }

    /* see DDS.Cache for javadoc */
    public final DDS.DCPSState pubsub_state ()  throws DDS.AlreadyDeleted{
		return DDS.DCPSState.from_int(jniPubsubState());
	}

    /* see DDS.Cache for javadoc */
    public final DDS.Publisher the_publisher ()  throws DDS.AlreadyDeleted{
        return jniThePublisher();
    }

    /* see DDS.Cache for javadoc */
    public final DDS.DomainParticipant the_participant ()  throws DDS.AlreadyDeleted{
        return jniTheParticipant();
    }

    /* see DDS.Cache for javadoc */
    public final DDS.Subscriber the_subscriber ()  throws DDS.AlreadyDeleted{
        return jniTheSubscriber();
    }

    /* see DDS.Cache for javadoc */
    public final boolean updates_enabled ()  throws DDS.AlreadyDeleted{
        return jniUpdatesEnabled();
    }

    /* see DDS.Cache for javadoc */
    public final DDS.ObjectHome[] homes ()  throws DDS.AlreadyDeleted{
        return jniHomes();
    }

    /* see DDS.Cache for javadoc */
    public final DDS.CacheAccess[] sub_accesses ()  throws DDS.AlreadyDeleted{
        return jniSubAccesses();
    }

    /* see DDS.Cache for javadoc */
    public final DDS.CacheListener[] listeners ()  throws DDS.AlreadyDeleted{
        return jniListeners();
    }

    /* see DDS.Cache for javadoc */
    public final DDS.CacheKind kind() throws DDS.AlreadyDeleted{
        return DDS.CacheKind.CACHE_KIND;
    }

    /* see DDS.Cache for javadoc */
    public final void register_all_for_pubsub () throws   DDS.BadHomeDefinition,
                                                    DDS.DCPSError,
                                                    DDS.AlreadyDeleted,
                                                    DDS.PreconditionNotMet {
        jniRegisterAllForPubsub();
    }

    /* see DDS.Cache for javadoc */
    public final void enable_all_for_pubsub () throws DDS.DCPSError,
                                                DDS.AlreadyDeleted,
                                                DDS.PreconditionNotMet {
        jniEnableAllForPubsub();
    }

    /* see DDS.Cache for javadoc */
    public final int register_home (DDS.ObjectHome home) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
        return jniRegisterHome(home);
    }

    /* see DDS.CacheBase for javadoc */
    public final DDS.ObjectRoot[] objects() throws DDS.AlreadyDeleted{
        return jniObjects();
    }

    /* see DDS.Cache for javadoc */
    public final DDS.ObjectHome find_home_by_name (String class_name) throws DDS.AlreadyDeleted {
        return jniFindHomeByName(class_name);
    }

    /* see DDS.Cache for javadoc */
    public final DDS.ObjectHome find_home_by_index (int index) throws DDS.AlreadyDeleted {
        return jniFindHomeByIndex(index);
    }

    /* see DDS.Cache for javadoc */
    public final boolean attach_listener (DDS.CacheListener listener) throws DDS.AlreadyDeleted{
        return jniAttachListener(listener);
    }

    /* see DDS.Cache for javadoc */
    public final boolean detach_listener (DDS.CacheListener listener)  throws DDS.AlreadyDeleted{
        return jniDetachListener(listener);
    }

    /* see DDS.Cache for javadoc */
    public final void enable_updates ()  throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
        jniEnableUpdates();
    }

    /* see DDS.Cache for javadoc */
    public final void disable_updates ()  throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
        jniDisableUpdates();
    }

    /* see DDS.Cache for javadoc */
    public final DDS.CacheAccess create_access (DDS.CacheUsage purpose) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted {
        return jniCreateAccess(new org.opensplice.dds.dlrl.CacheAccessImpl(purpose), purpose.value());
    }

    /* see DDS.Cache for javadoc */
    public final void delete_access (DDS.CacheAccess access) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted {
        jniDeleteAccess(access);
    }

    /* see DDS.CacheBase for javadoc */
    public final void refresh () throws DDS.DCPSError, DDS.AlreadyDeleted, DDS.PreconditionNotMet{
        jniRefresh();
    }

    private native int jniCacheUsage() throws DDS.AlreadyDeleted;
    private native int jniPubsubState() throws DDS.AlreadyDeleted;
    private native DDS.DomainParticipant jniTheParticipant() throws DDS.AlreadyDeleted;
    private native DDS.Publisher jniThePublisher() throws DDS.AlreadyDeleted;
    private native DDS.Subscriber jniTheSubscriber() throws DDS.AlreadyDeleted;
    private native boolean jniUpdatesEnabled() throws DDS.AlreadyDeleted;
    private native DDS.ObjectHome[] jniHomes() throws DDS.AlreadyDeleted;
    private native DDS.CacheAccess[] jniSubAccesses() throws DDS.AlreadyDeleted;
    private native DDS.CacheListener[] jniListeners() throws DDS.AlreadyDeleted;
    private native void jniRegisterAllForPubsub() throws DDS.BadHomeDefinition, DDS.DCPSError, DDS.AlreadyDeleted;
    private native void jniEnableAllForPubsub() throws DDS.DCPSError, DDS.AlreadyDeleted;
    private native int jniRegisterHome(DDS.ObjectHome a_home) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;
    private native DDS.ObjectHome jniFindHomeByName(String class_name) throws DDS.AlreadyDeleted;
    private native DDS.ObjectHome jniFindHomeByIndex(int index) throws DDS.AlreadyDeleted;
    private native boolean jniAttachListener(DDS.CacheListener listener) throws DDS.AlreadyDeleted;
    private native boolean jniDetachListener(DDS.CacheListener listener) throws DDS.AlreadyDeleted;
    private native void jniEnableUpdates() throws DDS.AlreadyDeleted;
    private native void jniDisableUpdates() throws DDS.AlreadyDeleted;
    private native DDS.CacheAccess jniCreateAccess(org.opensplice.dds.dlrl.CacheAccessImpl access,
												   int purpose) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted;
    private native void jniDeleteAccess(DDS.CacheAccess access) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted;
    private native void jniRefresh() throws DDS.DCPSError, DDS.AlreadyDeleted;
    protected native void jniDeleteCache();
    private native DDS.ObjectRoot[] jniObjects() throws DDS.AlreadyDeleted;

	private final static DDS.Publisher createPublisher(DDS.DomainParticipant participant){
        //create the publisher
		DDS.PublisherQosHolder qosHolder = new DDS.PublisherQosHolder();
		participant.get_default_publisher_qos(qosHolder);
        qosHolder.value.entity_factory.autoenable_created_entities = false;
        DDS.Publisher publisher = participant.create_publisher (qosHolder.value, null, 0);
		return publisher;
	}

	private final static DDS.Subscriber createSubscriber(DDS.DomainParticipant participant){
        //create the subscriber
		DDS.SubscriberQosHolder qosHolder = new DDS.SubscriberQosHolder();
		participant.get_default_subscriber_qos(qosHolder);
        qosHolder.value.entity_factory.autoenable_created_entities = false;
		DDS.Subscriber subscriber = participant.create_subscriber (qosHolder.value, null, 0);
        return subscriber;
	}

    private final static int deletePublisher(DDS.DomainParticipant participant, DDS.Publisher publisher){
        return participant.delete_publisher(publisher);
    }

    private final static int deleteSubscriber(DDS.DomainParticipant participant, DDS.Subscriber subscriber){
        return participant.delete_subscriber(subscriber);
    }

    private final void triggerListenersStartUpdates(DDS.CacheListener listeners[]){
        int length = listeners.length;
        for(int count = 0; count  < length; count++){
            DDS.CacheListener aListener = listeners[count];
            aListener.on_begin_updates();
        }
    }

    private final void triggerListenersEndUpdates(DDS.CacheListener listeners[]){
        int length = listeners.length;
        for(int count = 0; count  < length; count++){
            DDS.CacheListener aListener = listeners[count];
            aListener.on_end_updates();
        }
    }

    private final void triggerListenersUpdatesEnabled(DDS.CacheListener listeners[]){
        int length = listeners.length;
        for(int count = 0; count  < length; count++){
            DDS.CacheListener aListener = listeners[count];
            aListener.on_updates_enabled();
        }
    }

    private final void triggerListenersUpdatesDisabled(DDS.CacheListener listeners[]){
        int length = listeners.length;
        for(int count = 0; count  < length; count++){
            DDS.CacheListener aListener = listeners[count];
            aListener.on_updates_disabled();
        }
    }

    protected void finalize(){
	    jniDeleteCache();
    }
}