using System;
using System.Threading;
using System.IO;

using DDS;
using DDS.OpenSplice;

using OwnershipData;
using DDSAPIHelper;

namespace OwnershipPublisher
{
    class PublisherRunner
    {
        static void Main(string[] args)
        {
            if (args.Length != 4)
            {
                Console.WriteLine("Insufficient number of arguments.");
                usage();
            }
            else
            {
                string publisherName = args[0];
                int ownershipStrength = int.Parse(args[1]);
                int numOfIterations = int.Parse(args[2]);
                bool stopSubscriber = bool.Parse(args[3]);

                OwnershipPublisher pub = new OwnershipPublisher(publisherName, ownershipStrength);

                Console.WriteLine("=== [Publisher] Publisher  {0} with strength: {1} ", publisherName, ownershipStrength);
                Console.WriteLine(" / sending {0} prices ... stop_subscriber flag= {1}", numOfIterations, stopSubscriber.ToString());


                // The subscriber should display the prices in dollars 
                float price = 10.0f;

                for (int x = 0; x < numOfIterations; x++)
                {
                    pub.publishEvent(price, publisherName);
                    Thread.Sleep(200);
                    price += 0.5f;
                }

                Thread.Sleep(2000);

                if (stopSubscriber)
                {
                    price = -1.0f;
                    Console.WriteLine("=== Stopping the subscriber");
                    pub.publishEvent(price, publisherName);
                }
                
                pub.dispose();
            }
        }

        private static void usage()
        {
            Console.WriteLine("*** Error***");
            Console.WriteLine("*** Usage: OwnershipPublisher <publisher_name> <ownership_strength> <nb_iterations> <stop_subscriber_flag>");
            Console.WriteLine("*** publisher_name - Name of publisher ");
            Console.WriteLine("*** ownership_strength - Positive integer ");
            Console.WriteLine("*** nb_iterations - Positive integer for number of messages to publish");
            Console.WriteLine("*** stop_subscriber_flag  [false | true]");
        }
    }
}
