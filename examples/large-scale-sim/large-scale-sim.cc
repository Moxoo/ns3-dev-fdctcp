#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include <vector>
#include <map>
#include <utility>
#include <set>

// The CDF in TrafficGenerator
extern "C"
{
#include "../large-scale-sim/cdf.h"
}

#define LINK_CAPACITY_BASE    1000000000          // 1Gbps
//#define BUFFER_SIZE "600p"                           // 600/250 packets
#define RED_QUEUE_MARKING 65 		        	  // 65 for 10Gb

// The flow port range, each flow will be assigned a random port number within this range
#define PORT_START 10000
#define PORT_END 50000

// Adopted from the simulation from WANG PENG
// Acknowledged to https://williamcityu@bitbucket.org/williamcityu/2016-socc-simulation.git
#define PACKET_SIZE 1500
#define PRESTO_RATIO 10

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("CongaSimulationLarge");
enum RunMode {
    ECMP
};

double poission_gen_interval(double avg_rate)
{
    if (avg_rate > 0)
       return -logf(1.0 - (double)rand() / RAND_MAX) / avg_rate;
    else
       return 0;
}

template<typename T>
T rand_range (T min, T max)
{
    return min + ((double)max - min) * rand () / RAND_MAX;
}

void install_applications (int fromLeafId, NodeContainer servers, double requestRate, struct cdf_table *cdfTable,
        long &flowCount, long &totalFlowSize, int SERVER_COUNT, int LEAF_COUNT, double START_TIME, double END_TIME, double FLOW_LAUNCH_END_TIME)
{
    NS_LOG_INFO ("Install applications:");
    for (int i = 0; i < SERVER_COUNT; i++)
    {
        int fromServerIndex = fromLeafId * SERVER_COUNT + i;

        double startTime = START_TIME + poission_gen_interval (requestRate);
        while (startTime < FLOW_LAUNCH_END_TIME)
        {
            flowCount ++;
            uint16_t port = rand_range (PORT_START, PORT_END);

            int destServerIndex = fromServerIndex;
	        while (destServerIndex >= fromLeafId * SERVER_COUNT && destServerIndex < fromLeafId * SERVER_COUNT + SERVER_COUNT)
            {
		        destServerIndex = rand_range (0, SERVER_COUNT * LEAF_COUNT);
            }

	        Ptr<Node> destServer = servers.Get (destServerIndex);
	        Ptr<Ipv4> ipv4 = destServer->GetObject<Ipv4> ();
	        Ipv4InterfaceAddress destInterface = ipv4->GetAddress (1,0);
	        Ipv4Address destAddress = destInterface.GetLocal ();

	        BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (destAddress, port));
	        uint32_t flowSize = gen_random_cdf (cdfTable);

	        totalFlowSize += flowSize;
 	        source.SetAttribute ("SendSize", UintegerValue (PACKET_SIZE-40));
 	        source.SetAttribute ("MaxBytes", UintegerValue(flowSize));

 	        // Install apps
 	        ApplicationContainer sourceApp = source.Install (servers.Get (fromServerIndex));
 	        sourceApp.Start (Seconds (startTime));
 	        sourceApp.Stop (Seconds (END_TIME));

 	        // Install packet sinks
 	        PacketSinkHelper sink ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
 	        //sink.SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
 	        ApplicationContainer sinkApp = sink.Install (servers.Get (destServerIndex));
 	        sinkApp.Start (Seconds (START_TIME));
 	        sinkApp.Stop (Seconds (END_TIME));


//            NS_LOG_INFO ("\tFlow from server: " << fromServerIndex << " to server: "
//                    << destServerIndex << " on port: " << port << " with flow size: "
//                    << flowSize << " [start time: " << startTime <<"]");

 	        startTime += poission_gen_interval (requestRate);
        }
    }
}

void printCurrentPercent(double endTime)
{
	double percentage = (100. * Simulator::Now().GetSeconds()) / endTime;
	std::cout << "*** " << percentage << " *** " << std::endl;

	double deltaT = endTime/10;
	int t = Simulator::Now().GetSeconds() / deltaT;

	double nexttime = deltaT * (t+1);
	Simulator::Schedule(Seconds(nexttime), &printCurrentPercent, endTime);
}

int main (int argc, char *argv[])
{
#if 1
    LogComponentEnable ("CongaSimulationLarge", LOG_LEVEL_INFO);
//    LogComponentEnable ("Ipv4QueueDiscItem", LOG_LEVEL_DEBUG);
//    LogComponentEnable ("RedQueueFrontDisc", LOG_LEVEL_DEBUG);
//    LogComponentEnable ("TcpTxBuffer", LOG_LEVEL_INFO);
#endif

    // Command line parameters parsing
    std::string id = "0";
    std::string runModeStr = "ECMP";
    unsigned randomSeed = 0;
    std::string cdfFileName = "DCTCP_CDF.txt";
    double load = 0.0;

    std::string transportProt = "DcTcp";
    std::string queuedisc = "RedQueueDisc";

    bool RandomMark = false;
    int InitialCwnd = 10;
    double minRto = 5;

    // The simulation starting and ending time
    double START_TIME = 0.0;
    double END_TIME = 0.25;
    double FLOW_LAUNCH_END_TIME = 0.1;

    uint32_t linkLatency = 10;

    int SERVER_COUNT = 8;
    int SPINE_COUNT = 4;
    int LEAF_COUNT = 4;
    int LINK_COUNT = 1;

    std::string BUFFER_SIZE = "250p";

    uint64_t spineLeafCapacity = 10;
    uint64_t leafServerCapacity = 10;

    CommandLine cmd;
    cmd.AddValue ("ID", "Running ID", id);
    cmd.AddValue ("Buffer", "BUFFER SIZE of port", BUFFER_SIZE);
    cmd.AddValue ("StartTime", "Start time of the simulation", START_TIME);
    cmd.AddValue ("EndTime", "End time of the simulation", END_TIME);
    cmd.AddValue ("FlowLaunchEndTime", "End time of the flow launch period", FLOW_LAUNCH_END_TIME);
    cmd.AddValue ("runMode", "Running mode of this simulation:ECMP", runModeStr);
    cmd.AddValue ("randomSeed", "Random seed, 0 for random generated", randomSeed);
    cmd.AddValue ("cdfFileName", "File name for flow distribution", cdfFileName);
    cmd.AddValue ("load", "Load of the network, 0.0 - 1.0", load);
    cmd.AddValue ("transportProt", "Transport protocol to use: Tcp, DcTcp", transportProt);
    cmd.AddValue ("queuedisc", "Queue disc to use: RedQueueDisc, RedQueueFrontDisc", queuedisc);
    cmd.AddValue ("RandomMark", "If mark method use random mark or not", RandomMark);
    cmd.AddValue ("linkLatency", "Link latency, should be in MicroSeconds", linkLatency);
    cmd.AddValue ("serverCount", "The Server count", SERVER_COUNT);
    cmd.AddValue ("spineCount", "The Spine count", SPINE_COUNT);
    cmd.AddValue ("leafCount", "The Leaf count", LEAF_COUNT);
    cmd.AddValue ("linkCount", "The Link count", LINK_COUNT);
    cmd.AddValue ("spineLeafCapacity", "Spine <-> Leaf capacity in Gbps", spineLeafCapacity);
    cmd.AddValue ("leafServerCapacity", "Leaf <-> Server capacity in Gbps", leafServerCapacity);
    cmd.Parse (argc, argv);

    uint64_t SPINE_LEAF_CAPACITY = spineLeafCapacity * LINK_CAPACITY_BASE;
    uint64_t LEAF_SERVER_CAPACITY = leafServerCapacity * LINK_CAPACITY_BASE;
    Time LINK_LATENCY = MicroSeconds (linkLatency);

    RunMode runMode;
  
    if (runModeStr.compare ("ECMP") == 0)
      {
        runMode = ECMP;
      }
    else
      {
        NS_LOG_ERROR ("ECMP");
        return 0;
      }

    if (load < 0.0 || load >= 1.0)
      {
        NS_LOG_ERROR ("The network load should within 0.0 and 1.0");
        return 0;
      }

    if (transportProt.compare ("DcTcp") == 0)
      {
    	NS_LOG_INFO ("Enabling DcTcp");
    	Config::SetDefault ("ns3::TcpSocketBase::EcnMode", EnumValue(1));
    	Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpDctcp::GetTypeId ()));

    	NS_LOG_INFO ("Config " << queuedisc);
    	Config::SetDefault ("ns3::"+queuedisc+"::UseEcn", BooleanValue (true));
    	Config::SetDefault ("ns3::"+queuedisc+"::QW", DoubleValue (1));
    	Config::SetDefault ("ns3::"+queuedisc+"::MeanPktSize", UintegerValue (PACKET_SIZE));
    	Config::SetDefault ("ns3::"+queuedisc+"::MaxSize", QueueSizeValue (QueueSize(BUFFER_SIZE)));
    	Config::SetDefault ("ns3::"+queuedisc+"::Gentle", BooleanValue (false));
    	Config::SetDefault ("ns3::"+queuedisc+"::UseHardDrop", BooleanValue (false));
        if(queuedisc.compare("RedQueueDisc")==0 && RandomMark)
        {
        	NS_LOG_INFO ("RED use random mark mode");
            Config::SetDefault ("ns3::RedQueueDisc::UseRandomPeek", BooleanValue (RandomMark));
//            Config::SetDefault ("ns3::RedQueueDisc::UseScan", BooleanValue (false));
        }
      }

    NS_LOG_INFO ("Config TcpSocket parameters");
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue(PACKET_SIZE-40));
    Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
    Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (InitialCwnd));

    Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (160000000));
    Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (160000000));

//    Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (false));
    Config::SetDefault ("ns3::TcpSocketBase::Timestamp", BooleanValue (false));
    Config::SetDefault ("ns3::TcpSocketBase::MinRto", TimeValue (MilliSeconds (minRto)));
    Config::SetDefault ("ns3::TcpSocketBase::ClockGranularity", TimeValue (MicroSeconds (100)));
    Config::SetDefault ("ns3::RttEstimator::InitialEstimation", TimeValue (MicroSeconds (80)));


    NodeContainer spines;
    spines.Create (SPINE_COUNT);
    NodeContainer leaves;
    leaves.Create (LEAF_COUNT);
    NodeContainer servers;
    servers.Create (SERVER_COUNT * LEAF_COUNT);

    NS_LOG_INFO ("Install Internet stacks");
    InternetStackHelper internet;
    Ipv4GlobalRoutingHelper globalRoutingHelper;

    if (runMode == ECMP)
      {
        NS_LOG_INFO ("Enable ECMP");
//    	Config::SetDefault ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue (true));
      }

	internet.SetRoutingHelper (globalRoutingHelper);
	internet.Install (servers);
	internet.Install (spines);
	internet.Install (leaves);

    NS_LOG_INFO ("Install channels and assign addresses");
    PointToPointHelper p2p;
    Ipv4AddressHelper ipv4;
    TrafficControlHelper tc;
    if (transportProt.compare ("DcTcp") == 0)
      {
        tc.SetRootQueueDisc ("ns3::"+queuedisc+"", "MinTh", DoubleValue (RED_QUEUE_MARKING),
                                                   "MaxTh", DoubleValue (RED_QUEUE_MARKING));
      }
    if (transportProt.compare ("TCP") == 0)
      {
        tc.SetRootQueueDisc ("ns3::FifoQueueDisc", "MaxSize", QueueSizeValue (QueueSize(BUFFER_SIZE)));
      }

    NS_LOG_INFO ("Configuring servers");
    // Setting servers
    p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (LEAF_SERVER_CAPACITY)));
    p2p.SetChannelAttribute ("Delay", TimeValue(LINK_LATENCY));
    ipv4.SetBase ("10.1.0.0", "255.255.255.0");

    std::vector<Ipv4Address> leafNetworks (LEAF_COUNT);
    std::vector<Ipv4Address> serverAddresses (SERVER_COUNT * LEAF_COUNT);

    std::map<std::pair<int, int>, uint32_t> leafToSpinePath;
    std::map<std::pair<int, int>, uint32_t> spineToLeafPath;

    for (int i = 0; i < LEAF_COUNT; i++)
      {
	    Ipv4Address network = ipv4.NewNetwork ();
        leafNetworks[i] = network;

        for (int j = 0; j < SERVER_COUNT; j++)
          {
            int serverIndex = i * SERVER_COUNT + j;
            NodeContainer nodeContainer = NodeContainer (leaves.Get (i), servers.Get (serverIndex));
            NetDeviceContainer netDeviceContainer = p2p.Install (nodeContainer);

            if (transportProt.compare ("DcTcp") == 0)
		      {
		        //NS_LOG_INFO ("Install RED Queue for leaf: " << i << " and server: " << j);
	            tc.Install (netDeviceContainer);
              }
            Ipv4InterfaceContainer interfaceContainer = ipv4.Assign (netDeviceContainer);

//            NS_LOG_INFO ("Leaf - " << i << " is connected to Server - " << j << " with address "
//                    << interfaceContainer.GetAddress(0) << " <-> " << interfaceContainer.GetAddress (1)
//                    << " with port " << netDeviceContainer.Get (0)->GetIfIndex () << " <-> " << netDeviceContainer.Get (1)->GetIfIndex ());

            serverAddresses [serverIndex] = interfaceContainer.GetAddress (1);
		    if (transportProt.compare ("Tcp") == 0)
              {
                tc.Uninstall (netDeviceContainer);
              }
          }
      }

    NS_LOG_INFO ("Configuring switches");
    // Setting up switches
    p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (SPINE_LEAF_CAPACITY)));
    std::set<std::pair<uint32_t, uint32_t> > asymLink; // set< (A, B) > Leaf A -> Spine B is asymmetric

    for (int i = 0; i < LEAF_COUNT; i++)
      {
        for (int j = 0; j < SPINE_COUNT; j++)
          {
        	for (int l = 0; l < LINK_COUNT; l++)
        	  {
            // TODO
        		uint64_t spineLeafCapacity = SPINE_LEAF_CAPACITY;

        		p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (spineLeafCapacity)));
        		ipv4.NewNetwork ();

        		NodeContainer nodeContainer = NodeContainer (leaves.Get (i), spines.Get (j));
        		NetDeviceContainer netDeviceContainer = p2p.Install (nodeContainer);
        		if (transportProt.compare ("DcTcp") == 0)
        		  {
        			//NS_LOG_INFO ("Install RED Queue for leaf: " << i << " and spine: " << j);
        			tc.Install (netDeviceContainer);
        		  }
        		Ipv4InterfaceContainer ipv4InterfaceContainer = ipv4.Assign (netDeviceContainer);
        		//NS_LOG_INFO ("ipv4.Assign");
//        		NS_LOG_INFO ("Leaf - " << i << " is connected to Spine - " << j << " with address "
//                    << ipv4InterfaceContainer.GetAddress(0) << " <-> " << ipv4InterfaceContainer.GetAddress (1)
//                    << " with port " << netDeviceContainer.Get (0)->GetIfIndex () << " <-> " << netDeviceContainer.Get (1)->GetIfIndex ()
//                    << " with data rate " << spineLeafCapacity);

        		if (transportProt.compare ("Tcp") == 0)
        		  {
        			tc.Uninstall (netDeviceContainer);
        		  }
        	  }
          }
      }

    if (runMode == ECMP )
      {
        NS_LOG_INFO ("Populate global routing tables");
        Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
      }

    double oversubRatio = static_cast<double>(SERVER_COUNT * LEAF_SERVER_CAPACITY) / (SPINE_LEAF_CAPACITY * SPINE_COUNT * LINK_COUNT);
    NS_LOG_INFO ("Over-subscription ratio: " << oversubRatio);

    NS_LOG_INFO ("Initialize CDF table");
    struct cdf_table* cdfTable = new cdf_table ();
    init_cdf (cdfTable);
    load_cdf (cdfTable, cdfFileName.c_str ());

    NS_LOG_INFO ("Calculating request rate");
    double requestRate = load * LEAF_SERVER_CAPACITY * SERVER_COUNT / oversubRatio / (8 * avg_cdf (cdfTable)) / SERVER_COUNT;
    NS_LOG_INFO ("Average request rate: " << requestRate << " per second");

    NS_LOG_INFO ("Initialize random seed: " << randomSeed);
    if (randomSeed == 0)
      {
        srand ((unsigned)time (NULL));
      }
    else
      {
        srand (randomSeed);
      }

    NS_LOG_INFO ("Create applications");

    long flowCount = 0;
    long totalFlowSize = 0;

    for (int fromLeafId = 0; fromLeafId < LEAF_COUNT; fromLeafId ++)
      {
        install_applications(fromLeafId, servers, requestRate, cdfTable, flowCount, totalFlowSize, SERVER_COUNT, LEAF_COUNT, START_TIME, END_TIME, FLOW_LAUNCH_END_TIME);
      }

    NS_LOG_INFO ("Total flow: " << flowCount);

    NS_LOG_INFO ("Actual average flow size: " << static_cast<double> (totalFlowSize) / flowCount);

    NS_LOG_INFO ("Enabling flow monitor");

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    NS_LOG_INFO ("Enabling link monitor");



    flowMonitor->CheckForLostPackets ();

    std::stringstream flowMonitorFilename;
    std::stringstream linkMonitorFilename;
    if(RandomMark)id = "1";
    if(queuedisc.compare ("RedQueueFrontDisc") == 0) id="2";

    flowMonitorFilename << id << "-1-large-load-" << LEAF_COUNT << "X" << SPINE_COUNT << "-" << load << "-"  << transportProt <<"-";
    linkMonitorFilename << id << "-1-large-load-" << LEAF_COUNT << "X" << SPINE_COUNT << "-" << load << "-"  << transportProt <<"-";


    if (runMode == ECMP)
      {
        flowMonitorFilename << "ecmp-simulation-";
        linkMonitorFilename << "ecmp-simulation-";
      }

    flowMonitorFilename << randomSeed << "-";
    linkMonitorFilename << randomSeed << "-";

    flowMonitorFilename << "b" << BUFFER_SIZE << ".xml";
    linkMonitorFilename << "b" << BUFFER_SIZE << "-link-utility.out";


    NS_LOG_INFO ("Start simulation");
//    p2p.EnablePcapAll("Test", true);
    Simulator::Stop (Seconds (END_TIME));
    Simulator::Schedule(Seconds(0.0), &printCurrentPercent, END_TIME);
    Simulator::Run ();

    flowMonitor->SerializeToXmlFile(flowMonitorFilename.str (), true, true);

    Simulator::Destroy ();
    free_cdf (cdfTable);
    NS_LOG_INFO ("Stop simulation");
}
