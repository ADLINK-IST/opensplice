namespace test.sacs
{
    /// <date>Jun 21, 2005</date>
    public class StatusValidator
    {
        public static bool StatussesValid(DDS.IDataReader reader, int rdmTotalCount, int rdmTotalCountChange
            , int riqsTotalCount, int riqsTotalCountChange, int srTotalCount, int srTotalCountChange
            , int lcActiveCount, int lcActiveCountChange, int lcInactiveCount, int lcInactiveCountChange
            , int smTotalCount, int smTotalCountChange, int slTotalCount, int slTotalCountChange
            )
        {
            bool result = false;
            if (RequestedDeadlineMissedValid(reader, rdmTotalCount, rdmTotalCountChange))
            {
                if (RequestedIncompatibleQosValid(reader, riqsTotalCount, riqsTotalCountChange))
                {
                    if (SampleRejectedValid(reader, srTotalCount, srTotalCountChange))
                    {
                        if (LivelinessChangedValid(reader, lcActiveCount, lcActiveCountChange, lcInactiveCount
                            , lcInactiveCountChange))
                        {
                            if (SubscriptionMatchValid(reader, smTotalCount, smTotalCountChange))
                            {
                                if (SampleLostValid(reader, slTotalCount, slTotalCountChange))
                                {
                                    result = true;
                                }
                            }
                        }
                    }
                }
            }
            return result;
        }

        public static bool StatussesValid(DDS.IDataWriter writer, int odmTotalCount, int odmTotalCountChange
            , int oiqsTotalCount, int oiqsTotalCountChange, int llTotalCount, int llTotalCountChange
            , int pmTotalCount, int pmTotalCountChange)
        {
            bool result = false;
            if (OfferedDeadlineMissedValid(writer, odmTotalCount, odmTotalCountChange))
            {
                if (OfferedIncompatibleQosValid(writer, oiqsTotalCount, oiqsTotalCountChange))
                {
                    if (LivelinessLostValid(writer, llTotalCount, llTotalCountChange))
                    {
                        if (PublicationMatchValid(writer, pmTotalCount, pmTotalCountChange))
                        {
                            result = true;
                        }
                    }
                }
            }
            return result;
        }

        public static bool RequestedDeadlineMissedValid(DDS.IDataReader reader, int totalCount
            , int totalCountChange)
        {
            bool result = false;
            DDS.RequestedDeadlineMissedStatus holder = new DDS.RequestedDeadlineMissedStatus();
            DDS.ReturnCode rc = reader.GetRequestedDeadlineMissedStatus(ref holder);
            if (rc == DDS.ReturnCode.Ok)
            {
                DDS.RequestedDeadlineMissedStatus status = holder;
                if (status.TotalCount == totalCount)
                {
                    if (status.TotalCountChange == totalCountChange)
                    {
                        result = true;
                    }
                    else
                    {
                        System.Console.Error.WriteLine("requested_deadline_missed.TotalCountChange != '"
                             + totalCountChange + "', but '" + status.TotalCountChange + "'.");
                    }
                }
                else
                {
                    System.Console.Error.WriteLine("requested_deadline_missed.TotalCount != '" + totalCount
                         + "', but '" + status.TotalCount + "'.");
                }
            }
            else
            {
                System.Console.Error.WriteLine("get_requested_deadline_missed_status returned " +
                     rc);
            }
            return result;
        }

        public static bool RequestedIncompatibleQosValid(DDS.IDataReader reader, int totalCount
            , int totalCountChange)
        {
            bool result = false;
            DDS.RequestedIncompatibleQosStatus holder = new DDS.RequestedIncompatibleQosStatus();
            DDS.ReturnCode rc = reader.GetRequestedIncompatibleQosStatus(ref holder);
            if (rc == DDS.ReturnCode.Ok)
            {
                DDS.RequestedIncompatibleQosStatus status = holder;
                if (status.TotalCount == totalCount)
                {
                    if (status.TotalCountChange == totalCountChange)
                    {
                        result = true;
                    }
                    else
                    {
                        System.Console.Error.WriteLine("requested_incompatible_qos.TotalCountChange != '"
                             + totalCountChange + "', but '" + status.TotalCountChange + "'.");
                    }
                }
                else
                {
                    System.Console.Error.WriteLine("requested_incompatible_qos.TotalCount != '" + totalCount
                         + "', but '" + status.TotalCount + "'.");
                }
            }
            else
            {
                System.Console.Error.WriteLine("get_requested_incompatible_qos_status returned "
                    + rc);
            }
            return result;
        }

        public static bool SampleRejectedValid(DDS.IDataReader reader, int totalCount, int
             totalCountChange)
        {
            bool result = false;
            DDS.SampleRejectedStatus holder = new DDS.SampleRejectedStatus();
            DDS.ReturnCode rc = reader.GetSampleRejectedStatus(ref holder);
            if (rc == DDS.ReturnCode.Ok)
            {
                DDS.SampleRejectedStatus status = holder;
                if (status.TotalCount == totalCount)
                {
                    if (status.TotalCountChange == totalCountChange)
                    {
                        result = true;
                    }
                    else
                    {
                        System.Console.Error.WriteLine("sample_rejected.TotalCountChange != '" + totalCountChange
                             + "', but '" + status.TotalCountChange + "'.");
                    }
                }
                else
                {
                    System.Console.Error.WriteLine("sample_rejected.TotalCount != '" + totalCount +
                        "', but '" + status.TotalCount + "'.");
                }
            }
            else
            {
                System.Console.Error.WriteLine("get_sample_rejected_status returned " + rc);
            }
            return result;
        }

        public static bool LivelinessChangedValid(DDS.IDataReader reader, int activeCount,
            int activeCountChange, int inactiveCount, int inactiveCountChange)
        {
            bool result = false;
            DDS.LivelinessChangedStatus holder = new DDS.LivelinessChangedStatus();
            DDS.ReturnCode rc = reader.GetLivelinessChangedStatus(ref holder);
            if (rc == DDS.ReturnCode.Ok)
            {
                DDS.LivelinessChangedStatus status = holder;
                if (status.AliveCount == activeCount)
                {
                    if (status.AliveCountChange == activeCountChange)
                    {
                        if (status.NotAliveCount == inactiveCount)
                        {
                            if (status.NotAliveCountChange == inactiveCountChange)
                            {
                                result = true;
                            }
                            else
                            {
                                System.Console.Error.WriteLine("liveliness_change.not_alive_count_change != '" +
                                    inactiveCountChange + "', but '" + status.NotAliveCountChange + "'.");
                            }
                        }
                        else
                        {
                            System.Console.Error.WriteLine("liveliness_change.not_alive_count != '" + inactiveCount
                                 + "', but '" + status.NotAliveCount + "'.");
                        }
                    }
                    else
                    {
                        System.Console.Error.WriteLine("liveliness_change.AliveCountChange != '" + activeCountChange
                             + "', but '" + status.AliveCountChange + "'.");
                    }
                }
                else
                {
                    System.Console.Error.WriteLine("liveliness_change.AliveCount != '" + activeCount
                         + "', but '" + status.AliveCount + "'.");
                }
            }
            else
            {
                System.Console.Error.WriteLine("get_liveliness_changed_status returned " + rc);
            }
            return result;
        }

        public static bool SubscriptionMatchValid(DDS.IDataReader reader, int totalCount,
            int totalCountChange)
        {
            bool result = false;
            DDS.SubscriptionMatchedStatus holder = new DDS.SubscriptionMatchedStatus();
            DDS.ReturnCode rc = reader.GetSubscriptionMatchedStatus(ref holder);
            if (rc == DDS.ReturnCode.Ok)
            {
                DDS.SubscriptionMatchedStatus status = holder;
                if (status.TotalCount == totalCount)
                {
                    if (status.TotalCountChange == totalCountChange)
                    {
                        result = true;
                    }
                    else
                    {
                        System.Console.Error.WriteLine("subscription_match.TotalCountChange != '" + totalCountChange
                             + "', but '" + status.TotalCountChange + "'.");
                    }
                }
                else
                {
                    System.Console.Error.WriteLine("subscription_match.TotalCount != '" + totalCount
                         + "', but '" + status.TotalCount + "'.");
                }
            }
            else
            {
                System.Console.Error.WriteLine("get_subscription_match_status returned " + rc);
            }
            return result;
        }

        public static bool SampleLostValid(DDS.IDataReader reader, int totalCount, int totalCountChange
            )
        {
            bool result = false;
            DDS.SampleLostStatus holder = new DDS.SampleLostStatus();
            DDS.ReturnCode rc = reader.GetSampleLostStatus(ref holder);
            if (rc == DDS.ReturnCode.Ok)
            {
                DDS.SampleLostStatus status = holder;
                if (status.TotalCount == totalCount)
                {
                    if (status.TotalCountChange == totalCountChange)
                    {
                        result = true;
                    }
                    else
                    {
                        System.Console.Error.WriteLine("sample_lost.TotalCountChange != '" + totalCountChange
                             + "', but '" + status.TotalCountChange + "'.");
                    }
                }
                else
                {
                    System.Console.Error.WriteLine("sample_lost.TotalCount != '" + totalCount + "', but '"
                         + status.TotalCount + "'.");
                }
            }
            else
            {
                System.Console.Error.WriteLine("get_sample_lost_status returned " + rc);
            }
            return result;
        }

        public static bool OfferedDeadlineMissedValid(DDS.IDataWriter writer, int totalCount
            , int totalCountChange)
        {
            bool result = false;
			DDS.OfferedDeadlineMissedStatus holder = null;
            DDS.ReturnCode rc = writer.GetOfferedDeadlineMissedStatus(ref holder);
            if (rc == DDS.ReturnCode.Ok)
            {
                DDS.OfferedDeadlineMissedStatus status = holder;
                if (status.TotalCount == totalCount)
                {
                    if (status.TotalCountChange == totalCountChange)
                    {
                        result = true;
                    }
                    else
                    {
                        System.Console.Error.WriteLine("offered_deadline_missed.TotalCountChange != '"
                            + totalCountChange + "', but '" + status.TotalCountChange + "'.");
                    }
                }
                else
                {
                    System.Console.Error.WriteLine("offered_deadline_missed.TotalCount != '" + totalCount
                         + "', but '" + status.TotalCount + "'.");
                }
            }
            else
            {
                System.Console.Error.WriteLine("get_offered_deadline_missed_status returned " + rc
                    );
            }
            return result;
        }

        public static bool OfferedIncompatibleQosValid(DDS.IDataWriter writer, int totalCount
            , int totalCountChange)
        {
            bool result = false;
			DDS.OfferedIncompatibleQosStatus holder = null;
            DDS.ReturnCode rc = writer.GetOfferedIncompatibleQosStatus(ref holder);
            if (rc == DDS.ReturnCode.Ok)
            {
                DDS.OfferedIncompatibleQosStatus status = holder;
                if (status.TotalCount == totalCount)
                {
                    if (status.TotalCountChange == totalCountChange)
                    {
                        result = true;
                    }
                    else
                    {
                        System.Console.Error.WriteLine("offered_incompatible_qos.TotalCountChange != '"
                             + totalCountChange + "', but '" + status.TotalCountChange + "'.");
                    }
                }
                else
                {
                    System.Console.Error.WriteLine("offered_incompatible_qos.TotalCount != '" + totalCount
                         + "', but '" + status.TotalCount + "'.");
                }
            }
            else
            {
                System.Console.Error.WriteLine("get_offered_incompatible_qos_status returned " +
                    rc);
            }
            return result;
        }

        public static bool LivelinessLostValid(DDS.IDataWriter writer, int totalCount, int
             totalCountChange)
        {
            bool result = false;
			DDS.LivelinessLostStatus holder = null;
            DDS.ReturnCode rc = writer.GetLivelinessLostStatus(ref holder);
            if (rc == DDS.ReturnCode.Ok)
            {
                DDS.LivelinessLostStatus status = holder;
                if (status.TotalCount == totalCount)
                {
                    if (status.TotalCountChange == totalCountChange)
                    {
                        result = true;
                    }
                    else
                    {
                        System.Console.Error.WriteLine("liveliness_lost.TotalCountChange != '" + totalCountChange
                             + "', but '" + status.TotalCountChange + "'.");
                    }
                }
                else
                {
                    System.Console.Error.WriteLine("liveliness_lost.TotalCount != '" + totalCount +
                        "', but '" + status.TotalCount + "'.");
                }
            }
            else
            {
                System.Console.Error.WriteLine("get_liveliness_lost_status returned " + rc);
            }
            return result;
        }

        public static bool PublicationMatchValid(DDS.IDataWriter writer, int totalCount, int
             totalCountChange)
        {
            bool result = false;
			DDS.PublicationMatchedStatus holder = null;
            DDS.ReturnCode rc = writer.GetPublicationMatchedStatus(ref holder);
            if (rc == DDS.ReturnCode.Ok)
            {
                DDS.PublicationMatchedStatus status = holder;
                if (status.TotalCount == totalCount)
                {
                    if (status.TotalCountChange == totalCountChange)
                    {
                        result = true;
                    }
                    else
                    {
                        System.Console.Error.WriteLine("publication_matched.TotalCountChange != '" + totalCountChange
                             + "', but '" + status.TotalCountChange + "'.");
                    }
                }
                else
                {
                    System.Console.Error.WriteLine("publication_matched.TotalCount != '" + totalCount
                         + "', but '" + status.TotalCount + "'.");
                }
            }
            else
            {
                System.Console.Error.WriteLine("get_publication_matched_status returned " + rc);
            }
            return result;
        }
    }
}
