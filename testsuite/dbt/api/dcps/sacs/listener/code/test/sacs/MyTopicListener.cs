namespace test.sacs
{
    /// <date>Jun 16, 2005</date>
    public class MyTopicListener : DDS.TopicListener
    {
        public bool onInconsistentTopicCalled = false;

        public DDS.InconsistentTopicStatus ictStatus;

        public virtual void On_inconsistent_topic(DDS.ITopic the_topic, DDS.InconsistentTopicStatus status)
        {
            onInconsistentTopicCalled = true;
            ictStatus = status;
        }

        public virtual void Reset()
        {
            onInconsistentTopicCalled = false;
            ictStatus = new DDS.InconsistentTopicStatus();
        }
    }
}
