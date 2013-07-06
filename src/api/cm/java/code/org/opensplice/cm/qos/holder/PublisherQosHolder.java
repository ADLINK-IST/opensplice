package org.opensplice.cm.qos.holder;

import org.opensplice.cm.qos.PublisherQoS;

public class PublisherQosHolder extends QosHolder implements Comparable<PublisherQosHolder> {
    private PublisherQoS pq = null;

    public PublisherQosHolder(String name, PublisherQoS pq) {
        super(name);
        this.pq = pq;
    }

    public PublisherQoS getQos() {
        return pq;
    }

    @Override
    public int compareTo(PublisherQosHolder pqh) {
        return this.toString().compareTo(pqh.toString());
    }
}
