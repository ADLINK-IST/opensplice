package org.opensplice.cm.qos.holder;

import org.opensplice.cm.qos.TopicQoS;

public class TopicQosHolder extends QosHolder implements Comparable<TopicQosHolder> {
    private TopicQoS tq = null;

    public TopicQosHolder(String name, TopicQoS tq) {
        super(name);
        this.tq = tq;
    }

    public TopicQoS getQos() {
        return tq;
    }

    @Override
    public int compareTo(TopicQosHolder tqh) {
        return this.toString().compareTo(tqh.toString());
    }
}
