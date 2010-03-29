namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class SubscriberQosComparer
    {
        public static bool SubscriberQosEqual(DDS.SubscriberQos qos1, DDS.SubscriberQos qos2
            )
        {
            if (qos1.EntityFactory.AutoenableCreatedEntities != qos2.EntityFactory.AutoenableCreatedEntities)
            {
                System.Console.Error.WriteLine("'entity_factory.autoenable_created_entities' values do not match"
                    );
                return false;
            }
            if (!ByteArrayEqual(qos1.GroupData.Value, qos1.GroupData.Value))
            {
                System.Console.Error.WriteLine("'group_data.Value' values do not match");
                return false;
            }
            if (!StringArrayEqual(qos1.Partition.Name, qos2.Partition.Name))
            {
                System.Console.Error.WriteLine("'partition.Name' values do not match");
                return false;
            }
            if (qos1.Presentation.CoherentAccess != qos2.Presentation.CoherentAccess)
            {
                System.Console.Error.WriteLine("'presentation.CoherentAccess' values do not match"
                    );
                return false;
            }
            if (qos1.Presentation.OrderedAccess != qos2.Presentation.OrderedAccess)
            {
                System.Console.Error.WriteLine("'presentation.OrderedAccess' values do not match"
                    );
                return false;
            }
            if (qos1.Presentation.AccessScope != qos2.Presentation.AccessScope)
            {
                System.Console.Error.WriteLine("'presentation.AccessScope' values do not match");
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
    }
}
