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

NS_LOG_COMPONENT_DEFINE ("fifo-vs-red");

std::stringstream filePlotQueue;
std::stringstream filePlotThroughput;

static bool firstCwnd = true;
static bool firstSshThr = true;
static bool firstRtt = true;
static bool firstRto = true;
static Ptr<OutputStreamWrapper> cWndStream;
static Ptr<OutputStreamWrapper> ssThreshStream;
static Ptr<OutputStreamWrapper> rttStream;
static Ptr<OutputStreamWrapper> rtoStream;
static uint32_t cWndValue;
static uint32_t ssThreshValue;
uint64_t flowRecvBytes = 0;

static void
CwndTracer (uint32_t oldval, uint32_t newval)
{
  if (firstCwnd)
    {
      *cWndStream->GetStream () << "0.0 " << oldval << std::endl;
      firstCwnd = false;
    }
  *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  cWndValue = newval;

  if (!firstSshThr)
    {
      *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << " " << ssThreshValue << std::endl;
    }
}

static void
SsThreshTracer (uint32_t oldval, uint32_t newval)
{
  if (firstSshThr)
    {
      *ssThreshStream->GetStream () << "0.0 " << oldval << std::endl;
      firstSshThr = false;
    }
  *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  ssThreshValue = newval;

  if (!firstCwnd)
    {
      *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << cWndValue << std::endl;
    }
}

static void
RttTracer (Time oldval, Time newval)
{
  if (firstRtt)
    {
      *rttStream->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRtt = false;
    }
  *rttStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}

static void
RtoTracer (Time oldval, Time newval)
{
  if (firstRto)
    {
      *rtoStream->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRto = false;
    }
  *rtoStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}


static void
TraceCwnd (std::string cwnd_tr_file_name)
{
  AsciiTraceHelper ascii;
  cWndStream = ascii.CreateFileStream (cwnd_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));
}

static void
TraceSsThresh (std::string ssthresh_tr_file_name)
{
  AsciiTraceHelper ascii;
  ssThreshStream = ascii.CreateFileStream (ssthresh_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold", MakeCallback (&SsThreshTracer));
}

static void
TraceRtt (std::string rtt_tr_file_name)
{
  AsciiTraceHelper ascii;
  rttStream = ascii.CreateFileStream (rtt_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/RTT", MakeCallback (&RttTracer));
}

static void
TraceRto (std::string rto_tr_file_name)
{
  AsciiTraceHelper ascii;
  rtoStream = ascii.CreateFileStream (rto_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/RTO", MakeCallback (&RtoTracer));
}

void
CheckQueueSize (Ptr<QueueDisc> queue, std::string filePlotQueue)
{
  uint32_t qSize = StaticCast<FifoQueueDisc> (queue)->GetNPackets();

  // check queue size every 1/100 of a second
  Simulator::Schedule (Seconds (0.001), &CheckQueueSize, queue, filePlotQueue);

  std::ofstream fPlotQueue (filePlotQueue.c_str (), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue.close ();
}

void
ThroughputPerSecond(Ptr<PacketSink> sink1Apps,std::string filePlotThrough)
{
	uint32_t totalRecvBytes = sink1Apps->GetTotalRx();
	uint32_t currentPeriodRecvBytes = totalRecvBytes - flowRecvBytes;

	flowRecvBytes = totalRecvBytes;

	Simulator::Schedule (Seconds(0.02), &ThroughputPerSecond, sink1Apps, filePlotThrough);
	std::ofstream fPlotThroughput (filePlotThrough.c_str (), std::ios::out | std::ios::app);
	fPlotThroughput << Simulator::Now().GetSeconds() << " " << currentPeriodRecvBytes * 8 / (0.02 * 1000000) << std::endl;
}
//void
//PrintPayload(Ptr<const Packet> p)
//{
//	std::ofstream fPlotQueue("throught.dat",std::ios::out | std::ios::app);
//	fPlotQueue << Simulator::Now().GetSeconds()<< " " << p->GetSize() << std::endl;
//	fPlotQueue.close();
//}

int main (int argc, char *argv[])
{
  double minRto = 0.2;
  uint32_t initialCwnd = 1;
  uint32_t sendNum = 3;
  double start_time = 0;
  double stop_time = 10;
  uint64_t data_mbytes = 1;
  uint32_t PACKET_SIZE = 1400;
  std::string bandwidth = "100Mbps";
  std::string delay = "2ms";
  std::string neckbandwidth = "30Mbps";
  std::string neckdelay = "2ms";
  std::string transport_port = "TcpNewReno";
  std::string queue_disc_type = "ns3::FifoQueueDisc";
  //bool front_model = true;

  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer,sizeof(buffer),"%d-%m-%Y-%I-%M-%S",timeinfo);
  std::string currentTime (buffer);

  bool tracing = true;
  //bool writeForPlot = true;
  bool printRedStats = false;
  bool printFifoStats = true;
  //std::string pathOut = ".";
  std::string prefix_file_name = "fifo-vs-red";
  bool pcap = true;

  CommandLine cmd;
  cmd.AddValue ("sendNum",     "Number of left and right side leaf nodes", sendNum);
  cmd.AddValue ("bandwidth", "Bottleneck bandwidth", bandwidth);
  cmd.AddValue ("delay", "Bottleneck delay", delay);
  cmd.AddValue ("mtu", "Size of IP packets to send in bytes", PACKET_SIZE);
  cmd.AddValue ("data", "Number of Megabytes of data to transmit", data_mbytes);
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
  LogComponentEnable("fifo-vs-red", LOG_LEVEL_INFO);
  //LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);
  //LogComponentEnable("RedQueueDisc", LOG_LEVEL_ALL);
  //LogComponentEnable("FifoQueueDisc", LOG_LEVEL_LOGIC);
  //LogComponentEnable("Ipv4QueueDiscItem", LOG_LEVEL_DEBUG);

  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (PACKET_SIZE));

  NS_LOG_INFO ("bandwidth="+bandwidth);
  NS_LOG_INFO ("delay="+delay);

  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (initialCwnd));
  Config::SetDefault ("ns3::TcpSocketBase::MinRto", TimeValue (Seconds (minRto)));
  Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (true));
  Config::SetDefault("ns3::TcpSocketState::EnablePacing", BooleanValue (true));

  transport_port = std::string("ns3::") + transport_port;

  // Configure NewReno
  NS_LOG_INFO ("TcpNewReno");
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId()));
  //Config::SetDefault("ns3::TcpSocketBase::EcnMode", EnumValue(1));

  // Configure RED
  Config::SetDefault("ns3::RedQueueDisc::MeanPktSize", UintegerValue (PACKET_SIZE));
  //Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue (1));
  Config::SetDefault("ns3::RedQueueDisc::MaxSize", QueueSizeValue (QueueSize ("100p")));
  Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue (10));
  Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue (30));
  //Config::SetDefault("ns3::RedQueueDisc::UseHardDrop", BooleanValue (false));
  //Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue (true));
  //Config::SetDefault("ns3::RedQueueDisc::Gentle", BooleanValue (false));
  //Config::SetDefault("ns3::RedQueueDisc::frontMode", BooleanValue (front_model));

  // Configure FIFO
  Config::SetDefault("ns3::FifoQueueDisc::MaxSize", QueueSizeValue (QueueSize ("100p")));

  // Create sendNum, receiver, and switches
  NodeContainer switches;
  switches.Create(1);
  NodeContainer senders;
  senders.Create(sendNum);
  NodeContainer receiver;
  receiver.Create(1);

  // Configure channel attributes
  PointToPointHelper ptpLink;
  ptpLink.SetDeviceAttribute("DataRate", StringValue(bandwidth));
  ptpLink.SetChannelAttribute("Delay", StringValue(delay));

  PointToPointHelper neckLink;
  neckLink.SetDeviceAttribute("DataRate", StringValue(neckbandwidth));
  neckLink.SetChannelAttribute("Delay", StringValue(neckdelay));

  // Install InternetStack
  InternetStackHelper stack;
  stack.InstallAll();

  // Configure TrafficControlHelper
  TrafficControlHelper tchRed,tchFiFo;
  tchRed.SetRootQueueDisc("ns3::RedQueueDisc");
  tchFiFo.SetRootQueueDisc("ns3::FifoQueueDisc");

  // Configure Ipv4AddressHelper
  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.255.255.0");

  // Configure net devices in nodes and connect sendNum with switches
  for(uint32_t i = 0; i<sendNum; i++)
  {
	  NetDeviceContainer devices;
	  devices = ptpLink.Install(senders.Get(i), switches.Get(0));
	  //tchRed.Install(devices);
	  address.NewNetwork();
	  Ipv4InterfaceContainer interfaces = address.Assign(devices);
  }

  // Connect switches with receiver
  NetDeviceContainer devices;
  devices = neckLink.Install(switches.Get(0), receiver.Get(0));

  // Install queueDiscs in switch
  QueueDiscContainer queueDiscs;
  if(queue_disc_type.compare("ns3::FifoQueueDisc") == 0)
  {
	  NS_LOG_INFO ("Install FIFO");
	  queueDiscs = tchFiFo.Install(devices);
  }
  else if(queue_disc_type.compare("ns3::RedQueueDisc") == 0)
  {
	  NS_LOG_INFO ("Install RED");
	  queueDiscs = tchRed.Install(devices);
  }

  address.NewNetwork();
  Ipv4InterfaceContainer interfaces = address.Assign(devices);
  Ipv4InterfaceContainer sink_interfaces;
  sink_interfaces.Add(interfaces.Get(1));

  // Configure routing
  NS_LOG_INFO ("Initialize Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Configure port/address and install application
  uint16_t port = 50000;

  for(uint16_t i=0; i<senders.GetN(); i++)
  {
	  // BulkSend
	  BulkSendHelper ftp("ns3::TcpSocketFactory", InetSocketAddress(sink_interfaces.GetAddress(0,0),port));
	  ftp.SetAttribute("SendSize", UintegerValue(PACKET_SIZE));
	  ftp.SetAttribute("MaxBytes", UintegerValue(data_mbytes*0));
	  ApplicationContainer sourceApp = ftp.Install(senders.Get(i));
	  sourceApp.Start(Seconds(start_time));
	  sourceApp.Stop(Seconds(stop_time - 0.2));
  }

  //for(uint16_t i=0; i<senders.GetN(); i++){}

  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port));
  //sinkHelper.SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
  ApplicationContainer sinkApp = sinkHelper.Install(receiver.Get(0));
  sinkApp.Start(Seconds(start_time));
  sinkApp.Stop(Seconds(stop_time));

  // Collect information
  std::string dir = "results/" + transport_port.substr(5, transport_port.length()) + "/" + currentTime + "/";
  if (tracing)
  {
	  std::string dirToSave = "mkdir -p " + dir;
	  system (dirToSave.c_str ());
	  Simulator::Schedule (Seconds (start_time + 0.000001), &TraceCwnd, dir + "cwnd.data");
	  Simulator::Schedule (Seconds (start_time + 0.000001), &TraceSsThresh, dir + "ssth.data");
	  Simulator::Schedule (Seconds (start_time + 0.000001), &TraceRtt, dir + "rtt.data");
	  Simulator::Schedule (Seconds (start_time + 0.000001), &TraceRto, dir + "rto.data");

	  filePlotQueue << dir << "/" << "queue-size.plotme";

	  //remove (filePlotQueue.str ().c_str());
	  Ptr<QueueDisc> queue = queueDiscs.Get(0);
	  Simulator::ScheduleNow (&CheckQueueSize, queue, filePlotQueue.str());

	  filePlotThroughput << dir << "/" << "throughput.plotme";
	  //remove (filePlotThroughput.str ().c_str());
	  Simulator::ScheduleNow (&ThroughputPerSecond, sinkApp.Get(0)->GetObject<PacketSink>(), filePlotThroughput.str ());
  }

  if(pcap)
  {
	  ptpLink.EnablePcapAll(prefix_file_name, true);
  }

  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  std::ofstream myfile;
  myfile.open (dir + "config.txt", std::fstream::in | std::fstream::out | std::fstream::app);
  myfile << "minRto " << minRto << "\n";
  myfile << "sendNum " << sendNum << "\n";
  myfile << "bandwidth " << bandwidth << "\n";
  myfile << "delay  " << delay << "\n";
  myfile << "PACKET_SIZE " << std::to_string(PACKET_SIZE) << "\n";
  myfile << "data  " << std::to_string(data_mbytes) << "\n";
  myfile << "initialCwnd  " << initialCwnd << "\n";
  myfile << "transport_prot " << transport_port << "\n";
  myfile.close();

  AnimationInterface anim("fifoVSred.xml");
  Simulator::Stop (Seconds (stop_time));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
      std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 * 1000000000 / NanoSeconds(i->second.timeLastRxPacket-i->second.timeFirstTxPacket) / 1000 / 1000  << " Mbps\n";
      std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  Throughput: " << i->second.rxBytes * 8.0 * 1000000000 / NanoSeconds(i->second.timeLastRxPacket-i->second.timeFirstTxPacket) / 1000 / 1000  << " Mbps\n";
      std::cout << "  FCT:  " << NanoSeconds(i->second.timeLastRxPacket-i->second.timeFirstTxPacket)  << "\n";
    }

  if(printRedStats)
  {
	  RedQueueDisc::Stats st = StaticCast<RedQueueDisc> (queueDiscs.Get(0))->GetStats();
	  NS_LOG_INFO("*** RED Static from switch queue ***");
	  NS_LOG_INFO(st.nTotalReceivedPackets << " : packets received totally");
	  NS_LOG_INFO(st.nTotalEnqueuedPackets << " : packets enqueued totally");
	  NS_LOG_INFO(st.nTotalDroppedPackets << " : packets dropped totally");
	  NS_LOG_INFO(st.nTotalDroppedPacketsBeforeEnqueue << " : packets dropped before enqueue totally");
	  NS_LOG_INFO(st.nTotalMarkedPackets << " : packets marked totally");
  }
  if(printFifoStats)
  {
	  FifoQueueDisc::Stats st = StaticCast<FifoQueueDisc> (queueDiscs.Get(0))->GetStats();
	  NS_LOG_INFO("*** FIFO Static from switch queue ***");
	  NS_LOG_INFO(st.nTotalReceivedPackets << " : packets received totally");
	  NS_LOG_INFO(st.nTotalEnqueuedPackets << " : packets enqueued totally");
	  NS_LOG_INFO(st.nTotalDroppedPackets << " : packets dropped totally");
	  NS_LOG_INFO(st.nTotalDroppedPacketsBeforeEnqueue << " : packets dropped before enqueue totally");
  }

  Simulator::Destroy ();
  return 0;
}
