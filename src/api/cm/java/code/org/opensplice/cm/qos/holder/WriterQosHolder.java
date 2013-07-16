package org.opensplice.cm.qos.holder;

import org.opensplice.cm.qos.WriterQoS;

public class WriterQosHolder extends QosHolder implements Comparable<WriterQosHolder> {
    private WriterQoS wq = null;

    public WriterQosHolder(String name, WriterQoS wq) {
        super(name);
        this.wq = wq;
    }

    public WriterQoS getQos() {
        return wq;
    }

    @Override
    public int compareTo(WriterQosHolder wqh) {
        return this.toString().compareTo(wqh.toString());
    }
}

