/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

/************************************************************************
 * LOGICAL_NAME:    DurabilityDataSubscriber.cs
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C# programming language.
 * DATE             September 2010.
 ************************************************************************
 *
 * This file contains the implementation of the Subscriber for the 'Durability' example.
 *
 ***/
using System;
using System.Threading;

using DDS;
using DDS.OpenSplice;
using DurabilityData;
using DDSAPIHelper;
using System.IO;

namespace DurableSubscriber
{
    class DurableSubscriber
    {
        static void Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Insufficient number of arguments.");
                usage();
            }
            else
            {
                DDSEntityManager mgr = new DDSEntityManager("Durability");
                String partitionName = "Durability Example";

                String durabilityKind = args[0];

                // create Domain Participant
                mgr.createParticipant(partitionName);

                // set the durability kind
                mgr.setDurabilityKind(durabilityKind);

                // create Type
                MsgTypeSupport msgTS = new MsgTypeSupport();
                mgr.registerType(msgTS);

                // create Topic
                mgr.createTopic("DurabilityData_Msg");

                // create Subscriber
                mgr.createSubscriber();
                mgr.createReader(false);

                IDataReader dreader = mgr.getReader();
                MsgDataReader DurabilityDataReader = dreader as MsgDataReader;

                Msg[] msgSeq = null;
                DDS.SampleInfo[] infoSeq = null;
                Boolean terminate = false;
                ReturnCode status;
                Console.WriteLine("=== [Subscriber] Ready ...");
                while (!terminate)
                {
                    status = DurabilityDataReader.Take(ref msgSeq, ref infoSeq, Length.Unlimited, SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
                    ErrorHandler.checkStatus(status, "DataReader.Take");
                    for (int i = 0; i < msgSeq.Length; i++)
                    {
                        if (infoSeq[i].ValidData)
                        {
                            Console.WriteLine("{0} : {1}", msgSeq[i].id, msgSeq[i].content);
                            if (msgSeq[i].content.Equals("9"))
                            {
                                terminate = true;
                                break;
                            }
                        }
                    }
                    status = DurabilityDataReader.ReturnLoan(ref msgSeq, ref infoSeq);
                    ErrorHandler.checkStatus(status, "MsgDataReader.ReturnLoan");
                }
                // clean up
                mgr.getSubscriber().DeleteDataReader(DurabilityDataReader);
                mgr.deleteSubscriber();
                mgr.deleteTopic();
                mgr.deleteParticipant();
            }
        }

        private static void usage()
        {
            Console.WriteLine("*** Error***");
            Console.WriteLine("*** Usage: DurableSubscriber <durability_kind> ");
            Console.WriteLine("*** durability_kind [transient | persistent] ");
        }
    }
}
