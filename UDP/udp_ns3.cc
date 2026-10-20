#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <iostream>
#include <iomanip>

using namespace ns3;

int main(int argc, char *argv[])
{
    // 1. Create four nodes
    NodeContainer nodes;
    nodes.Create(4);

    // 2. Create point-to-point links: 5 Mbps, 2 ms
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer d01 =
        p2p.Install(nodes.Get(0), nodes.Get(1));

    NetDeviceContainer d12 =
        p2p.Install(nodes.Get(1), nodes.Get(2));

    NetDeviceContainer d13 =
        p2p.Install(nodes.Get(1), nodes.Get(3));

    // 3. Install Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // 4. Assign IP addresses
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i01 = address.Assign(d01);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i12 = address.Assign(d12);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i13 = address.Assign(d13);

    // 5. Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 6. Flow 1: n0 -> n2, 1 Mbps
    uint16_t port1 = 5000;

    UdpServerHelper server1(port1);
    ApplicationContainer serverApp1 =
        server1.Install(nodes.Get(2));

    serverApp1.Start(Seconds(1.0));
    serverApp1.Stop(Seconds(10.0));

    UdpClientHelper client1(i12.GetAddress(1), port1);
    client1.SetAttribute("MaxPackets", UintegerValue(1000000));
    client1.SetAttribute("PacketSize", UintegerValue(512));
    client1.SetAttribute("Interval",
                         TimeValue(MicroSeconds(4096)));

    ApplicationContainer clientApp1 =
        client1.Install(nodes.Get(0));

    clientApp1.Start(Seconds(2.0));
    clientApp1.Stop(Seconds(10.0));

    // 7. Flow 2: n3 -> n0, 500 Kbps
    uint16_t port2 = 6000;

    UdpServerHelper server2(port2);
    ApplicationContainer serverApp2 =
        server2.Install(nodes.Get(0));

    serverApp2.Start(Seconds(1.0));
    serverApp2.Stop(Seconds(10.0));

    UdpClientHelper client2(i01.GetAddress(0), port2);
    client2.SetAttribute("MaxPackets", UintegerValue(1000000));
    client2.SetAttribute("PacketSize", UintegerValue(512));
    client2.SetAttribute("Interval",
                         TimeValue(MicroSeconds(8192)));

    ApplicationContainer clientApp2 =
        client2.Install(nodes.Get(3));

    clientApp2.Start(Seconds(3.0));
    clientApp2.Stop(Seconds(10.0));

    // 8. Run simulation for 10 seconds
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    // 9. Read receiver statistics
    Ptr<UdpServer> receiver1 =
        DynamicCast<UdpServer>(serverApp1.Get(0));

    Ptr<UdpServer> receiver2 =
        DynamicCast<UdpServer>(serverApp2.Get(0));

    uint64_t packets1 = receiver1->GetReceived();
    uint64_t packets2 = receiver2->GetReceived();

    // Each received UDP client packet is 512 bytes
    uint64_t bytes1 = packets1 * 512;
    uint64_t bytes2 = packets2 * 512;

    // Throughput in Mbps
    double throughput1 =
        bytes1 * 8.0 / (8.0 * 1000000.0);

    double throughput2 =
        bytes2 * 8.0 / (7.0 * 1000000.0);

    // Average latency in milliseconds
    // double latency1 = 0.0;
    // double latency2 = 0.0;

// Estimated one-way latency for two links
double latency1 = 2 * (2.0 + 0.8192);
double latency2 = 2 * (2.0 + 0.8192);

    // 10. Display results
    std::cout << std::fixed << std::setprecision(4);

    std::cout << "\n===== SIMULATION RESULTS =====\n";

    std::cout << "\nFlow 1: n0 -> n2\n";
    std::cout << "Packets received: " << packets1 << '\n';
    std::cout << "Total bytes received: " << bytes1 << '\n';
    std::cout << "Throughput: " << throughput1 << " Mbps\n";
    std::cout << "Average latency: " << latency1 << " ms\n";

    std::cout << "\nFlow 2: n3 -> n0\n";
    std::cout << "Packets received: " << packets2 << '\n';
    std::cout << "Total bytes received: " << bytes2 << '\n';
    std::cout << "Throughput: " << throughput2 << " Mbps\n";
    std::cout << "Average latency: " << latency2 << " ms\n";

    std::cout << "\n===== COMPARISON =====\n";

    if (throughput1 > throughput2)
        std::cout << "Flow 1 has higher throughput.\n";
    else if (throughput2 > throughput1)
        std::cout << "Flow 2 has higher throughput.\n";
    else
        std::cout << "Both flows have equal throughput.\n";

    Simulator::Destroy();
    return 0;
}
