#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("convergence");

std::stringstream filePlotQueue;
std::stringstream filePlotThroughput1;
std::stringstream filePlotThroughput2;

static Ptr<OutputStreamWrapper> cWndStream;
static Ptr<OutputStreamWrapper> ssThreshStream;
static Ptr<OutputStreamWrapper> rttStream;
static Ptr<OutputStreamWrapper> rtoStream;
static double interval = 0.0002;
uint64_t flowRecvBytes1 = 0;
uint64_t flowRecvBytes2 = 0;

void
CheckQueueSize (Ptr<QueueDisc> queue, std::string filePlotQueue)
{
  uint32_t qSize = StaticCast<RedQueueDisc> (queue)->GetNPackets();

  // check queue size every 1/1000 of a second
  Simulator::Schedule (Seconds (0.0002), &CheckQueueSize, queue, filePlotQueue);

  std::ofstream fPlotQueue (filePlotQueue.c_str (), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue.close ();
}

void
ThroughputPerSecond1(Ptr<PacketSink> sinkApps,std::string filePlotThrough)
{
	uint32_t totalRecvBytes = sinkApps->GetTotalRx();
	uint32_t currentPeriodRecvBytes = totalRecvBytes - flowRecvBytes1;

	flowRecvBytes1 = totalRecvBytes;

	Simulator::Schedule (Seconds(interval), &ThroughputPerSecond1, sinkApps, filePlotThrough);
	std::ofstream fPlotThroughput (filePlotThrough.c_str (), std::ios::out | std::ios::app);
	fPlotThroughput << Simulator::Now().GetSeconds() << " " << currentPeriodRecvBytes * 8 / (interval * 1000000) << std::endl;
}

void
ThroughputPerSecond2(Ptr<PacketSink> sinkApps,std::string filePlotThrough)
{
	uint32_t totalRecvBytes = sinkApps->GetTotalRx();
	uint32_t currentPeriodRecvBytes = totalRecvBytes - flowRecvBytes2;

	flowRecvBytes2 = totalRecvBytes;

	Simulator::Schedule (Seconds(interval), &ThroughputPerSecond2, sinkApps, filePlotThrough);
	std::ofstream fPlotThroughput (filePlotThrough.c_str (), std::ios::out | std::ios::app);
	fPlotThroughput << Simulator::Now().GetSeconds() << " " << currentPeriodRecvBytes * 8 / (interval * 1000000) << std::endl;
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
  // Configure information
  std::string prefix_file_name = "convergence";
  uint32_t SNum = 2;
  std::string transport_port = "TcpDctcp";
  std::string queue_disc_type = "RedQueueDisc";//only for dctcp

  std::string queue_limit = "170p";
  double K = 30;//100 for 600buffer/65 for 250buffer

  std::string bandwidth = "10000Mbps";
  std::string delay = "0.001ms";
  std::string bottleneck_bw = "10000Mbps";
  std::string bottleneck_delay = "0.001ms";

  uint32_t IP_PACKET_SIZE = 1000;
  uint32_t TCP_SEGMENT = IP_PACKET_SIZE-40;
  uint64_t data_mbytes = 0*16*1000;

  double minRto = 10;
  uint32_t initialCwnd = 1;

  double start_time = 0;
  double stop_time = 0.1;

  bool pcap = true;
  bool randommark = false;
  bool doscan = false;

  std::string id = "0";
  unsigned randomSeed = 0;

//  //random variable generation
//  srand(time(NULL));
//  SeedManager::SetSeed(rand());
//  Ptr<UniformRandomVariable> random = CreateObject<UniformRandomVariable> ();

  CommandLine cmd;
  cmd.AddValue ("randomSeed", "Random seed, 0 for random generated", randomSeed);
  cmd.AddValue ("SNum","Number of short flow", SNum);
  cmd.AddValue ("queuedisc","type of queuedisc", queue_disc_type);
  cmd.AddValue ("randommark","type of mark", randommark);
  cmd.AddValue ("doscan","scan queue or not", doscan);
  cmd.AddValue ("bandwidth", "Access bandwidth", bandwidth);
  cmd.AddValue ("delay", "Access delay", delay);
  cmd.AddValue ("bottleneck_bw", "Bottleneck bandwidth", bottleneck_bw);
  cmd.AddValue ("bottleneck_delay", "Bottleneck delay", bottleneck_delay);
  cmd.AddValue ("TCP_SEGMENT", "Packet size", TCP_SEGMENT);
  cmd.AddValue ("data", "Number of Megabytes of data to transmit, 0 means infinite", data_mbytes);
  cmd.AddValue ("start_time", "Start Time", start_time);
  cmd.AddValue ("stop_time", "Stop Time", stop_time);
  cmd.AddValue ("initialCwnd", "Initial Cwnd", initialCwnd);
  cmd.AddValue ("minRto", "Minimum RTO", minRto);
  cmd.AddValue ("transport_prot", "Transport protocol to use: TcpNewReno, "
                "TcpHybla, TcpDctcp, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat, "
                "TcpLp, TcpBbr", transport_port);
  cmd.Parse (argc,argv);

  // To enable logging
  LogComponentEnable("convergence", LOG_LEVEL_INFO);
//  LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);
//  LogComponentEnable("RedQueueDisc", LOG_LEVEL_DEBUG);
//  LogComponentEnable("FifoQueueDisc", LOG_LEVEL_LOGIC);
//  LogComponentEnable("Ipv4QueueDiscItem", LOG_LEVEL_DEBUG);
//  LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);
//  LogComponentEnable("TcpDctcp", LOG_LEVEL_INFO);

  NS_LOG_INFO ("Configure TcpSocket");
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (TCP_SEGMENT));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (initialCwnd));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue(1));

  Config::SetDefault ("ns3::TcpSocketBase::MinRto", TimeValue (MilliSeconds (minRto)));
  Config::SetDefault ("ns3::TcpSocketBase::Timestamp", BooleanValue (false));

  if(transport_port.compare("TcpDctcp") == 0)
  {
	  NS_LOG_INFO ("Configure Dctcp");
	  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpDctcp::GetTypeId()));
	  Config::SetDefault ("ns3::TcpSocketBase::EcnMode", EnumValue(1));

	  NS_LOG_INFO ("Configure Red");
	  Config::SetDefault ("ns3::"+queue_disc_type+"::UseEcn", BooleanValue (true));
	  Config::SetDefault ("ns3::"+queue_disc_type+"::MaxSize", QueueSizeValue (QueueSize (queue_limit)));
	  Config::SetDefault ("ns3::"+queue_disc_type+"::MeanPktSize", UintegerValue (IP_PACKET_SIZE));
	  Config::SetDefault("ns3::"+queue_disc_type+"::QW", DoubleValue (1));
	  Config::SetDefault("ns3::"+queue_disc_type+"::MinTh", DoubleValue (K));
	  Config::SetDefault("ns3::"+queue_disc_type+"::MaxTh", DoubleValue (K));
	  Config::SetDefault ("ns3::"+queue_disc_type+"::Gentle", BooleanValue (false));
	  Config::SetDefault ("ns3::"+queue_disc_type+"::UseHardDrop", BooleanValue (false));
	  if(queue_disc_type.compare("RedQueueDisc") == 0)
	  {
		  Config::SetDefault ("ns3::RedQueueDisc::UseDoScan", BooleanValue (doscan));
	  }

	  if (randommark)
	  {
		  NS_LOG_INFO ("Red uses random mark mode");
		  Config::SetDefault ("ns3::"+queue_disc_type+"::UseRandomPeek", BooleanValue (randommark));
	  }
  }
  else if(transport_port.compare("TcpCubic") == 0)
  {
	  NS_LOG_INFO ("Configure Cubic And FIFO");
	  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpCubic::GetTypeId()));
	  queue_disc_type = "FifoQueueDisc";
	  Config::SetDefault("ns3::FifoQueueDisc::MaxSize", QueueSizeValue (QueueSize (queue_limit)));
  }

  // Create sendNum, receiver, and switches
  NodeContainer switches;
  switches.Create(1);
  NodeContainer senders;
  senders.Create(SNum);
  NodeContainer receiver;
  receiver.Create(1);

  // Configure channel attributes
  PointToPointHelper ptpLink;
  ptpLink.SetDeviceAttribute("DataRate", StringValue(bandwidth));
  ptpLink.SetChannelAttribute("Delay", StringValue(delay));

  PointToPointHelper neckLink;
  neckLink.SetDeviceAttribute("DataRate", StringValue(bottleneck_bw));
  neckLink.SetChannelAttribute("Delay", StringValue(bottleneck_delay));

  // Install InternetStack
  InternetStackHelper stack;
  stack.InstallAll();

  // Configure TrafficControlHelper
  TrafficControlHelper tchRed;
  tchRed.SetRootQueueDisc("ns3::"+queue_disc_type);
  NS_LOG_INFO ("Install " << queue_disc_type);

  // Configure Ipv4AddressHelper
  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.255.255.0");

  // Configure net devices in nodes and connect sendNum with switches
  for(uint32_t i = 0; i<senders.GetN(); i++)
  {
	  NetDeviceContainer devices;
	  devices = ptpLink.Install(senders.Get(i), switches.Get(0));
	  tchRed.Install(devices);
	  address.NewNetwork();
	  Ipv4InterfaceContainer interfaces = address.Assign(devices);
  }

  // Connect switch1 with receivers
  Ipv4InterfaceContainer sink_interfaces;
  NetDeviceContainer devices;
  devices = ptpLink.Install(switches.Get(0), receiver.Get(0));

  // In queueDiscs in switch
  QueueDiscContainer queueDiscs;
  queueDiscs = tchRed.Install(devices);
  address.NewNetwork();
  Ipv4InterfaceContainer interfaces = address.Assign(devices);
  sink_interfaces.Add(interfaces.Get(1));

  // Configure routing
  NS_LOG_INFO ("Initialize Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Configure port/address and install application, establish connection
  NS_LOG_INFO ("Build connections");
  uint16_t port = 50000;

  // long flow 1
  BulkSendHelper ftp1("ns3::TcpSocketFactory", InetSocketAddress(sink_interfaces.GetAddress(0,0),port));
  ftp1.SetAttribute("SendSize", UintegerValue(TCP_SEGMENT));
  ftp1.SetAttribute("MaxBytes", UintegerValue(0*data_mbytes));
  ApplicationContainer sourceApp1 = ftp1.Install(senders.Get(0));
  sourceApp1.Start(Seconds(start_time));
  sourceApp1.Stop(Seconds(stop_time));
  PacketSinkHelper sinkHelper1("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port));
  ApplicationContainer sinkApp1 = sinkHelper1.Install(receiver.Get(0));
  sinkApp1.Start(Seconds(start_time));
  sinkApp1.Stop(Seconds(stop_time));

  double start_time_2 = 0.02;
  if (randomSeed == 0)
    {
//      srand ((unsigned)time (NULL));

    }
  else
    {
      srand (randomSeed);
      double start_time_2 = 0.1 + rand()/double(RAND_MAX)/5;
      std::cout << "2nd flow start time: " << start_time_2 <<std::endl;
    }

  // long flow 2
  BulkSendHelper ftp2("ns3::TcpSocketFactory", InetSocketAddress(sink_interfaces.GetAddress(0,0),port+1));
  ftp2.SetAttribute("SendSize", UintegerValue(TCP_SEGMENT));
  ftp2.SetAttribute("MaxBytes", UintegerValue(0*data_mbytes));
  ApplicationContainer sourceApp2 = ftp2.Install(senders.Get(1));
  sourceApp2.Start(Seconds(start_time_2));
  sourceApp2.Stop(Seconds(stop_time));
  PacketSinkHelper sinkHelper2("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port+1));
  ApplicationContainer sinkApp2 = sinkHelper2.Install(receiver.Get(0));
  sinkApp2.Start(Seconds(start_time));
  sinkApp2.Stop(Seconds(stop_time));

  std::stringstream flowMonitorFilename;
  if (transport_port.compare("TcpDctcp")==0 && queue_disc_type.compare("RedQueueDisc")==0)id="tm";//DCTCP
  if (transport_port.compare("TcpDctcp")==0 && randommark==true)id="rm";//DCTCP&random mark
  if (queue_disc_type.compare("RedQueueFrontDisc")==0)id="fm";//DCTCP&front mark

  // Get queue size
  filePlotQueue << id<< "-queue-size.plotme";
  remove (filePlotQueue.str ().c_str());
  Ptr<QueueDisc> queue = queueDiscs.Get(0);
  Simulator::ScheduleNow (&CheckQueueSize, queue, filePlotQueue.str());

  // Get throughput1
  filePlotThroughput1  << id<<"_throughput1.plotme";
  remove (filePlotThroughput1.str ().c_str());
  Simulator::ScheduleNow (&ThroughputPerSecond1, sinkApp1.Get(0)->GetObject<PacketSink>(), filePlotThroughput1.str ());

  // Get throughput2
  filePlotThroughput2  << id<<"_throughput2.plotme";
  remove (filePlotThroughput2.str ().c_str());
  Simulator::ScheduleNow (&ThroughputPerSecond2, sinkApp2.Get(0)->GetObject<PacketSink>(), filePlotThroughput2.str ());

  if(pcap)
  {
	  ptpLink.EnablePcapAll(prefix_file_name, true);
	  neckLink.EnablePcapAll(prefix_file_name, true);
  }

  // Enable flow monitor
  NS_LOG_INFO ("Enabling flow monitor");
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();
  flowMonitor->CheckForLostPackets ();


  flowMonitorFilename << id << "-" << SNum << "-"<< randomSeed<<".xml";

  //AnimationInterface anim("dctcpVScubic.xml");
  Simulator::Stop (Seconds(stop_time));
  Simulator::Schedule(Seconds(0.0), &printCurrentPercent, stop_time);
  Simulator::Run ();

  flowMonitor->SerializeToXmlFile(flowMonitorFilename.str (), true, true);

  Simulator::Destroy ();
  return 0;
}
