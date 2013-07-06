package org.opensplice.cm.qos.holder;

public class QosHolder {
    private String name = null;

    public QosHolder(String name) {
        this.name = name;
    }

    @Override
    public String toString() {
        return name;
    }
}
