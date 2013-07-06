package DDS;

public final class ReaderLifespanQosPolicy {

    public boolean use_lifespan;
    public Duration_t duration = null;

    public ReaderLifespanQosPolicy() {
    }

    public ReaderLifespanQosPolicy(
        boolean _use_lifespan,
        Duration_t _duration)
    {
        use_lifespan = _use_lifespan;
        duration = _duration;
    }

}
