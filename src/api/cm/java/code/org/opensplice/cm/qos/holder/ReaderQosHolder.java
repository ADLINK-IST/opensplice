package org.opensplice.cm.qos.holder;

import org.opensplice.cm.qos.ReaderQoS;

public class ReaderQosHolder extends QosHolder implements Comparable<ReaderQosHolder> {
    private ReaderQoS rq = null;

    public ReaderQosHolder(String name, ReaderQoS wq) {
        super(name);
        this.rq = wq;
    }

    public ReaderQoS getQos() {
        return rq;
    }

    @Override
    public int compareTo(ReaderQosHolder rqh) {
        return this.toString().compareTo(rqh.toString());
    }
}
