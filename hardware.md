# Hardware

This page contains basic concepts I stumbled upon during my journey on learning low level programming :)

## Bus

- a communication system that transfers data between components inside a computer, or between computers.
- the graphics card talks to the processor using the computer's bus as a communication path.
- the advantage of a bus is that it makes parts more interchangeable.

## System bus or local bus

- is a single bus that connects the major components of a computer system, combining the functions of a data bus to carry information, an address bus to determine where it should be sent, and a control bus to determine its operation.
- the technique was developed to reduce costs and improve modularity, and although popular in the 1970s and 1980s, more modern computers use a variety of separate buses adapted to more specific needs.
- is the fastest bus in the system.

<p align="center">
    <img src="https://i.imgur.com/XoCAfrl.png" width="400px" height="auto">
</p>

## Slower Bus

- for communicating with things like hard disks and sound cards.
- one very common bus of this type is known as the PCI bus.
- these slower buses connect to the system bus through a bridge, which is a part of the computer's chipset and acts as a traffic cop, integrating the data from the other buses to the system bus.

## Industry Standard Architecture (ISA)

- is the 16-bit internal bus of IBM PC/AT and similar computers based on the Intel 80286 and its immediate successors during the 1980s.
- the bus was (largely) backward compatible with the 8-bit bus of the 8088-based IBM PC, including the IBM PC/XT as well as IBM PC compatibles.
- superseded by PCI and AGP.
- speed: 8MB/s, up to 16 devices.
- was the basis for development of the ATA interface, used for Parallel ATA (IDE) and more recently Serial ATA (SATA) hard disks.

## Peripheral Component Interconnect (PCI)

- PCI presents a hybrid of sorts between ISA and VL-Bus.
- provides direct access to system memory for connected devices, but uses a bridge to connect to the frontside bus and therefore to the CPU. Basically, this means that it is capable of even higher performance than VL-Bus while eliminating the potential for interference with the CPU.

<p align="center">
    <img src="https://i.imgur.com/EpWARck.png" width="300px" height="auto">
</p>

## Bridge

- a PCI bridge is a hardware connection between two different buses.
- the bridge may be PCI to PCI, PCI to ISA or some other kind of bus. Bus standards are limited by electrical characteristics. - adding devices called bridges allows the buses to be expanded.
- a PCI bridge allows expansion of the PCI bus by adding more buses to the system.

## Serial Bus

- many communication systems were generally designed to connect two integrated circuits on the same printed circuit board, connected by signal traces on that board (rather than external cables).
- integrated circuits are more expensive when they have more pins. To reduce the number of pins in a package, many ICs use a serial bus to transfer data when speed is not important. Some examples of such low-cost serial buses include RS-232, SPI, I²C, DC-BUS, UNI/O, 1-Wire and PCI Express.

## SMBus

- **System Management Bus** (SMBus or SMB) is a single-ended simple two-wire bus for the purpose of lightweight communication.
- Most commonly it is found in computer motherboards for communication with the power source for ON/OFF instructions.

## Chipset

- is a set of (chips) electronic components in an integrated circuit known as a "Data Flow Management System" that manages the data flow between the processor, memory and peripherals.
- usually designed to work with a specific family of microprocessors.
- historically, chips (for keyboard controller, memory controller, ...) were scattered around the motherboard. With time, engineers reduced the number of chips to the same job and condensed them to only a few chips or what is now called a **chipset**.
- recently, the north bridge were built inside the CPU to maximize performance (1 jump instead of 2 from south bridge + performance sensitive devices talks directly to the CPU without the latency added by north bridge) and the south bridge is called **Platform Controller Hub**.

## Northbridge/Southbridge

- **Northbridge** or (host bridge) is the chip whose main role is to connect the CPU and the RAM memory. It also connects the CPU to the AGP and PCI Express slots. These are all processes which have to go really fast.
- The **Southbridge** is more relaxed and connects the CPU via the Northbridge to the I/O devices. E.g. ATA hard drives, USBs, standard PCI slots, sound and Ethernet. These connections (bridges) are actually very fast but by comparison with the CPU are they slow.

<p align="center">
  <img src="https://i.imgur.com/hBE3n10.png" width="300px" height="auto">
</p>

## Memory-mapped IO vs Port-mapped IO

- if the device is accessible in a dedicated address space, separate from the address space or memory, then the type of IO is called **port-mapped IO** or **isolated IO (PMIO)**.
- if the device is accessible as part of an unique address space, where memory is also located, then the type of IO is called **memory-mapped IO (MMIO)**.

| Memory-mapped IO                                     | Port-mapped IO                                                 |
| ---------------------------------------------------- | -------------------------------------------------------------- |
| Same address bus to address memory and I/O devices   | Different address spaces for memory and I/O devices            |
| Access to the I/O devices using regular instructions | Uses a special class of CPU instructions to access I/O devices |
| Most widely used I/O method                          | x86 Intel microprocessors: IN and OUT instructions             |

- http://www.bogotobogo.com/Embedded/memory_mapped_io_vs_port_mapped_isolated_io.php

## Programmed-IO vs Interrupt-Driven IO & Direct Memory Access

- In **Progammed I/O (PIO)**, the CPU manually check if there are any I/O requests available periodically **polling**. If there isn't, it keeps executing its normal workflow. If there is, it handles the IO request instead.
- In **Interrupt-Driven I/O**, the CPU doesn't need to manually check for IO requests. When there is an I/O request available, the CPU is immediately notified using **interrupts**, and the request is immediately handled using a interrupt service routines. The use of DMA allows interrupt-driven IO to be used. Otherwise, a system must use programmed I/O if DMA is not available.
- **Direct Memory Access (DMA)** is a method allowing devices (typically has very slow I/O speeds) to access main memory without needing the CPU to explicitly handle the requests. When the CPU initiates data transfer from IO devices to main memory, the CPU instructs the DMA controller to handle this task. The CPU will then "forget" about this operation, and proceed with other tasks. When the DMA controller has completed the transfer, it will signal the CPU using an interrupt. The CPU will then "conclude" the task needed associated with the data transfer it initiated.

## Video RAM

- **Video RAM (VRAM)** is a dual-ported variant of dynamic RAM (DRAM), which was once commonly used to store the [framebuffer](https://en.wikipedia.org/wiki/Framebuffer) in graphics adapters.
- System RAM is great for loading and running programs, but when you need graphics power, VRAM is where it's at. This is the memory used to store image data that the computer displays; it acts as a buffer between the CPU and the video card. When a picture is to be displayed on the screen, the image is first read by the processor and then written to the VRAM.

<p align="center">
    <img src="https://i.imgur.com/waiV3E0.png" width="600px" height="auto">
</p>

## Programmable interrupt controller (PIC) vs Advanced Programmable Interrupt Controller (APIC)

- PIC one of the most important chips making up the x86 architecture, its role is to manage hardware interrupts and send them to the appropriate system interrupt. This allows the system to respond to devices needs without loss of time (from polling the device, for instance).
- The 8259 PIC controls the CPU's interrupt mechanism, by accepting several interrupt requests and feeding them to the processor in order.
- Each PIC has 8 input lines, called **Interrupt Requests (IRQ)**, numbered from 0 to 7.
- It is important to note that APIC has replaced the 8259 PIC in more modern systems, especially those with multiple cores/processors.
- The APIC is used for sophisticated interrupt redirection, and for sending interrupts between processors. These things weren't possible using the older PIC specification.
- APIC enables the construction of multiprocessor systems, It is one of several architectural designs intended to solve interrupt routing efficiency issues in multiprocessor computer systems.
- The [APIC](https://wiki.osdev.org/APIC) is a split architecture design, with a local component (LAPIC) usually integrated into the processor itself, and an optional I/O APIC on a system bus.
- The `CPUID.01h:EDX[bit 9]` flag specifies whether a CPU has a built-in local APIC.

## Programmable Interval Timer (PIT)

- is a counter that generates an output signal when it reaches a programmed count.
- The output signal may trigger an interrupt.
- PITs may be one-shot or periodic.
- One-shot timers will signal only once and then stop counting.
- Periodic timers signal every time they reach a specific value and then restart, thus producing a signal at periodic intervals.

## Input–output memory management unit (IOMMU)

- is a memory management unit (MMU) that connects a DMA-capable I/O bus to the main memory.
- Like a traditional MMU, which translates CPU-visible virtual addresses to physical addresses, the IOMMU maps device-visible virtual addresses (also called device addresses or I/O addresses in this context) to physical addresses.
- IOMMU allows guest VMs to directly use peripheral devices, such as Ethernet, accelerated graphics cards, and hard-drive controllers, through DMA and **interrupt remapping**. This is sometimes called **PCI passthrough**.
  <p align="center"> <img src="https://upload.wikimedia.org/wikipedia/commons/d/d6/MMU_and_IOMMU.svg" width="400px" height="auto">
  </p>

## Non-Maskable Interrupt (NMI)

- Is a hardware interrupt that standard interrupt-masking techniques in the system cannot ignore.
- It typically occurs to signal attention for non-recoverable hardware errors. (Some NMIs may be masked, but only by using proprietary methods specific to the particular NMI.)

## Inter-processor interrupts (IPIs)

- Is a special type of interrupt by which one processor may interrupt another processor in a multiprocessor system if the interrupting processor requires action from the other processor.
- Actions that might be requested include:
  - flushes of memory management unit caches, such as translation look-aside buffers, on other processors when memory mappings are changed by one processor
  - stopping when the system is being shut down by one processor.

## E820

- is shorthand to refer to the facility by which the BIOS of x86-based computer systems reports the memory map to the operating system or boot loader.
- it is accessed via the int 15h call, by setting the AX register to value E820 in hexadecimal. It reports which memory address ranges are usable and which are reserved for use by the BIOS.

## Bus snooping

- Bus snooping or bus sniffing is a scheme by which a coherency controller (snooper) in a cache monitors or snoops the bus transactions, and its goal is to maintain a cache coherency in distributed shared memory systems.
- A cache containing a coherency controller (snooper) is called a snoopy cache.

## Load-Store Architecture

- is an ISA that divides instructions into two categories:
  - memory access (load and store between memory and registers)
  - and ALU operations (which only occur between registers).
- RISC ISAs such as PowerPC, SPARC, RISC-V, ARM, and MIPS are load–store architectures.
- For instance, in a load–store approach both operands and destination for an ADD operation must be in registers. This differs from a register–memory architecture (for example, a CISC instruction set architecture such as x86) in which one of the operands for the ADD operation may be in memory, while the other is in a register.

## Memory Controller

- is a digital circuit that manages the flow of data going to and from the computer's main memory.
- is sometimes also called a **memory chip controller (MCC)** or a **memory controller unit (MCU)**.
- can be a **separate** chip or **integrated** into another chip, such as being placed on the same die or as an integral part of a microprocessor.
- in the latter case, it is usually called an **integrated memory controller (IMC)**.
- most modern desktop or workstation microprocessors use an integrated memory controller (IMC), including microprocessors from Intel, AMD, and those built around the ARM architecture.
- one of the downfalls to RAM is that it must be supplied with a constant flow of power in order to operate. As soon as the influx of power stops, the information stored in RAM chips is lost. The memory controller fulfills this need by "refreshing" the RAM at a constant rate while the computer is powered on.

## RAM types

- DRAM is memory which contains capacitors.
  - capacitors are small buckets which store electricity.
  - capacitors have to be refreshed with electricity constantly.
  - operates async with the system clock.
- SDRAM: synchronous Dynamic RAM
  - have also capacitors like DRAM
  - but difference is speed, it operates in sync with the system clock
- DIMM supercedes SIMM (which have a 32 bits at a time) and have 64 bit data path
- pc-100: the speed at which it operates (100Mhz * 8) = 800MB/s
- RIMM: Rambus Inline Memory Module
  - has 184 pins
  - looks similar to DIMMs
  - bottom notches are near the center of the module
  - was a breakthrough in the 90s, but quickly fall behind DIMM.
- DDR: Double Date Rate
  - DDR: sends the double the amount of data in each clock signal compared to a non DDR + have 184 pins.
  - DDR2 allows for higher bus speeds, uses less power, and has more pins (240).
  - DDR3 is twice as fast as DDR2, also have 240 pins, bottom nutches are in different places
  - DDR4 uses less power than previous generation, have 288 pins, higher range of speed than DDR3
- NVDIMM: A non-volatile dual in-line memory module (NVDIMM)
  - is memory that retains its contents even when electrical power is removed, for example from an unexpected power loss, system crash, or normal shutdown.
  - "Dual in-line" identifies the memory as using the DIMM package. NVDIMMs improve application performance and system crash recovery time.
  - They enhance solid-state drive (SSD) endurance and reliability

## IDE vs SATA

- Parallel ATA (PATA) also known as ATA or IDE is an interface standard for connection of storages devices such as Hard Disk Drives (HDD), Solid State Drives (SSD) and CD/DVD drives to the computer.
- IDE does not suppot hot plugging, its max speed is 133MB/s, and the IDE cables are wide and can be up to 18 inches long.
- SATA does support hot plugging, transfer speed goes to 300MB/s, and SATA cable can be narrow and can be up to 39 inches long.
- IDE drives are slower than SATA drives and they have a parallel connection.
- IDE has a single cable for data and power.

## SCSI

- is a set of standards for physically connecting and transferring data between computers and peripheral devices. The SCSI standards define commands, protocols, electrical, optical and logical interfaces.
- SCSI drives will be more **reliable** simple because they are built better.
- They are designed and priced to be put into **high end machines** - primarily servers where they will get a lot of abuse and quite possibly run 24/7/365 for 5-7 years.
- They are made of higher quality parts, etc. SATA drives are **consumer grade** drives. They are made as cheaply as possible and are not designed with the kind of workloads Servers typically do in mind.
- As far as speed, you will actually see a fairly big difference in speed just do to the spindle speeds of the SCSI devices being so much higher. You're talking 7200 RPM vs 15k RPM on the high ends of both devices. The faster spindle speed, gives you lower latency and higher access speeds.

## Device Registers

- **Device registers** are small storage locations inside a hardware device that software reads and writes to control the device and learn its state. They are the device's control panel: the driver never reaches into the device's internals, it only touches the registers.
- Registers usually fall into a few kinds:
  - **Control registers:** the driver writes here to tell the device what to do (for example "enable yourself", "start transmitting", "reset").
  - **Status registers:** the driver reads here to find out what is going on (for example "I'm busy", "data is ready", "an error occurred").
  - **Data registers:** used to pass actual data in or out, such as a byte to send over a serial port.
  - **Configuration registers:** describe or set up the device (for example its type, its capabilities, a network card's MAC address).
- Unlike CPU registers, which live inside the processor, device registers live in the device and are accessed through an address.
  - **Memory-mapped I/O (MMIO):** the registers are assigned addresses in the same address space as ordinary memory. Writing to address `0x10001000` doesn't store anything in RAM; it goes to the device.
  - **Port I/O:** an older x86 mechanism in which registers sit in a separate address space and are accessed with special CPU instructions (`in` and `out`).

**An example.** A simple serial port might have:

| Address offset | Register | Meaning                         |
| -------------- | -------- | ------------------------------- |
| 0x00           | Data     | Write a byte here to send it    |
| 0x04           | Status   | Bit 0 = 1 means "ready to send" |
| 0x08           | Control  | Write 1 to enable the port      |

To send a character, the driver writes 1 to Control, reads Status until the ready bit is set, then writes the character to Data.

**In virtio.** A virtio device exposes registers such as the device ID (is this a network card or a disk?), feature bits (what the device supports), a status register (how far the driver has got in initializing it), and a "notify" register the driver writes to in order to say "I've queued new work for you." With the MMIO transport these sit at the predefined memory address the passage mentions; with PCI, the guest finds their location by scanning the bus.

For a purely virtual device there is no actual hardware behind those addresses. When the guest reads or writes one, the hypervisor intercepts the access and responds in software as a real device would.

## PCI Bus Discovery

- **The guest firmware** (for example SeaBIOS or OVMF/UEFI) runs first at boot. It scans the PCI bus, assigns address ranges to each device, and finds a device to boot from.
- **The guest kernel's PCI subsystem** then does its own scan when the OS starts.
- Every PCI device has a standard block of registers called the **configuration space**, and the first fields in it are the **Vendor ID** and **Device ID**. The kernel walks through every possible slot (bus number, device number, function number) and reads the Vendor ID at each one:
  - If it reads back `0xFFFF`, the slot is empty and the kernel moves on.
  - Otherwise there is a device, and the kernel reads the Device ID and the rest of the configuration.
- In a VM there is no real PCI bus. When the guest reads a configuration register, the **hypervisor** intercepts the access and returns the values a real device would. 
- Virtio devices use vendor ID `0x1AF4` (registered to Red Hat) and device IDs in the range `0x1000`–`0x107F`. Each kernel driver declares the IDs it supports, and the PCI subsystem matches what it found against those lists. For virtio this happens in two steps:
  1. The generic `virtio-pci` driver claims any device with the virtio vendor ID.
  2. That driver then works out which kind of virtio device it is (network, block, console, and so on), and the matching specific driver, such as `virtio-net` or `virtio-blk`, takes over.
