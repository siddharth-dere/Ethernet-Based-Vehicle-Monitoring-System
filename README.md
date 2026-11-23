Ethernet-Based Vehicle Monitoring System using Linux Socket Programming📌 Project Overview & Industry ContextModern automotive Electronic Control Units (ECUs) increasingly rely on Automotive Ethernet and TCP/IP stacks to exchange high-bandwidth sensor telemetry, diagnostics, and control signals. Before working with higher-level automotive middleware such as SOME/IP or DoIP, embedded engineers must master low-level socket programming and packet-level traffic inspection.This project implements a real-time Vehicle Monitoring System in C on Linux, simulating communication between two discrete automotive nodes:Vehicle ECU (Sender / Client): Periodically samples and serializes vehicle telemetry (Vehicle Speed, Engine RPM, and Steering Angle) at $1\text{ Hz}$ and transmits it over Ethernet/IPv4.Dashboard ECU (Receiver / Server): Listens on a dedicated network port, deserializes incoming telemetry frames, and renders real-time instrument cluster readings.Core Engineering Objectives:Implement deterministic client-server communication in C using POSIX <sys/socket.h> APIs.Architect dual-protocol support for both connection-oriented (TCP) and connectionless (UDP) transport mechanisms.Perform deep-packet inspection in Wireshark to verify 3-way handshakes (SYN, SYN-ACK, ACK), payload encapsulation, and connection teardown/reset (RST, FIN) behavior.Execute fault-injection validation scenarios (offline receiver, continuous streaming, and malformed IP configurations).🏗️ System Architecture & Communication FlowBoth ECU applications execute as independent processes on Ubuntu Linux, exchanging structured binary payloads over the local loopback Ethernet interface (lo / 127.0.0.1) on port 8080.System Architecture Diagramgraph LR
    subgraph Vehicle_Node ["Vehicle ECU (Client Process)"]
        A[Telemetry Generator<br>1 Hz Sampling Loop] --> B[VehicleData Struct<br>12 Bytes Binary Payload]
        B --> C[POSIX Socket API<br>SOCK_STREAM / SOCK_DGRAM]
    end

    subgraph Network_Stack ["Linux Kernel Network Stack (lo)"]
        D[Ethernet II / IPv4<br>127.0.0.1 : Port 8080<br>Wireshark Tap Point]
    end

    subgraph Dashboard_Node ["Dashboard ECU (Server Process)"]
        E[POSIX Socket Listener<br>bind / listen / accept] --> F[Payload Deserializer]
        F --> G[Instrument Cluster Console<br>Real-Time Display]
    end

    C -->|send / sendto| D
    D -->|recv / recvfrom| E
Transport Protocol Sequence FlowsequenceDiagram
    autonumber
    participant V as Vehicle ECU (Client)
    participant D as Dashboard ECU (Server)

    Note over V,D: Mode A: UDP Communication (Connectionless)
    loop Every 1 Second (1 Hz)
        V->>D: UDP Datagram [Speed, RPM, Steering] (12 B Payload)
        Note right of D: No ACK returned; immediate display
    end

    Note over V,D: Mode B: TCP Communication (Connection-Oriented)
    V->>D: [SYN] Seq=0
    D->>V: [SYN, ACK] Seq=0, Ack=1
    V->>D: [ACK] Seq=1, Ack=1
    loop Every 1 Second (1 Hz)
        V->>D: [PSH, ACK] VehicleData Payload (12 B)
        D->>V: [ACK] Ack = Seq + 12
    end
    V->>D: [FIN, ACK] Graceful Teardown
    D->>V: [FIN, ACK]
📡 Telemetry Protocol Specification (vehicle_protocol.h)Both ECUs share a unified C header defining the network endpoint and binary message frame structure (sizeof(VehicleData) = 12 bytes):Signal FieldC Data TypeSizeRange / UnitDescriptionspeedint$4\text{ bytes}$$0 - 119\text{ km/h}$Instantaneous vehicle wheel speedrpmint$4\text{ bytes}$$800 - 3180\text{ RPM}$Engine crankshaft rotational speedsteering_anglefloat$4\text{ bytes}$$-10.0^\circ \text{ to } +9.0^\circ$Steering column angular displacement🗂️ Repository Architecture📦 Vehicle-Monitoring-System
 ┣ 📂 docs/
 ┃ ┣ 📂 source/                  # Markdown source files for technical documentation
 ┃ ┣ 📜 Design_Document.pdf      # System architecture, socket flow, and data structures
 ┃ ┣ 📜 Validation_Report.pdf    # Test execution logs and Wireshark verification across Scenarios 1-5
 ┃ ┗ 📜 TCP_UDP_Comparison.pdf   # Quantitative & architectural comparison of TCP vs. UDP
 ┣ 📂 logs/                      # Captured stdout/stderr execution logs for all test scenarios
 ┣ 📂 screenshots/               # Wireshark packet captures and multi-terminal validation evidence
 ┣ 📂 src/
 ┃ ┣ 📜 vehicle_protocol.h       # Shared telemetry struct, port, and IP configuration
 ┃ ┣ 📜 vehicle_ecu.c            # Vehicle ECU sender implementation (TCP & UDP modes)
 ┃ ┣ 📜 dashboard_ecu.c          # Dashboard ECU receiver implementation (TCP & UDP modes)
 ┃ ┗ 📜 Makefile                 # GCC build automation script
 ┣ 📂 wireshark/                 # Raw .pcapng packet capture files for deep inspection
 ┣ 📜 .gitignore                 # Excludes compiled binaries, temp docs, and virtual environments
 ┗ 📜 README.md                  # Project documentation and reproduction guide
🚀 Build & Execution Guide1. PrerequisitesEnsure gcc, make, and wireshark are installed on your Linux system:sudo apt update && sudo apt install build-essential wireshark -y
2. Compile the ECU Binariesgit clone https://github.com/siddharth-dere/Vehicle-Monitoring-System.git
cd Vehicle-Monitoring-System/src
make
3. Run the System (Dual-Terminal Execution)To run in UDP mode:# Terminal 1: Start the Dashboard ECU receiver
./dashboard_ecu udp

# Terminal 2: Start the Vehicle ECU sender
./vehicle_ecu udp
To run in TCP mode:# Terminal 1: Start the Dashboard ECU receiver
./dashboard_ecu tcp

# Terminal 2: Start the Vehicle ECU sender
./vehicle_ecu tcp
🧪 Validation Scenarios & Wireshark FindingsScenarioTest ConditionTransportObserved System & Wireshark BehaviorScenario 1Normal ECU CommunicationTCP & UDPBoth modes sustained $1\text{ Hz}$ telemetry delivery. Wireshark verified TCP 3-way handshake (SYN -> SYN-ACK -> ACK) followed by 12-byte [PSH, ACK] segments, whereas UDP transmitted datagrams immediately without setup overhead.Scenario 2Dashboard ECU OfflineTCPconnect() failed immediately (Connection refused). Wireshark captured an OS-generated [RST, ACK] packet on port 8080, confirming active connection rejection.Scenario 2Dashboard ECU OfflineUDPsendto() succeeded at the application layer without error; packets were silently dropped by the kernel (ICMP Port Unreachable) due to UDP's stateless design.Scenario 3Extended Continuous RunTCP & UDPContinuous $1\text{ Hz}$ streaming executed cleanly without socket buffer exhaustion, memory leaks, or sequence drift.Scenario 4Invalid IP (256.256.256.256)TCP & UDPinet_pton() rejected the malformed IPv4 octet string prior to socket transmission, validating input sanitization.Scenario 5Protocol Overhead & LatencyTCP vs. UDPTCP incurred $20\text{-byte}$ transport headers plus bidirectional ACK frames per packet; UDP required only an $8\text{-byte}$ header with zero return traffic.⚖️ Comparative Study: TCP vs. UDP in Automotive SystemsParameterTCP (Transmission Control Protocol)UDP (User Datagram Protocol)ReliabilityHigh: Guaranteed, in-order byte-stream delivery via sequence/acknowledgment tracking.Best-Effort: Fire-and-forget datagrams; lost packets are not retransmitted.Latency & SpeedModerate: Handshake setup and ACK waits introduce deterministic latency overhead.Ultra-Low: Zero connection setup; ideal for real-time periodic sensor streams.Header OverheadMinimum $20\text{ bytes}$ (plus TCP options, typically $32\text{ bytes}$ on Linux).Fixed $8\text{ bytes}$ (Source Port, Dest Port, Length, Checksum).Error HandlingAutomatic retransmission timers, checksums, and flow/congestion control.Checksum verification only; corrupted packets are silently discarded.Connection StateStateful (SYN -> SYN-ACK -> ACK setup; FIN/RST teardown).Stateless and connectionless.Automotive Use CasesUDS Diagnostics over IP (DoIP), ECU firmware OTA flashing, critical state commands.High-frequency CAN-over-Ethernet telemetry (Speed, RPM), LiDAR/Camera streams.
