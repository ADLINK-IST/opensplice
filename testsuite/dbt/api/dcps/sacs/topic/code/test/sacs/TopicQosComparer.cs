namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class TopicQosComparer
    {
        public static bool TopicQosEqual(DDS.TopicQos qos1, DDS.TopicQos qos2)
        {
            if (!DurationEqual(qos1.Deadline.Period, qos2.Deadline.Period))
            {
                System.Console.Error.WriteLine("'deadline.Period' values do not match");
                return false;
            }
            if (qos1.DestinationOrder.Kind != qos2.DestinationOrder.Kind)
            {
                System.Console.Error.WriteLine("'destination_order.kind' values do not match");
                return false;
            }
            if (qos1.Durability.Kind != qos2.Durability.Kind)
            {
                System.Console.Error.WriteLine("'durability.kind' values do not match");
                return false;
            }
            if (!DurationEqual(qos1.DurabilityService.ServiceCleanupDelay, qos2.DurabilityService.ServiceCleanupDelay))
            {
                System.Console.Error.WriteLine("'durability.ServiceCleanupDelay' values do not match"
                    );
                return false;
            }
            if (qos1.DurabilityService.HistoryKind != qos2.DurabilityService.HistoryKind)
            {
                System.Console.Error.WriteLine("'durability_service.HistoryKind' values do not match"
                    );
                return false;
            }
            if (qos1.DurabilityService.HistoryDepth != qos2.DurabilityService.HistoryDepth)
            {
                System.Console.Error.WriteLine("'durability_service.HistoryDepth' values do not match"
                    );
                return false;
            }
            if (qos1.DurabilityService.MaxInstances != qos2.DurabilityService.MaxInstances)
            {
                System.Console.Error.WriteLine("'durability_service.MaxInstances' values do not match"
                    );
                return false;
            }
            if (qos1.DurabilityService.MaxSamples != qos2.DurabilityService.MaxSamples)
            {
                System.Console.Error.WriteLine("'durability_service.MaxSamples' values do not match"
                    );
                return false;
            }
            if (qos1.DurabilityService.MaxSamplesPerInstance != qos2.DurabilityService.MaxSamplesPerInstance)
            {
                System.Console.Error.WriteLine("'durability_service.MaxSamplesPerInstance' values do not match"
                    );
                return false;
            }
            if (qos1.History.Depth != qos2.History.Depth)
            {
                System.Console.Error.WriteLine("'history.Depth' values do not match");
                return false;
            }
            if (qos1.History.Kind != qos2.History.Kind)
            {
                System.Console.Error.WriteLine("'history.kind' values do not match");
                return false;
            }
            if (!DurationEqual(qos1.LatencyBudget.Duration, qos2.LatencyBudget.Duration))
            {
                System.Console.Error.WriteLine("'latency_budget.Duration' values do not match");
                return false;
            }
            if (!DurationEqual(qos1.Lifespan.Duration, qos2.Lifespan.Duration))
            {
                System.Console.Error.WriteLine("'lifespan.Duration' values do not match");
                return false;
            }
            if (qos1.Liveliness.Kind != qos2.Liveliness.Kind)
            {
                System.Console.Error.WriteLine("'liveliness.kind' values do not match");
                return false;
            }
            if (!DurationEqual(qos1.Liveliness.LeaseDuration, qos2.Liveliness.LeaseDuration
                ))
            {
                System.Console.Error.WriteLine("'liveliness.LeaseDuration' values do not match");
                return false;
            }
            if (qos1.Ownership.Kind != qos2.Ownership.Kind)
            {
                System.Console.Error.WriteLine("'ownership.kind' values do not match");
                return false;
            }
            if (qos1.Reliability.Kind != qos2.Reliability.Kind)
            {
                System.Console.Error.WriteLine("'reliability.kind' values do not match");
                return false;
            }
            if (!DurationEqual(qos1.Reliability.MaxBlockingTime, qos2.Reliability.MaxBlockingTime
                ))
            {
                System.Console.Error.WriteLine("'reliability.MaxBlockingTime' values do not match"
                    );
                return false;
            }
            if (qos1.ResourceLimits.MaxInstances != qos2.ResourceLimits.MaxInstances)
            {
                System.Console.Error.WriteLine("'resource_limits.MaxInstances' values do not match"
                    );
                return false;
            }
            if (qos1.ResourceLimits.MaxSamples != qos2.ResourceLimits.MaxSamples)
            {
                System.Console.Error.WriteLine("'resource_limits.MaxSamples' values do not match"
                    );
                return false;
            }
            if (qos1.ResourceLimits.MaxSamplesPerInstance != qos2.ResourceLimits.MaxSamplesPerInstance)
            {
                System.Console.Error.WriteLine("'resource_limits.MaxSamplesPerInstance' values do not match"
                    );
                return false;
            }
            if (!ByteArrayEqual(qos1.TopicData.Value, qos2.TopicData.Value))
            {
                System.Console.Error.WriteLine("'topic_data.Value' values do not match");
                return false;
            }
            if (qos1.TransportPriority.Value != qos2.TransportPriority.Value)
            {
                System.Console.Error.WriteLine("'transport_priority.Value' values do not match");
                return false;
            }
            return true;
        }

        public static bool ByteArrayEqual(byte[] arr1, byte[] arr2)
        {
            if (arr1.Length != arr2.Length)
            {
                System.Console.Error.WriteLine("Byte array lengths not equal.(" + arr1.Length + " != "
                     + arr2.Length + ")");
                return false;
            }
            for (int i = 0; i < arr1.Length; i++)
            {
                if (arr1[i] != arr2[i])
                {
                    System.Console.Error.WriteLine("Byte arrays not equal (index: " + i + ")");
                    return false;
                }
            }
            return true;
        }

        public static bool StringArrayEqual(string[] arr1, string[] arr2)
        {
            if (arr1.Length != arr2.Length)
            {
                System.Console.Error.WriteLine("String array lengths not equal. (" + arr1.Length
                    + " != " + arr2.Length + ")");
                return false;
            }
            for (int i = 0; i < arr1.Length; i++)
            {
                if (!arr1[i].Equals(arr2[i]))
                {
                    System.Console.Error.WriteLine("String arrays not equal (index: " + i + ")");
                    return false;
                }
            }
            return true;
        }

        public static bool DurationEqual(DDS.Duration t1, DDS.Duration t2)
        {
            return t1 == t2;
        }
    }
}
