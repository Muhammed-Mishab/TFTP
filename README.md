# TFTP File Transfer System (C)

## Overview
This project implements a **Trivial File Transfer Protocol (TFTP)** based file transfer system using C.  
It enables file transfer between a client and server over a network using **UDP sockets**.

---

## Features
- Client-Server architecture using UDP
- Supports **file upload (put)** and **download (get)**
- Implements core TFTP operations: **RRQ, WRQ, DATA, ACK, ERROR**
- Supports multiple transfer modes:
  - Normal
  - Octet
  - Netascii
- Reliable data transfer using **block-based communication and acknowledgments**

---

## 🛠️ Tech Stack
- Language: C
- Networking: UDP Socket Programming
- OS: Linux
- Tools: GCC, VS Code, Git

---

## ⚙️ How It Works
1. Client connects to the server using IP and port
2. User selects operation:
   - `get` → Download file from server
   - `put` → Upload file to server
3. File is divided into **512-byte data packets**
4. Each packet is sent with a **block number**
5. Receiver sends **ACK** for each packet
6. Transfer continues until all packets are sent

---

## 📂 Project Structure
```bash
tftp/
│── tftp.c            # Core file transfer logic
│── tftp.h            # Packet structure & definitions
│── tftp_client.c     # Client implementation
│── tftp_client.h     # Client header
│── tftp_server.c     # Server implementation
