#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

#include <iostream>
#include <iomanip>
#include <map>

using namespace ns3;

int main(int argc, char *argv[])
{
    // 1. Create nodes
    NodeContainer nodes;
    nodes.Create(4);

    // 2. Create point-to-point links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer d01 = p2p.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer d12 = p2p.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer d13 = p2p.Install(nodes.Get(1), nodes.Get(3));

    // 3. Install Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // 4. Assign IP addresses
    Ipv4AddressHelper ip;

    ip.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i01 = ip.Assign(d01);

    ip.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i12 = ip.Assign(d12);

    ip.SetBase("10.1.3.0", "255.255.255.0");
    ip.Assign(d13);

    // 5. Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 6. Flow 1: n0 -> n2, 1 Mbps, port 5000
    UdpServerHelper server1(5000);
    ApplicationContainer s1 = server1.Install(nodes.Get(2));
    s1.Start(Seconds(0.0));
    s1.Stop(Seconds(10.0));

    UdpClientHelper client1(i12.GetAddress(1), 5000);
    client1.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    client1.SetAttribute("PacketSize", UintegerValue(512));
    client1.SetAttribute("Interval",
        TimeValue(Seconds(512 * 8.0 / 1000000.0)));

    ApplicationContainer c1 = client1.Install(nodes.Get(0));
    c1.Start(Seconds(2.0));
    c1.Stop(Seconds(10.0));

    // 7. Flow 2: n3 -> n0, 500 Kbps, port 6000
    UdpServerHelper server2(6000);
    ApplicationContainer s2 = server2.Install(nodes.Get(0));
    s2.Start(Seconds(0.0));
    s2.Stop(Seconds(10.0));

    UdpClientHelper client2(i01.GetAddress(0), 6000);
    client2.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    client2.SetAttribute("PacketSize", UintegerValue(512));
    client2.SetAttribute("Interval",
        TimeValue(Seconds(512 * 8.0 / 500000.0)));

    ApplicationContainer c2 = client2.Install(nodes.Get(3));
    c2.Start(Seconds(3.0));
    c2.Stop(Seconds(10.0));

    // 8. Install FlowMonitor
    FlowMonitorHelper fmh;
    Ptr<FlowMonitor> mon = fmh.InstallAll();

    // 9. Run simulation
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    // 10. Check FlowMonitor results
    mon->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> cl =
        DynamicCast<Ipv4FlowClassifier>(fmh.GetClassifier());

    std::map<FlowId, FlowMonitor::FlowStats> stats =
        mon->GetFlowStats();

    std::cout << std::fixed << std::setprecision(4);

    for (const auto &kv : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = cl->FindFlow(kv.first);
        const auto &s = kv.second;

        // Only display the two UDP destination ports
        if (t.destinationPort != 5000 &&
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

        std::cout << "Packets Sent: " << s.txPackets << '\n';
        std::cout << "Packets Received: " << s.rxPackets << '\n';
        std::cout << "Throughput: " << mbps << " Mbps\n";
        std::cout << "Average Delay: " << delay << " ms\n";
        std::cout << "Packet Loss: " << loss << " %\n";
        std::cout << "PDR: " << pdr << " %\n";
    }

    // 11. Cleanup
    Simulator::Destroy();
    return 0;
}
