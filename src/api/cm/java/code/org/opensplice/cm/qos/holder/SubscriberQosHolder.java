package org.opensplice.cm.qos.holder;

import org.opensplice.cm.qos.SubscriberQoS;

public class SubscriberQosHolder extends QosHolder implements Comparable<SubscriberQosHolder> {
    private SubscriberQoS sq = null;

    public SubscriberQosHolder(String name, SubscriberQoS sq) {
        super(name);
        this.sq = sq;
    }

    public SubscriberQoS getQos() {
        return sq;
    }

    @Override
    public int compareTo(SubscriberQosHolder sqh) {
        return this.toString().compareTo(sqh.toString());
    }
}
