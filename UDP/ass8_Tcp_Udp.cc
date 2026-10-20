#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/traffic-control-module.h"

#include <iostream>
#include <iomanip>
#include <map>

using namespace ns3;

int main(int argc, char *argv[])
{
    // 1. Create hosts and routers
    NodeContainer hosts;
    hosts.Create(4);

    NodeContainer routers;
    routers.Create(2);

    // 2. Access links: 10 Mbps, 2 ms, DropTail 50 packets
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    p2p.SetQueue("ns3::DropTailQueue<Packet>",
                 "MaxSize", StringValue("50p"));

    NetDeviceContainer dev0r1 =
        p2p.Install(hosts.Get(0), routers.Get(0));
    NetDeviceContainer dev1r1 =
        p2p.Install(hosts.Get(1), routers.Get(0));
    NetDeviceContainer devR2N2 =
        p2p.Install(routers.Get(1), hosts.Get(2));
    NetDeviceContainer devR2N3 =
        p2p.Install(routers.Get(1), hosts.Get(3));

    // 3. Bottleneck link: 2 Mbps, 10 ms
    PointToPointHelper p2pBtl;
    p2pBtl.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
    p2pBtl.SetChannelAttribute("Delay", StringValue("10ms"));
    p2pBtl.SetQueue("ns3::DropTailQueue<Packet>",
                    "MaxSize", StringValue("50p"));

    NetDeviceContainer devBtl =
        p2pBtl.Install(routers.Get(0), routers.Get(1));

    // Expose DropTail device queue on bottleneck
    TrafficControlHelper tch;
    tch.Uninstall(devBtl);

    // 4. Install Internet stack
    InternetStackHelper internet;
    internet.Install(hosts);
    internet.Install(routers);

    // 5. Assign IP addresses
    Ipv4AddressHelper ipv4;

    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer iface0r1 = ipv4.Assign(dev0r1);

    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer iface1r1 = ipv4.Assign(dev1r1);

    ipv4.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer ifaceBtl = ipv4.Assign(devBtl);

    ipv4.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer ifaceR2N2 = ipv4.Assign(devR2N2);

    ipv4.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer ifaceR2N3 = ipv4.Assign(devR2N3);

    // 6. Populate routing tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 7. TCP congestion control setup
    Config::SetDefault("ns3::TcpSocket::SegmentSize",
                       UintegerValue(1024));

    // TCP sink on Node 3, port 8080
    PacketSinkHelper tcpSink(
        "ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), 8080));

    ApplicationContainer tSinkApp = tcpSink.Install(hosts.Get(3));
    tSinkApp.Start(Seconds(0.0));
    tSinkApp.Stop(Seconds(20.0));

    // TCP source: Node 0 -> Node 3
    BulkSendHelper tcpSource(
        "ns3::TcpSocketFactory",
        InetSocketAddress(ifaceR2N3.GetAddress(1), 8080));

    tcpSource.SetAttribute("SendSize", UintegerValue(1024));
    tcpSource.SetAttribute("MaxBytes", UintegerValue(0));

    ApplicationContainer tSrcApp = tcpSource.Install(hosts.Get(0));
    tSrcApp.Start(Seconds(1.0));
    tSrcApp.Stop(Seconds(20.0));

    // 8. UDP flow: Node 1 -> Node 2, 1 Mbps
    UdpServerHelper server1(6000);
    ApplicationContainer sApp1 = server1.Install(hosts.Get(2));
    sApp1.Start(Seconds(0.0));
    sApp1.Stop(Seconds(20.0));

    UdpClientHelper client1(ifaceR2N2.GetAddress(1), 6000);
    client1.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    client1.SetAttribute("PacketSize", UintegerValue(512));
    client1.SetAttribute("Interval",
        TimeValue(Seconds(512 * 8.0 / 1e6)));

    ApplicationContainer cApp1 = client1.Install(hosts.Get(1));
    cApp1.Start(Seconds(1.0));
    cApp1.Stop(Seconds(20.0));

    // 9. Packet tracing
    p2p.EnablePcapAll("sim-access");
    p2pBtl.EnablePcapAll("sim-bottleneck");

    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll(ascii.CreateFileStream("access.tr"));
    p2pBtl.EnableAsciiAll(ascii.CreateFileStream("bottleneck.tr"));

    // 10. Install FlowMonitor
    FlowMonitorHelper fmh;
    Ptr<FlowMonitor> mon = fmh.InstallAll();

    // 11. Run simulation
    Simulator::Stop(Seconds(20.0));
    Simulator::Run();
    mon->CheckForLostPackets();

    // 12. Analyze flow statistics
    Ptr<Ipv4FlowClassifier> cl =
        DynamicCast<Ipv4FlowClassifier>(fmh.GetClassifier());

    std::map<FlowId, FlowMonitor::FlowStats> stats =
        mon->GetFlowStats();

    std::cout << std::fixed << std::setprecision(4);

    for (const auto &kv : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = cl->FindFlow(kv.first);
        const auto &s = kv.second;

        if (t.destinationPort != 8080 &&
            t.destinationPort != 6000)
            continue;

        double dur =
            (s.timeLastRxPacket - s.timeFirstTxPacket).GetSeconds();

        double mbps = (dur > 0) ?
            (s.rxBytes * 8.0 / dur / 1e6) : 0;

        double delay = (s.rxPackets > 0) ?
            (s.delaySum.GetSeconds() / s.rxPackets * 1000.0) : 0;

        double loss = (s.txPackets > 0) ?
            (100.0 * (s.txPackets - s.rxPackets) / s.txPackets) : 0;

        double pdr = (s.txPackets > 0) ?
            (100.0 * s.rxPackets / s.txPackets) : 0;

        std::cout << "\nFlow " << kv.first
                  << " (" << t.sourceAddress
                  << " -> " << t.destinationAddress
                  << ", Port " << t.destinationPort << ")\n";

        std::cout << "Protocol: "
                  << (t.destinationPort == 8080 ? "TCP" : "UDP") << '\n';
        std::cout << "Packets Sent: " << s.txPackets << '\n';
        std::cout << "Packets Received: " << s.rxPackets << '\n';
        std::cout << "Throughput: " << mbps << " Mbps\n";
        std::cout << "Average Delay: " << delay << " ms\n";
        std::cout << "Packet Loss: " << loss << " %\n";
        std::cout << "PDR: " << pdr << " %\n";
    }

    // 13. Save FlowMonitor results
    mon->SerializeToXmlFile("flow-results.xml", true, true);

    // 14. Cleanup
    Simulator::Destroy();
    return 0;
}
