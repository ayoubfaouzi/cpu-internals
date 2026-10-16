# Chapter 3 Basic Execution Environment

#### Modes of Operation

- **Protected mode**:
  - native state of the processor.
  - includes (virtual-8086) feature to execute `real-mode` 8086 software in protected, multi-tasking env.
- **Real-address mode**:

  - programming env for 8086
  - ability to switch to protected more or SMM mode.
  - the processor is placed in real mode after power-up or reset.
  - DOS run in real mode

- **System Management Mode (SMM)**:
  - implements platform-specific functions such as power management and system security.
  - cpu enters SMM modes when SMI is received from the APIC or when the external SMM interrupt pin (SMI##) is activated.
- Intel 64 architecture adds the **IA-32e mode**. IA-32e mode has two sub-modes:
  - **Compatibility mode**: permits most legacy 16-bit and 32-bit apps to run without re-compilation under a 64-bit OS.
  - **Long mode**: This mode enables a 64-bit OS to run apps written to access 64-bit linear address space.
  - In 64-bits mode: GPR and SIMD registers extends from 8 to 16 + GPR are widened to 64 bits.

#### Overview of the Basic Execution Environment

- Any program or task running on an IA-32 processor is given a set of resources for executing instructions and for storing code, data, and state information.

<p align="center"><img src="https://i.imgur.com/i9Djo8N.png" width="500px" height="auto"></p>

<p align="center"> <img src="https://i.imgur.com/5V6dS2F.png" width="500px" height="auto"></p>

#### Memory Organization

##### IA-32 Memory Models

- programs do not directly address physical memory, istead, they access memory using one of three memory models: `flat`, `segmented`, or `real` address mode.
- **Flat memory model**:
  - memory appears to a program as a single, continuous address space.
  - `Code`, `data`, and `stacks` are all contained in this address space (from 0 to 2^32 - 1 in non x64 mode).
- **Segmented memory mode**:
  - memory appears to a program as a group of independent address spaces called _segments_.
  - code, data, and stacks are typically contained in separate segments.
  - to address a byte in a segment, a program issues a **logical address** (far pointer) = segment selector + offset = **linear address**.
  - programs running on an IA-32 processor can address up to 16,383 segments (2^14 - 1).
  - internally, all the segments that are defined for a system are mapped into the processor’s linear address space.
  - to access a memory location, the processor translates each logical address into a linear address.
  - this translation is transparent to the application program.
  - the primary reason for using segmented memory is to **increase the reliability of programs and systems**.
  - for example, placing a program’s stack in a separate segment prevents the stack from growing into the code or data space and overwriting instructions or data, respectively.
- **Real-address mode memory mode**:
  - memory model for the 8086 processor.
  - linear address space for the program and the os/executive consists of an array of segments of up to 64 KBytes in size each.
  - maximum size of the linear address space in real-address mode is 220 bytes.

 <p align="center"> <img src="https://i.imgur.com/EfpFmVB.png" width="500px" height="auto"></p>

##### Paging and Virtual Memory

- when paging is disabled, each linear address has a one-to-one correspondence with a physical address.
- when paging is enabled, linear address space is divided into pages which are mapped to virtual memory. The pages of virtual memory are then mapped as needed into physical memory.
- paging is completely transparent to an app, all that the application sees is linear address space.
- In addition, IA-32 architecture’s paging mechanism includes extensions that support:
  - `Physical Address Extensions (PAE)` to address physical address space greater than 4 GB.
  - `Page Size Extensions (PSE)` to map linear address to physical address in 4-MB pages.

##### Modes of Operation vs. Memory Model

- relationship between operating modes and memory models is as follows:
  - **Protected mode**: the processor can use any of the memory models.
  - **Real-address mode**: the processor can only supports the real-address mode memory model.
  - **SMM mode**:
    - the processor switches to a separate address space (the _system management RAM (SMRAM)_.
    - the memory model used to address bytes in this address space is similar to the real-address mode model.
- **Compatibility mode**:
  - software should observe the same memory model as those targeted to run in 32-bit protected mode.
  - the effect of segmentation is the same as it is in - 32-bit protected mode semantics.
- **64-bit mode**
  - segmentation is generally (but not completely) disabled, creating a flat 64-bit linear-address space.
  - the processor treats the segment base of CS, DS, ES, and SS as zero in 64-bit mode (this makes a linear address equal an effective address).
  - Segmented and real address modes are not available in 64-bit mode.

#### Basic Program Execution Registers

- **General-purpose registers**: eight registers are available for storing operands and pointers.
- **Segment registers**: hold up to six segment selectors.
- **EFLAGS** (program status and control) register: report on the status of the program being executed and allows limited (application-program level) control of the processor.
- **EIP** (instruction pointer) register: contains a 32-bit pointer to the next instruction to be executed.

##### General Purpose Registers

- EAX — Accumulator for operands and results data
- EBX — Pointer to data in the DS segment
- ECX — Counter for string and loop operations
- EDX — I/O pointer
- ESI — Pointer to data in the segment pointed to by the DS register; source pointer for string operations
- EDI — Pointer to data (or destination) in the segment pointed to by the ES register; destination pointer for string operations
- ESP — Stack pointer (in the SS segment)
- EBP — Pointer to data on the stack (in the SS segment)

 <p align="center"> <img src="https://i.imgur.com/cvFWSF9.png" width="500px" height="auto"></p>

##### General-Purpose Registers in 64-Bit Mode

- 16 general purpose registers and the default operand size is 32 bits.
- If a 32-bit operand size is specified: EAX,EBX, ECX, EDX, EDI, ESI, EBP, ESP, R8D - R15D are available.
- If a 64-bit operand size is specified: RAX, RBX, RCX, RDX, RDI, RSI, RBP, RSP, R8-R15 are available.
- R8D-R15D/R8-R15 represent eight new general-purpose registers.
- All of these registers can be accessed at the byte, word, dword, and qword level.
- REX prefixes are used to generate 64-bit operand sizes or to reference registers R8-R15.

#### Segment Registers

- The segment registers (CS, DS, SS, ES, FS, and GS) hold 16-bit segment selectors.
- A segment selector is a special pointer that identifies a segment in memory.
- To access a particular segment in memory, the segment selector for that segment must be present in the appropriate segment register.
- When using the flat (unsegmented) memory model, segment registers are loaded with segment selectors that point to overlapping segments, each of which begins at address 0 of the linear address space. These overlapping segments then comprise the linear address space for the program.
- Typically, two overlapping segments are defined: one for code and another for data and stacks.
- The CS segment register points to the code segment and all the other segment registers point to the data and stack segment.

 <p align="center"> <img src="https://i.imgur.com/AbhbgWY.png" width="500px" height="auto"></p>

- When using the segmented memory model, each segment register is ordinarily loaded with a different segment selector so that each segment register points to a different segment within the linear address space.
- At any time, a program can thus access up to six segments in the linear address space. To access a segment not pointed to by one of the segment registers, a program must first load the segment selector for the segment to be accessed into a segment register.

 <p align="center"> <img src="https://i.imgur.com/fu7EMkz.png" width="500px" height="auto"></p>

- Each of the segment registers is associated with one of three types of storage: code, data, or stack.
- The CS register contains the segment selector for the code segment where the instructions being executed are located.
  - The EIP register contains the offset within the code segment of the next instruction to be executed.
  - The CS register cannot be loaded explicitly by an application program => `EXCEPTION_ILLEGAL_INSTRUCTION`.
  - Instead, it is loaded implicitly by instructions or internal processor operations that change program control (such as procedure calls, interrupt handling, or task switching)
- The DS, ES, FS, and GS registers point to four **data** segments.
- The availability of four data segments permits efficient and secure access to different types of data structures.
- The SS register contains the segment selector for the stack segment.
- If you dont precise the segment, the compiler will use explicitely:
  - CS for code access, DS for data access and SS for stack access.
  - Stack operations such as push and pop, as well as memory references using the stack pointer %esp or base pointer register %ebp, use the stack segment %ss.
  - Other memory references use the data segment %ds.
  - String operations additionally use the extra segment %es.
  - Windows make use of FS to store the Win32 Thread Information Block (TIB) data structure.

 <p align="center"> <img src="https://i.imgur.com/cixb0xe.png" width="600px" height="auto"></p>

##### Segment Registers in 64-Bit Mode

- In 64-bit mode: CS, DS, ES, SS are treated as if each segment base is 0, regardless of the value of the associated segment descriptor base.
- This creates a flat address space for code, data, and stack. FS and GS are exceptions.
- Limit checks for CS, DS, ES, SS, FS, and GS are disabled in 64-bit mode.
