# Mini Water Tower Range — Project Checklist

*A hands-on ICS/SCADA lab built on a Raspberry Pi + 2 PCs/laptops, using C, Python, and Java*

---

## Phase 0: Environment & Repo Setup

- [X] Pick a repo name and create it (e.g. `MiniWaterTower`)
- [X] Create folder structure: `/plant-sim`, `/plc`, `/hmi`, `/attacker`, `/ids`, `/docs`, `/diagrams`
- [X] Install Docker Desktop (or Docker Engine) on your laptop
- [X] Install Docker Engine on the Raspberry Pi (`curl -sSL get.docker.com | sh`)
- [X] Confirm `docker buildx` is available (`docker buildx version`) — needed for ARM builds later
- [ ] Create a free Docker Hub account (or plan to use `docker save`/`scp` instead)
- [ ] Assign static IPs to: Pi, Laptop 1 (HMI), Laptop 2 (attacker/IDS) — write them down somewhere in `/docs`
- [ ] Confirm all three devices can ping each other on the LAN
- [ ] Decide: ROAS Pi for daily dev, Layer 3 switch reserved for Phase 4 session only

---

## Phase 1: Plant Simulator (C or C++)

- [X] Install `libmodbus` dev headers on your laptop (`apt install libmodbus-dev` or build from source)
- [X] Write a `tank_level` float variable and a `valve_open` boolean
- [X] Write the physics loop: level rises when valve open, drains at constant rate otherwise
- [X] Expose `tank_level` and `valve_open` as Modbus registers via a Modbus TCP **slave/server**
- [X] Run the loop on a fixed interval (e.g. 500ms) using `sleep`/`usleep`
- [X] Test locally: connect with `mbpoll` or a simple Modbus client script and confirm you can read the register
- [X] Test writing to the valve register manually and confirm the simulated level responds
- [ ] Write a `Dockerfile` for the plant simulator
- [ ] Build and run the container locally, confirm it still answers Modbus requests from the host
- [ ] Commit code + Dockerfile to repo

---

## Phase 2: PLC (OpenPLC)

- [ ] Pull the official `openplcproject/openplc_v3` Docker image
- [ ] Run it on the Pi (or a second device) and confirm you can reach its web UI
- [ ] Add Phase 1's plant simulator as a **remote Modbus slave device** in OpenPLC's device config
- [ ] Confirm OpenPLC can read the tank level from the plant simulator
- [ ] Write Structured Text logic: if level > 90%, force valve closed; if level < 20%, force valve open
- [ ] Compile/deploy the logic in OpenPLC
- [ ] Test end-to-end: let the simulated tank fill and confirm the PLC closes the valve automatically
- [ ] Confirm OpenPLC itself is reachable as a Modbus server (this is what the HMI will poll)
- [ ] Document the OpenPLC config (screenshot or exported config file) in `/docs`

---

## Phase 3: HMI / SCADA Dashboard (Java or Python)

- [ ] Decide: desktop GUI (Java Swing/JavaFX) or web dashboard (Java Javalin/Spark or Python Flask) — web recommended for portability
- [ ] Add a Modbus client library (`jlibmodbus` for Java, `pymodbus` for Python)
- [ ] Poll the PLC (not the plant directly) every 1 second for tank level and valve state
- [ ] Display tank level as a simple gauge or bar
- [ ] Log historical readings to a file or SQLite database
- [ ] Add a manual override control (button) to force the valve open/closed via a Modbus write
- [ ] Test the override and confirm it changes PLC/plant behavior
- [ ] If web-based: containerize it (Dockerfile) so it's reachable from any device on the LAN
- [ ] If desktop: package as a runnable `.jar` and confirm it runs unmodified on both laptop and Pi (if desired)
- [ ] Commit code + Dockerfile (if applicable) to repo

---

## Phase 4: Network Segmentation

- [ ] Power on the Layer 3 switch for this session (or use the ROAS Pi setup if skipping the switch)
- [ ] Define 3 logical segments: Plant/PLC (Level 0/1), HMI (Level 2), Attacker/IDS (untrusted)
- [ ] Configure VLANs (on switch) or sub-interfaces (on ROAS Pi) for each segment
- [ ] Assign each device to its correct VLAN/segment
- [ ] Write firewall rules (`iptables`/`ufw` on each host, or switch ACLs) allowing only:
  - [ ] Plant simulator ↔ PLC on port 502 (Modbus)
  - [ ] PLC ↔ HMI on port 502
  - [ ] Block attacker segment from reaching Plant or PLC directly
- [ ] Test: confirm HMI can still read PLC data after rules are applied
- [ ] Test: confirm a device outside the allowed rules is blocked (try from Laptop 2 before Phase 6)
- [ ] Draw a Purdue-model network diagram (CAD tool: QElectroTech, KiCad, or even draw.io) showing all segments and allowed flows
- [ ] Save diagram + firewall rules + switch config screenshots to `/diagrams` and `/docs`
- [ ] Power down the switch when done if noise is an issue; keep config exported/saved

---

## Phase 5: Monitoring (IDS)

- [ ] Install Wireshark on Laptop 2
- [ ] Capture traffic during normal operation (plant running, PLC polling, HMI updating)
- [ ] Identify and screenshot: a normal Modbus **read** request/response, a normal Modbus **write** request/response
- [ ] Install Zeek or Suricata (Docker image is fine) on Laptop 2
- [ ] Write one custom rule/script: alert on any Modbus **write** command from an IP other than the HMI's
- [ ] Test the rule by manually sending a write from an unauthorized IP (can be a quick script or `mbpoll` from the wrong device)
- [ ] Confirm the alert fires correctly
- [ ] Save the rule file and a sample alert log to `/docs`

---

## Phase 6: The Attack (Python + Scapy)

- [ ] Install Scapy on Laptop 2
- [ ] Write a script that crafts a raw Modbus TCP write packet to force the valve open
- [ ] Spoof the source IP to match the HMI's address
- [ ] Send the packet toward the PLC (or plant, depending on what you're testing)
- [ ] Observe the simulated tank overflow in the Phase 1 program's output/logs
- [ ] Confirm the Phase 5 IDS rule catches the spoofed write (or note if it doesn't, and why)
- [ ] If segmentation (Phase 4) is active, confirm whether the attack is blocked at the network layer before it even reaches the PLC
- [ ] Save the attack script, a packet capture of the attack, and before/after screenshots to `/docs`

---

## Phase 7: Write-Up

- [ ] Write a short system overview: what each component does and how they connect
- [ ] Describe the vulnerability found (e.g. Modbus has no authentication or encryption)
- [ ] Describe the attack performed and its impact (simulated overflow)
- [ ] Describe the detection method and whether it worked
- [ ] Describe the segmentation/mitigation and whether it would have prevented the attack
- [ ] Give a clear recommendation (e.g. IP allow-listing, move to an authenticated protocol, add the IDS rule in production)
- [ ] Include the network diagram from Phase 4
- [ ] Include key screenshots: OpenPLC config, HMI dashboard, Wireshark capture, IDS alert
- [ ] Proofread and keep it concise — aim for something a hiring manager could read in 5 minutes

---

## Phase 8: Docker Multi-Arch Deployment (cross-cutting)

- [ ] Confirm which components need ARM builds for the Pi (plant simulator, attacker scripts, IDS)
- [ ] Set up a `docker buildx` builder instance (`docker buildx create --use`)
- [ ] Build multi-platform images (`--platform linux/amd64,linux/arm64`) for each containerized component
- [ ] Push images to Docker Hub, or `docker save` + `scp` to each device if avoiding a registry
- [ ] Pull/load images on the Pi and confirm they run correctly
- [ ] Write a `docker-compose.yml` for local testing on your laptop (all services on one bridge network)
- [ ] Write per-device run instructions (or separate compose files) for the real distributed deployment
- [ ] Document the deployment architecture (which container runs on which physical device) in `/docs`

---

## Final Portfolio Checklist

- [ ] All code committed with clear commit history
- [ ] README with project summary, architecture diagram, and "what I learned"
- [ ] Write-up (Phase 7) included or linked
- [ ] Repo is public and well-organized
- [ ] One or two sentences about this project ready to drop into a resume/cover letter