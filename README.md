# CPU Internals
> These notes are taken from Intel SDM. You can consider them as a short/resumed version of some parts of the manuals that I found worth looking at when learning about system programming, OS internals or virtualization.

## Contents

- [Volume 1 Basic Architecture](#volume-1-basic-architecture)
    - [Chapter 3 Basic Execution Environment](#chapter-3-basic-execution-environment)
    - [Chapter 6 Procedure Calls, Interrupts, and Exceptions](#chapter-6-procedure-calls-interrupts-and-exceptions)
- [Volume 2 Instruction Set Reference](#volume-2-instruction-set-reference)
    - [Chapter 6 Safer Mode Extensions Reference](#chapter-6-safer-mode-extensions-reference)
- [Volume 3 System Programming Guide](#volume-3-system-programming-guide)
    - [Chapter 2 System Architecture Overview](#chapter-2-system-architecture-overview)
    - [Chapter 3 Protected Mode Memory Management](#chapter-3-protected-memory-mode-management)
    - [Chapter 4 Paging](#chapter-4-paging)
    - [Chapter 6 Interrupt and Exception Handling](#chapter-6-interrupt-and-exception-handling)
    - [Chapter 11 Memory Cache Control](#chapter-11-memory-cache-control)
    - [Chapter 17 Debug, Branch Profile, TSC, and Intel® Resource Director Technology (Intel® RDT) Features](#Debug)
    - [Chapter 22 Architecture Compatibility](#chapter-22-architecture-compatibility)
    - [Chapter 23 Introduction To Virtual Machine Extensions](#chapter-23-introduction-to-virtual-machine-extensions)
    - [Chapter 24 Virtual Machine Control Structures](#chapter-24-virtual-machine-control-structures)
    - [Chapter 25 VMX Non-Root Operation](#chapter-25-vmx-non-root-operation)
    - [Chapter 28 VMX Support For Address Translation](#chapter-28-vmx-support-for-address-translation)
    - [Chapter 30 VMX Instruction Reference](#chapter-30-vmx-instruction-reference)

## Volume 1 Basic Architecture

### Chapter 3 Basic Execution Environment

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
- **Segment registers**: hold upp to six segment selectors.
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

### Chapter 6 Procedure Calls, Interrupts, and Exceptions

#### Stacks

- A stack can be up to 4 GBytes long, the maximum size of a segment.
- The stack **grows down** in memory (towards lesser addresses) when items are pushed on the stack and **shrinks up** (towards greater addresses) when the items are popped from the stack.
- When a system sets up many stacks, only one stack—the current stack—is available at a time. The current stack is the one contained in the segment referenced by the SS register.
- The processor references the SS register automatically for all stack operations. For example, when the ESP register is used as a memory address, it automatically points to an address in the current stack. Also, the CALL, RET, PUSH, POP, ENTER, and LEAVE instructions all perform operations on the current stack.
<p align="center"><img src="https://i.imgur.com/bcoXzvx.png" width="500px" height="auto"></p>

##### Stack-Frame Base Pointer

- The stack is typically divided into frames. Each stack frame can then contain local variables, parameters to be passed to another procedure, and procedure linking information.
- The stack-frame base pointer (contained in the EBP register) identifies a fixed reference point within the stack frame for the called procedure. To use the stackframe base pointer, the called procedure typically copies the contents of the ESP register into the EBP register prior to pushing any local variables on the stack.
- The stack

##### Return Instruction Pointer

- Prior to branching to the first instruction of the called procedure, the CALL instruction pushes the address in the EIP register onto the current stack.
- This address is then called the return-instruction pointer and it points to the instruction where execution of the calling procedure should resume following a return from the called procedure.
- Upon returning from a called procedure, the RET instruction pops the return-instruction pointer from the stack back into the EIP register. Execution of the calling procedure then resumes.

##### Stack Behavior in 64-Bit Mode

- Address calculations that reference SS segments are treated as if the segment base is zero.
- Fields (base, limit, and attribute) in segment descriptor registers are ignored. 
- SS DPL is modified such that it is always equal to CPL
- Some forms of segment load instructions are invalid (for example, LDS, POP ES).

#### Calling Procedures Using CALL and RET

- The CALL instruction allows control transfers to procedures within the current code segment (**near call**) and in a different code segment (**far call**).
- Near calls usually provide access to local procedures within the currently running program or task.
- Far calls are usually used to access operating system procedures or procedures in a different task.
- The RET instruction also allows near and far returns to match the near and far versions of the CALL instruction.

##### Far CALL and RET Operation

When executing a far call, the processor performs these actions:
1. Pushes the current value of the CS register on the stack.
2. Pushes the current value of the EIP register on the stack.
3. Loads the segment selector of the segment that contains the called procedure in the CS register.
4. Loads the offset of the called procedure in the EIP register.
5. Begins execution of the called procedure.

When executing a far return, the processor does the following:
1. Pops the top-of-stack value (the return instruction pointer) into the EIP register.
2. Pops the top-of-stack value (the segment selector for the code segment being returned to) into the CS register.
3. If the RET instruction has an optional n argument, increments the stack pointer by the number of bytes
specified with the n operand to release parameters from the stack.
4. Resumes execution of the calling procedure.

<p align="center"> <img src="https://i.imgur.com/Nn0SFOy.png" width="500px" height="auto"></p>

##### Parameter Passing

- Through the **General-Purpose Registers**: The processor does not save the state of the general-purpose registers on procedure calls.
- Through the **Stack**: To pass a large number of parameters to the called procedure, the parameters can be placed on the stack.
- Through the **Argument List**:  an alternate method of passing a larger number of parameters (or a data structure) to the called procedure is to place the parameters in an argument list in one of the data segments in memory. A pointer to the argument list can then be passed to the called procedure through a general-purpose register or the stack.

##### Calls to Other Privilege Levels

- The IA-32 architecture’s protection mechanism recognizes four privilege levels, numbered from 0 to 3, where a greater number mean less privilege. 
- The reason to use privilege levels is to improve the reliability of operating systems.

<p align="center"> <img src="https://i.imgur.com/RTkdO9L.png" width="500px" height="auto"></p>

- Code modules in lower privilege segments can only access modules operating at higher privilege segments by means of a tightly controlled and protected interface called a **gate.** 
- Attempts to access higher privilege segments without going through a protection gate and without having sufficient access rights causes a general-protection exception (#GP) to be generated.
- If an operating system or executive uses this multilevel protection mechanism, a call to a procedure that is in a
more privileged protection level than the calling procedure is handled in a similar manner as a far call. The differences are as follows:
  - The segment selector provided in the CALL instruction references a special data structure called a **call gate descriptor**. Among other things, the call gate descriptor provides the following:
    - access rights information
    - the segment selector for the code segment of the called procedure
    - an offset into the code segment (that is, the instruction pointer for the called procedure).
  - The processor switches to a new stack to execute the called procedure. Each privilege level has its own stack. The segment selector and stack pointer for the privilege level 3 stack are stored in the SS and ESP registers, respectively, and are automatically saved when a call to a more privileged level occurs. The segment selectors and stack pointers for the privilege level 2, 1, and 0 stacks are stored in a system segment called the task state segment (TSS).

##### CALL and RET Operation Between Privilege Levels

When making a call to a more privileged protection level, the processor does the following (see Figure 6-4):
1. Performs an access rights check (privilege check).
2. Temporarily saves (internally) the current contents of the SS, ESP, CS, and EIP registers.
3. Loads the segment selector and stack pointer for the new stack (that is, the stack for the privilege level being called) from the TSS into the SS and ESP registers and switches to the new stack.
4. Pushes the temporarily saved SS and ESP values for the calling procedure’s stack onto the new stack.
5. Copies the parameters from the calling procedure’s stack to the new stack. A value in the call gate descriptor determines how many parameters to copy to the new stack.
6. Pushes the temporarily saved CS and EIP values for the calling procedure to the new stack.
7. Loads the segment selector for the new code segment and the new instruction pointer from the call gate into the CS and EIP registers, respectively.
8. Begins execution of the called procedure at the new privilege level.

When executing a return from the privileged procedure, the processor performs these actions:
1. Performs a privilege check.
2. Restores the CS and EIP registers to their values prior to the call.
3. If the RET instruction has an optional n argument, increments the stack pointer by the number of bytes specified with the n operand to release parameters from the stack. If the call gate descriptor specifies that one or more parameters be copied from one stack to the other, a RET n instruction must be used to release the parameters from both stacks. Here, the n operand specifies the number of bytes occupied on each stack by the parameters. On a return, the processor increments ESP by n for each stack to step over (effectively remove) these parameters from the stacks.
4. Restores the SS and ESP registers to their values prior to the call, which causes a switch back to the stack of
the calling procedure.
5. If the RET instruction has an optional n argument, increments the stack pointer by the number of bytes specified with the n operand to release parameters from the stack.
6. Resumes execution of the calling procedure.

<p align="center"> <img src="https://i.imgur.com/ZNrnZQd.png" width="500px" height="auto"></p>

## Volume 2 Instruction Set Reference

### Chapter 6 Safer Mode Extensions Reference

#### Overview

- Provide a programming interface for system software to establish a measured environment within the platform to support trust decisions by end users.
- The measurement and protection mechanisms used by a measured environment are supported by the capabilities of an Intel TXT.

#### SMX functionality

- SMX functionality is provided in an Intel 64 processor through the **GETSEC** instruction via leaf functions.
- Software can detect support for SMX operation using the ```CPUID.01H.ECX[Bit 6] == 1```


## Volume 3 System Programming Guide

### Chapter 2 System Architecture Overview

#### Overview of the System-Level Architecture

- System-level architecture consists of a set of registers, data structures, and instructions designed to support basic system-level operations such as memory management, interrupt and exception handling, task management, and control of multiple processors

<p align="center"> <img src="https://i.imgur.com/O0QCFwt.png" width="700px" height="auto"></p>
<p align="center"> <img src="https://i.imgur.com/gCayvoh.png" width="700px" height="auto"></p>

##### Global and Local Descriptor Tables

- When operating in protected mode, all memory accesses pass through either the global descriptor table (GDT) or an optional local descriptor table (LDT).
- These tables contain entries called segment descriptors and can contain up to 8192 (2^13) 8-byte descriptors.
- Segment descriptors (8 byte large) provide the base address of segments as well as access rights, type, and usage information.
- Each segment descriptor has an associated segment selector.
- A segment selector (16-bits) provides the software that uses it with an index into the GDT or LDT (the offset of its associated segment descriptor), a global/local flag (determines whether the selector points to the GDT or the LDT), and access rights information (CPL).

###### Global and Local Descriptor Tables in IA-32e Mode

- GDTR and LDTR registers are expanded to 64-bits wide in both IA-32e sub-modes.
- GDT/LDT are expanded in 64-bit mode to support 64-bit base addresses, (16-byte LDT descriptors hold a 64-bit base address and various attributes).

##### System Segments, Segment Descriptors, and Gates

- The architecture defines two system segments: the task-state segment (TSS) and the LDT.
- The architecture defines a set of special descriptors called gates: `call gates`, `interrupt gates`, `trap gates`, and `task gates`.
- These gates provide protected gateways to system procedures and handlers that may operate at a different privilege level than application programs and most procedures.
- For example, a CALL to a call gate can provide access to a procedure in a code segment that is at the same or a numerically lower privilege level (more privileged) than the current code segment.

###### Gates in IA-32e Mode

- In IA-32e mode, the following descriptors are 16-byte descriptors (expanded to allow a 64-bit base): LDT descriptors, 64-bit TSSs, call gates, interrupt gates, and trap gates.
- Call gates facilitate transitions between 64-bit mode and compatibility mode. Task gates are not supported in IA-32e mode.
- On privilege level changes, stack segment selectors are not read from the TSS. Instead, they are set to
  NULL.

##### Task-State Segments and Task Gates

- The `TSS` defines the state of the execution environment for a task. It includes the state of:
  - GPR, segment registers, the EFLAGS register, the EIP register.
  - segment selectors with stack pointers for three stack segments (one stack for each privilege level).
  - segment selector for the LDT associated with the task and the base address of the paging-structure hierarchy.
- All program execution in protected mode happens within the context of a task called the `current task`.
- In switching tasks, the processor performs the following actions:
  1. Stores the state of the current task in the current TSS.
  2. Loads the task register with the segment selector for the new task.
  3. Accesses the new TSS through a segment descriptor in the GDT.
  4. Loads the state of the new task from the new TSS into the general-purpose registers, the segment registers, the LDTR, control register CR3, the EFLAGS register, and the EIP register.
  5. Begins execution of the new task.
- A task can also be accessed through a task gate. A task gate is similar to a call gate, except that it provides access (through a segment selector) to a TSS rather than a code segment

##### Task-State Segments in IA-32e Mode

- Hardware task switches are not supported in IA-32e mode. However, TSSs continue to exist.
- The base address of a TSS is specified by its descriptor.
- A 64-bit TSS holds the following information that is important to 64-bit operation:
  - Stack pointer addresses for each privilege level
  - Pointer addresses for the interrupt stack table
  - Offset address of the IO-permission bitmap (from the TSS base)
- The task register is expanded to hold 64-bit base addresses in IA-32e mode.

##### Interrupt and Exception Handling

- External interrupts, software interrupts and exceptions are handled through the interrupt descriptor table (IDT).
- The IDT stores a collection of _gate descriptors_ that provide access to interrupt and exception handlers
- The linear address for the base of the IDT is contained in the IDT register (IDTR)
- Gate descriptors in the IDT can be interrupt, trap, or task gate descriptors.
- To access an interrupt or exception handler, the processor first receives an interrupt vector from internal hardware, an external interrupt controller, or from software by means of an INT n, INTO, INT3, INT1, or BOUND instruction.

##### Memory Management

- The base physical address of the paging-structure hierarchy is contained in control register CR3.
- The entries in the paging structures determine the physical address of the base of a page frame, access rights and memory management information.

##### Memory Management in IA-32e Mode

- Four levels of system data structures are used:
  - **The page map level 4 (PML4)** — An entry in a PML4 table contains the physical address of the base of a page directory pointer table, access rights, and memory management information. The base physical address of the PML4 is stored in CR3.
  - **A set of page directory pointer tables** — An entry in a page directory pointer table contains the physical
    address of the base of a page directory table, access rights, and memory management information.
  - **Sets of page directories** — An entry in a page directory table contains the physical address of the base of a page table, access rights, and memory management information.
  - **Sets of page tables** — An entry in a page table contains the physical address of a page frame, access rights, and memory management information.

##### System Registers

- The `system flags` and `IOPL` field in the EFLAGS register control task and mode switching, interrupt handling, instruction tracing, and access rights.
- The `control registers CR0, CR2, CR3, and CR4` contain a variety of flags and data fields for controlling system level operations.
- The `debug registers` allow the setting of breakpoints for use in debugging programs and systems software.
- The `GDTR, LDTR`, and `IDTR` registers contain the linear addresses and sizes (limits) of their respective tables.
- The `TR task register` contains the linear address and size of the TSS for the current task.
- Model-specific registers: group of registers available primarily to OS or executive procedures (that is code running at privilege level 0). These registers control items such as the debug extensions, the performance-monitoring counters, the machine- check architecture, and the memory type ranges (MTRRs)

##### System Registers in IA-32e Mode

- system-descriptor-table registers (GDTR, IDTR, LDTR, and TR) are expanded in hardware to hold 64-bit base addresses.
- EFLAGS becomes the 64-bit RFLAGS register.
- CR0–CR4 are expanded to 64 bits.
- CR8 becomes available. CR8 provides read-write access to the task priority register (TPR) so that the OS can control the priority classes of external interrupts.
- In 64-bit mode, debug registers DR0–DR7 are 64 bits. In compatibility mode, address-matching in DR0–DR3 is
  also done at 64-bit granularity.
- On systems that support IA-32e mode, the extended feature enable register (IA32_EFER) is available. This MSR controls activation of IA-32e mode and other IA-32e mode operations.
- In addition, there are several model-specific registers that govern IA-32e mode instructions:
  - **IA32_KERNEL_GS_BASE** — Used by SWAPGS instruction.
  - **IA32_LSTAR** — Used by SYSCALL instruction.
  - **IA32_FMASK** — Used by SYSCALL instruction.
  - **IA32_STAR** — Used by SYSCALL and SYSRET instruction

#### MODES OF OPERATION

<p align="center"> <img src="https://i.imgur.com/bbcrQlk.png" width="600px" height="auto"></p>

- The processor is placed in real-address mode following power-up or a reset.
- The PE flag in control register CR0 then controls whether the processor is operating in real-address or protected mode.
- The VM flag in the EFLAGS register determines whether the processor is operating in protected mode or virtual-8086 mode.
- The LMA bit (IA32_EFER.LMA[bit 10]) determines whether the processor is operating in IA-32e mode.
- When running in IA-32e mode, 64-bit or compatibility sub-mode operation is determined by CS.L bit of the code segment.
- The processor switches to SMM whenever it receives an SMI while the processor is in real-address, protected, virtual-8086, or IA-32e modes. Upon execution of the RSM instruction, the processor always returns to the mode it was in when the SMI occurred.

#### Extended Feature Enable Register

<p align="center"> <img src="https://i.imgur.com/FepFdCU.png" width="600px" height="auto"></p>

#### SYSTEM FLAGS AND FIELDS IN THE EFLAGS REGISTER

- The system flags and IOPL field of the EFLAGS register control I/O, maskable hardware interrupts, debugging, task switching, and the virtual-8086 mode.
- Only privileged code (typically operating system or executive code) should be allowed to modify these bits.

<p align="center"> <img src="https://i.imgur.com/1YFZsgw.png" width="600px" height="auto"></p>

- TF: Set to enable single-step mode for debugging; clear to disable single-step mode.
- IF: Set to respond to maskable hardware interrupt requests; cleared to inhibit maskable hardware interrupts.
- IOPL: Indicates the I/O privilege level of the currently running program or task. The CPL of the currently running program or task must be less than or equal to the IOPL to access the I/O address space.
- NT: Controls the chaining of interrupted and called tasks.
- RF: Set to temporarily disables debug exceptions (##DB) from being generated for instruction breakpoints; cleared to generate debug exceptions.
- VM: Set to enable virtual-8086 mode; clear to return to protected mode.
- AC: An alignment-check exception is generated when reference is made to an unaligned operand, such as a word at an odd byte address or a doubleword at an address which is not an integral multiple of four.
- VIP: Set by software to indicate that an interrupt is pending; cleared to indicate that no interrupt is pending.
- VIF: Contains a virtual image of the IF flag.
- ID: The ability of a program or procedure to set or clear this flag indicates support for the CPUID instruction.

#### System Flags and Fields in IA-32e Mode

- In 64-bit mode, the RFLAGS register expands to 64 bits with the upper 32 bits reserved.
- In IA-32e mode, the processor does not allow the VM bit to be set because virtual-8086 mode is not supported

#### MEMORY-MANAGEMENT REGISTERS

- The processor provides four memory-management registers (GDTR, LDTR, IDTR, and TR) that specify the locations of the data structures which control segmented memory management

<p align="center"> <img src="https://i.imgur.com/yx6piV0.png" width="600px" height="auto"></p>

#### Global Descriptor Table Register (GDTR)

- The GDTR register holds the base address (32 bits in protected mode; 64 bits in IA-32e mode) and the 16-bit table limit for the GDT
- The base address specifies the linear address of byte 0 of the GDT; the table limit specifies the
  number of bytes in the table.
- The `LGDT` and `SGDT` instructions load and store the GDTR register, respectively.

#### Local Descriptor Table Register (LDTR)

- The LDTR register holds the 16-bit segment selector, base address (32 bits in protected mode; 64 bits in IA-32e mode), segment limit, and descriptor attributes for the LDT.
- The base address specifies the linear address of byte 0 of the LDT segment; the segment limit specifies the number of bytes in the segment.
- The `LLDT` and `SLDT` instructions load and store the segment selector part of the LDTR register, respectively.
- The segment that contains the LDT must have a segment descriptor in the GDT.
- LDT is intended to used per process and switched when the kernel switch between process contexts. GDT is for use system wide.
- The OS sets those tables.

#### IDTR Interrupt Descriptor Table Register

- The IDTR register holds the base address (32 bits in protected mode; 64 bits in IA-32e mode) and 16-bit table limit for the IDT.
- The base address specifies the linear address of byte 0 of the IDT; the table limit specifies the number
  of bytes in the table.
- The `LIDT` and `SIDT` instructions load and store the IDTR register, respectively.

#### Task Register (TR)

- The task register holds the 16-bit segment selector, base address (32 bits in protected mode; 64 bits in IA-32e mode), segment limit, and descriptor attributes for the TSS of the current task.
- The selector references the TSS descriptor in the GDT.
- The base address specifies the linear address of byte 0 of the TSS; the segment limit specifies the number of bytes in the TSS.
- The `LTR` and `STR` instructions load and store the segment selector part of the task register, respectively.

### Chapter 3 Protected Mode Memory Management

#### MEMORY MANAGEMENT OVERVIEW

- memory managements facilities of the IA-32 archietcture are divided into two parts:
  - **Segmentation** provides a mechanism of isolating individual code, data, and stack modules so that multiple programs (or tasks) can run on the same processor without interfering with one another.
  - **Paging** provides a mechanism for implementing a conventional demand-paged, virtual-memory system where sections of a program’s execution environment are mapped into physical memory as needed.
- paging can also be used to provide isolation between multiple tasks.
- when operating in protected mode, some form of segmentation must be used.
- there is no mode bit to disable segmentation. The use of paging, however, is optional.
- Segments can be used to hold the code, data, and stack for a program or to hold system data structures (such as a TSS or LDT).

<p align="center"> <img src="https://i.imgur.com/QpA3AS9.png" width="600px" height="auto"></p

- If paging is not used, the linear address space of the processor is mapped directly into the physical address space
  of processor.

#### USING SEGMENTS

- The segmentation mechanism supported by the IA-32 architecture can be used to implement a wide variety of
  system designs.
- These designs range from flat models that make only minimal use of segmentation to protect programs to multi-segmented models that employ segmentation to create a robust operating environment in which multiple programs and tasks can be executed reliably.

#### Basic Flat Model

- the operating system and application programs have access to a continuous, unsegmented address space.
- this basic flat model hides the segmentation mechanism of the architecture from both the system designer and the application programmer.
- To implement a basic flat memory model with the IA-32 architecture, at least two segment descriptors must be
  created, one for referencing a code segment and one for referencing a data segment.
- Both of these segments, however, are mapped to the entire linear address space: that is, both segment descriptors have
  the same base address value of 0 and the same segment limit of 4 GBytes.
- By setting the segment limit to 4 GBytes, the segmentation mechanism is kept from generating exceptions for out of limit memory references, even if no physical memory resides at a particular address.

<p align="center"> <img src="https://i.imgur.com/tQ02vfP.png" width="600px" height="auto"></p

#### Protected Flat Model

- The protected flat model is similar to the basic flat model, except the segment limits are set to include only the
  range of addresses for which physical memory actually exists.
- A general-protection exception (#GP) is then generated on any attempt to access nonexistent memory. This model provides a minimum level of hardware protection against some kinds of program bugs.

<p align="center"> <img src="https://i.imgur.com/dSPFaY8.png" width="600px" height="auto"></p

#### Multi-Segment Model

- A multi-segment model uses the full capabilities of the segmentation mechanism to provide hardware enforced protection of code, data structures, and programs and tasks.
- Here, each program (or task) is given its own table of segment descriptors and its own segments.
- The segments can be completely private to their assigned programs or shared among programs.
- Access to all segments and to the execution environments of individual programs running on the system is controlled by hardware.

<p align="center"> <img src="https://i.imgur.com/X6pRDKv.png" width="600px" height="auto"></p

- Access checks can be used to protect not only against referencing an address outside the limit of a segment, but
  also against performing disallowed operations in certain segments.
- For example, since code segments are designated as read-only segments, hardware can be used to prevent writes into code segments
- The access rights information created for segments can also be used to set up protection rings or levels. Protection levels can be used to protect operating-system procedures from unauthorized access by application programs.

#### Segmentation in IA-32e Mode

- In IA-32e mode of Intel 64 architecture, the effects of segmentation depend on whether the processor is running in compatibility mode or 64-bit mode.
- In compatibility mode, segmentation functions just as it does using legacy 16-bit or 32-bit protected mode semantics.
- In 64-bit mode, segmentation is generally (but not completely) **disabled**, creating a flat 64-bit linear-address
  space.
- The processor treats the segment base of CS, DS, ES, SS as zero, creating a linear address that is equal to the effective address.
- The FS and GS segments are exceptions. These segment registers (which hold the segment base) can be used as additional base registers in linear address calculations. They facilitate addressing local data and certain operating system data structures.

#### Paging and Segmentation

- Paging can be used with any of the segmentation models described above.
- The processor’s paging mechanism divides the linear address space (into which segments are mapped) into pages.
- These linear-address-space pages are then mapped to pages in the physical address space.
- The paging mechanism offers several page-level protection facilities that can be used with or instead of the segment protection facilities.
- For example, it lets read-write protection be enforced on a page-by-page basis. The paging mechanism also provides two-level user-supervisor protection that can also be specified on a page-by-page basis.

#### LOGICAL AND LINEAR ADDRESSES

- At the system-architecture level in protected mode, the processor uses two stages of address translation to arrive at a physical address: logical-address translation through `segmentation` and linear address space through `paging`.
- Segmentation provides a mechanism for dividing adressable memory space into segments.
- Segmentation translates logical addresses to linear addresses in hardware by using table lookups.
- A logical address consists of a 16-bit segment selector and a 32-bit offset.
- A linear address is a 32-bit address in the processor’s linear address space. Like the physical address space, the linear address space is a flat (unsegmented), 2^32-byte address space, with addresses ranging from 0 to FFFFFFFFH.
- To translate a logical address into a linear address, the processor does the following:
  1. Uses the offset in the segment selector to locate the segment descriptor for the segment in the GDT or LDT and reads it into the processor. (This step is needed only when a new segment selector is loaded into a segment register.)
  2. Examines the segment descriptor to check the access rights and range of the segment to insure that the segment is accessible and that the offset is within the limits of the segment.
  3. Adds the base address of the segment from the segment descriptor to the offset to form a linear address.

<p align="center"> <img src="https://i.imgur.com/jh5Bp46.png" width="600px" height="auto"></p>

#### Logical Address Translation in IA-32e Mode

- In 64-bit mode, the offset and base address of the segment are 64-bits instead of 32 bits.
- The linear address format is also 64 bits wide and is subject to the canonical form requirement.
- Each code segment descriptor provides an L bit. This bit allows a code segment to execute 64-bit code or legacy 32-bit code by code segment.

#### Segment Selectors

- A segment selector is a 16-bit identifier for a segment. It does not point directly to the segment, but instead points to the segment descriptor that defines the segment.

<p align="center"> <img src="https://i.imgur.com/JUOiXYX.png" width="600px" height="auto"></p>

- The first entry of the GDT is not used by the processor.

#### Segment Registers

- To reduce address translation time and coding complexity, the processor provides registers for holding up to 6 segment selectors
- Each of these segment registers support a specific kind of memory reference (code, stack, or data).
- For virtually any kind of program execution to take place, at least the code-segment (CS), data-segment (DS), and stack-segment (SS) registers must be loaded with valid segment selectors.
- Although a system can define thousands of segments, only 6 can be available for immediate use
- A segment have a visible part (segment selector) which can be read or written to by software running at any privilege level, and a hidden part which act like a cache so that segment descriptor info (base, limits, access rights) doesn't have to be looked up each time (only accessible by the hardware).

<p align="center"> <img src="https://i.imgur.com/lxlmUtA.png" width="600px" height="auto"></p>

- When a segment selector is loaded into the visible part of a segment register, the processor also loads the hidden part of the segment register with the base address, segment limit, and access control information from the segment descriptor pointed to by the segment selector.
- Two kinds of load instructions are provided for loading the segment registers:
  1. Direct load instructions such as the MOV, POP, LDS, LES, LSS, LGS, and LFS instructions. These instructions explicitly reference the segment registers.
  2. Implied load instructions such as the far pointer versions of the CALL, JMP, and RET instructions, the SYSENTER and SYSEXIT instructions, and the IRET, INTn, INTO and INT3 instructions.

#### Segment Descriptors

- A segment descriptor is a data structure in a GDT or LDT that provides the processor with the size and location of a segment, as well as access control and status information.
- Segment descriptors are typically created by compilers, linkers, loaders, or the OS or executive, but not application programs.

<p align="center"> <img src="https://i.imgur.com/yjnRlez.png" width="600px" height="auto"></p>

#### System Descriptor Types

- When the S (descriptor type) flag in a segment descriptor is clear, the descriptor type is a system descriptor. The processor recognizes the following types of system descriptors:
  - Local descriptor-table (LDT) segment descriptor.
  - Task-state segment (TSS) descriptor.
  - Call-gate descriptor.
  - Interrupt-gate descriptor.
  - Trap-gate descriptor.
  - Task-gate descriptor.

#### Segment Descriptor Tables

- Each system must have one GDT defined, which may be used for all programs and tasks in the system. Optionally, one or more LDTs can be defined. For example, an LDT can be defined for each separate task being run, or some or all tasks can share the same LDT.

<p align="center"> <img src="https://i.imgur.com/qEnuApv.png" width="600px" height="auto"></p>

- The base address of the GDT should be aligned on an eight-byte boundary to yield the best processor performance.
- The limit value for the GDT is expressed in bytes.
- As with segments, the limit value is added to the base address to get
  the address of the last valid byte. A limit value of 0 results in exactly one valid byte. Because segment descriptors
  are always 8 bytes long, the GDT limit should always be one less than an integral multiple of eight (that is, 8N – 1).
- The first descriptor in the GDT is not used by the processor. A segment selector to this `null descriptor` does not generate an exception when loaded into a data-segment register (DS, ES, FS, or GS), but it always generates a #GP exception when an attempt is made to access memory using the descriptor.
- The LDT is located in a system segment of the LDT type. The GDT must contain a segment descriptor for the LDT segment. If the system supports multiple LDTs, each must have a separate segment selector and segment descriptor in the GDT. The segment descriptor for an LDT can be located anywhere in the GDT.
- An LDT is accessed with its segment selector. To eliminate address translations when accessing the LDT, the segment selector, base linear address, limit, and access rights of the LDT are stored in the LDTR register

### Chapter 4 Paging

- Paging translates each linear address to a physical address and determines, for each translation, what
  accesses to the linear address are allowed (the address’s access rights) and the type of caching used for such
  accesses (the address’s memory type)

#### PAGING MODES AND CONTROL BITS

- Paging behavior is controlled by the following control bits:
  - The WP and PG flags in control register CR0 (bit 16 and bit 31, respectively).
  - The PSE, PAE, PGE, PCIDE, SMEP, SMAP, and PKE flags in control register CR4 (bit 4, bit 5, bit 7, bit 17, bit 20,
    bit 21, and bit 22, respectively).
  - The LME and NXE flags in the IA32_EFER MSR (bit 8 and bit 11, respectively).
  - The AC flag in the EFLAGS register (bit 18).
- Software enables paging by using the MOV to CR0 instruction to set CR0.PG.

#### Three Paging Modes

- If `CR0.PG = 0`, paging is not used. The logical processor treats all linear addresses as if they were physical addresses. CR4.PAE and IA32_EFER.LME are ignored by the processor, as are CR0.WP, CR4.PSE, CR4.PGE, CR4.SMEP, CR4.SMAP, and IA32_EFER.NXE.
- Paging is enabled if `CR0.PG = 1`. Paging can be enabled only if protection is enabled (`CR0.PE = 1`).
- If paging is enabled, one of three paging modes is used. The values of CR4.PAE and IA32_EFER.LME determine which paging
  mode is used:
  - If CR0.PG = 1 and CR4.PAE = 0, **32-bit paging** is used. 32-bit paging. 32-bit paging uses CR0.WP, CR4.PSE, CR4.PGE, CR4.SMEP, and CR4.SMAP.
  - If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 0, **PAE paging** is used. PAE paging uses CR0.WP, CR4.PGE, CR4.SMEP, CR4.SMAP, and IA32_EFER.NXE.
  - If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 1, **4-level paging** is used. 4-level paging uses CR0.WP, CR4.PGE, CR4.PCIDE, CR4.SMEP, CR4.SMAP, CR4.PKE, and IA32_EFER.NXE. 4-level paging is available only on processors that support the Intel 64 architecture.

<p align="center"> <img src="https://i.imgur.com/zXoVCCw.png" width="600px" height="auto"></p>

#### Paging-Mode Enabling

<p align="center"> <img src="https://i.imgur.com/QUBkDw5.png" width="600px" height="auto"></p>

#### Paging-Mode Modifiers

- Details of how each paging mode operates are determined by the following control bits:
  - The **WP** flag in CR0 allows pages to be protected from supervisor-mode writes.
  - The **PSE** enables 4-MByte pages for 32-bit paging.
  - The **PGE** enables global pages, which allows translations may be shared across address spaces
  - The **PCIDE** enables process-context identifiers (PCIDs) for 4-level paging; PCIDs allow a logical processor to cache information for multiple linear-address spaces.
  - The **SMEP** allows pages to be protected from supervisor-mode instruction fetches; software
    operating in supervisor mode cannot fetch instructions from linear addresses that are accessible in user mode.
  - The **SMAP** allows pages to be protected from supervisor-mode data accesses; software operating
    in supervisor mode cannot access data at linear addresses that are accessible in user mode.
  - The **PKE** allows each linear address to be associated with a protection key.
  - The **NXE** enables execute-disable access rights for PAE paging and 4-level paging. If IA32_EFER.NXE = 1,
    instruction fetches can be prevented from specified linear addresses

#### Hierarchical Paging Structures: An Overview

- Every paging structure is **4096** Bytes in size and comprises a number of individual **entries**.
- The processor uses the upper portion of a linear address to identify a series of paging-structure entries.
- The last of these entries identifies the physical address of the region to which the linear address translates (called **the page frame**).
- The lower portion of the linear address (called the **page offset**) identifies the specific address within that region to which the linear address translates.
- The first paging structure used for any translation is located at the physical address in **CR3**.

- **32-bit paging**:
  - each paging structure comprises 1024 = 2^10 entries, each entry is 32 bits (4 bytes).
  - For this reason, the translation process uses 10 bits at a time from a 32-bit linear address.
  - Bits 31:22 identify the first paging-structure entry and bits 21:12 identify a second.
  - The latter identifies the page frame. Bits 11:0 of the linear address are the page offset within the 4-KByte page frame.
- **PAE paging**:
  - the first paging structure comprises only 4 = 2^2 entries, each entry is 64 bits (8 bytes).
  - Translation thus begins by using bits 31:30 from a 32-bit linear address to identify the first paging-structure entry.
  - Other paging structures comprise 512 =2^9 entries, so the process continues by using 9 bits at a time.
  - Bits 29:21 identify a second paging-structure entry and bits 20:12 identify a third.
  - This last identifies the page frame.
- **4-level paging**:
  - each paging structure comprises 512 = 2^9 entries (each entry is 64 bits (8 bytes)) and translation uses 9 bits at a time from a 48-bit linear address.
  - Bits 47:39 identify the first paging-structure entry, bits 38:30 identify a second, bits 29:21 a third, and bits 20:12 identify a fourth.
  - Again, the last identifies the page frame.

<p align="center"> <img src="https://i.imgur.com/xsLNtRp.png" width="600px" height="auto"></p>

#### 32-BIT PAGING

- 32-bit paging may map linear addresses to either 4-KByte pages or 4-MByte pages.
<p align="center"> <img src="https://i.imgur.com/yk5DDik.png" width="500px" height="auto"></p>
<p align="center"> <img src="https://i.imgur.com/graLHn1.png" width="500px" height="auto"></p>

#### PAE PAGING

- With PAE paging, a logical processor maintains a set of **four PDPTE** registers (64-bits), which are loaded from an address in CR3.
- Linear address are translated using 4 hierarchies of in-memory paging structures, each located using one of the **PDPTE registers**. (This is different from the other paging modes, in which there is one hierarchy referenced by CR3.
- Each PDPTE controls access to a 1-GByte region of the linear-address space.
- Bits 31:30 of the linear address select a PDPTE register; this is PDPTE*i*, where _i_ is the value of bits 31:30.
- Because a PDPTE register is identified using bits 31:30 of the linear address, it controls access to a 1-GByte region of the linear-address space.

<p align="center"> <img src="https://i.imgur.com/PHpSltX.png" width="500px" height="auto"></p>
<p align="center"> <img src="https://i.imgur.com/tZxmqVu.png" width="500px" height="auto"></p>
<p align="center"> <img src="https://i.imgur.com/DPSmC3x.png" width="500px" height="auto"></p>

#### 4-LEVEL PAGING

- Use of CR3 with 4-level paging depends on whether process context identifiers (PCIDs) have been enabled by setting CR4.PCIDE

<p align="center"> <img src="https://i.imgur.com/ycLpWUf.png" width="500px" height="auto"></p>
<p align="center"> <img src="https://i.imgur.com/s6dZ8fY.png" width="500px" height="auto"></p>
<p align="center"> <img src="https://i.imgur.com/QcWP0ZZ.png" width="500px" height="auto"></p>
<p align="center"> <img src="https://i.imgur.com/2gEHP0B.png" width="500px" height="auto"></p>

#### Protection Keys

- The protection-key feature provides an additional mechanism by which 4-level paging controls access to usermode addresses.
- When CR4.PKE = 1, every linear address is associated with the 4-bit protection key located in bits 62:59 of the paging-structure entry that mapped the page containing the linear address.
- The PKRU register determines, for each protection key, whether user-mode addresses with that protection key may be
  read or written.

#### PAGE-FAULT EXCEPTIONS

- Accesses using linear addresses may cause page-fault exceptions (#PF; exception 14).
- An access to a linear address may cause a page-fault exception for either of two reasons:
  1. there is no translation for the linear address;
  2. there is a translation for the linear address, but its access rights do not permit the access.

<p align="center"> <img src="https://i.imgur.com/XIIyvJN.png" width="500px" height="auto"></p>

#### ACCESSED AND DIRTY FLAGS

- Whenever the processor uses a paging-structure entry as part of linear-address translation, it sets the **accessed** flag in that entry (if it is not already set).
- Whenever there is a write to a linear address, the processor sets the dirty flag (if it is not already set) in the paging structure entry that identifies the final physical address for the linear address (either a PTE or a paging-structure entry in which the PS flag is 1).

### Chapter 6 Interrupt and Exception Handling

#### Interrupt And Exception Overview

- **Interrupts** and **exceptions** are events that indicate that a condition exists somewhere in the system, the processor, or within the currently executing program or task that requires the attention of a processor.
- They typically result in a forced transfer of execution from the currently running program or task to a special software routine or task called an **interrupt handler** or an **exception handler**.
- Interrupts occur at random times during the execution of a program, in response to signals from hardware. System hardware uses interrupts to handle events external to the processor, such as requests to service peripheral devices. Software can also generate interrupts by executing the INT n instruction.
- Exceptions occur when the processor detects an error condition while executing an instruction, such as division by zero. The processor detects a variety of error conditions including protection violations, page faults, and internal machine faults.

#### Exception and Interrupt Vectors

- To aid in handling exceptions and interrupts, each architecturally defined exception and each interrupt condition
requiring special handling by the processor is assigned a unique identification number, called a **vector number**.
- The processor uses the vector number assigned to an exception or interrupt as an **index** into the interrupt descriptor table (IDT).
- The allowable range for vector numbers is **0 to 255**. Vector numbers in the range **0 through 31** are reserved by the Intel 64 and IA-32 architectures for architecture-defined exceptions and interrupts.
- Vector numbers in the range **32 to 255** are designated as user-defined interrupts and are not reserved by the Intel 64 and IA-32 architecture. These interrupts are generally assigned to **external I/O devices** to enable those devices to send interrupts to the processor through one of the external hardware interrupt mechanisms .

<p align="center"> <img src="https://i.imgur.com/lSdAuQl.png" width="600px" height="auto"></p>

#### Sources of Interrupts

- The processor receives interrupts from two sources:
  - External (hardware generated) interrupts.
  - Software-generated interrupts.

##### External Interrupts

- External interrupts are received through pins on the processor or through the local APIC.
- When the local APIC is enabled, the `LINT[1:0]` pins can be programmed through the APIC’s **local vector table(LVT)** to be associated with any of the processor’s exception or interrupt vectors
- When the local APIC is global/hardware disabled, these pins are configured as INTR and NMI pins, respectively.

##### Maskable Hardware Interrupts

- Any external interrupt that is delivered to the processor by means of the INTR pin or through the local APIC is called a **maskable hardware interrupt**.
- The IF flag in the EFLAGS register permits all maskable hardware interrupts to be masked as a group.

##### Software-Generated Interrupts

- The **INT n** instruction permits interrupts to be generated from within software by supplying an interrupt vector number as an operand.
- Any of the interrupt vectors from 0 to 255 can be used as a parameter in this instruction.
- If the processor’s predefined NMI vector is used, however, the response of the processor will not be the same as it would be from an NMI interrupt generated in the normal manner
- Interrupts generated in software with the INT n instruction cannot be masked by the IF flag in the EFLAGS register.


#### Sources of Exceptions

- The processor receives exceptions from three sources:
  - Processor-detected program-error exceptions.
  - Software-generated exceptions.
  - Machine-check exceptions.

##### Program-Error Exceptions

- The processor generates one or more exceptions when it detects program errors during the execution in an application program or the operating system or executive.

##### Software-Generated Exceptions

- The **INTO, INT1, INT3, and BOUND** instructions permit exceptions to be generated in software.
- The INT n instruction can be used to emulate exceptions in software; but there is a limitation.
- If INT n provides a vector for one of the architecturally-defined exceptions, the processor generates an interrupt to the correct vector (to access the exception handler) but does not push an error code on the stack. This is true even if the associated hardware-generated exception normally produces an error code. The exception handler will still attempt to pop an error code from the stack while handling the exception. Because no error code was pushed, the handler will pop off and discard the EIP instead (in place of the missing error code). This sends the return to the wrong location.

##### Machine-Check Exceptions

- The P6 family and Pentium processors provide both internal and external machine-check mechanisms for checking the operation of the internal chip hardware and bus transactions.
- These mechanisms are implementation dependent. When a machine-check error is detected, the processor signals a machine-check exception (vector 18) and
returns an error code.

#### Exceptions Classifications

- Exceptions are classified as **faults, traps, or aborts** depending on the way they are reported and whether the instruction that caused the exception can be restarted without loss of program or task continuity:
  - Faults: A fault is an exception that can generally be corrected and that, once corrected, allows the program to be restarted with no loss of continuity. When a fault is reported, the processor restores the machine state to the state prior to the beginning of execution of the faulting instruction. The return address (saved contents of the CS and EIP registers) for the fault handler points to the faulting instruction, rather than to the instruction following the faulting instruction.
  - Traps: A trap is an exception that is reported immediately following the execution of the trapping instruction. Traps allow execution of a program or task to be continued without loss of program continuity. The return address for the trap handler points to the instruction to be executed after the trapping instruction.
  - Aborts: An abort is an exception that does not always report the precise location of the instruction causing the exception and does not allow a restart of the program or task that caused the exception. Aborts are used to report severe errors, such as hardware errors and inconsistent or illegal values in system tables.

#### Exception and Interrupt Handling

### Chapter 10 Advanced Programmable Interrupt Controller (APIC)

- the local APIC performs two primary functions for the processor:
  - receives interrupts from the processor’s interrupt pins, from internal sources and from an external I/O APIC (or other external interrupt controller). It sends these to the processor core for handling.
  - in SMP, it sends and receives interprocessor interrupt (IPI) messages to and from other logical processors on the system bus. IPI messages can be used to distribute interrupts among the processors in the system or to execute system wide functions (such as, booting up processors or distributing work among a group of processors).



### Chapter 11 Memory Cache Control

#### Internal caches, TLBs, and buffers

- The Intel 64 and IA-32 architectures support cache, translation look aside buffers (TLBs), and a store buffer for temporary on-chip (and external) storage of instructions and data.

<p align="center"><img src="https://i.imgur.com/75tM4Iq.png"  width="500px" height="auto"></p>

<p align="center"><img src="https://i.imgur.com/ToePjTl.png"  width="500px" height="auto"></p>


#### Methods Of Caching Available

<p align="center"><img src="https://i.imgur.com/gVtPKG5.png"  width="700px" height="auto"></p>

Intel 64 and IA-32 processors may implement four types of caches: __the trace cache, the level 1 (L1) cache, the level 2 (L2) cache, and the level 3 (L3) cache__:
  - The L1 cache is divided into two sections: one section is dedicated to caching instructions (pre-decoded instructions) and the other caches data.
  - The L2 cache is a unified data and instruction cache. Each processor core has its own L1 and L2.
  - The L3 cache is an inclusive, unified data and instruction cache, shared by all processor cores inside a physical package.
  - No trace cache is implemented..
- The **TLBs** store the **most recently used page-directory** and **page-table entries**. They speed up memory accesses
when paging is enabled by reducing the number of memory accesses that are required to read the page tables stored in system memory.
- The TLBs are divided into four groups:
  - instruction TLBs for 4-KByte pages
  - data TLBs for 4-KByte pages;
  - instruction TLBs for large pages (2-MByte, 4-MByte or 1-GByte pages),
  - and data TLBs for large pages.

#### Page Attribute Table (PAT)

- The **Page Attribute Table (PAT)** extends the IA-32 architecture’s page-table format to allow memory types to be
assigned to regions of physical memory based on linear address mappings.
- The PAT is a companion feature to the MTRRs; that is:
  - MTRRs allow mapping of memory types to regions of the physical address space,
  - where the PAT allows mapping of memory types to pages within the linear address space.

### Chapter 17 Debug, Branch Profile, TSC, and Intel® Resource Director Technology (Intel® RDT) Features

#### Overview of Debug Support Facilities

- The following processor facilities support debugging and performance monitoring:
  - **Debug exception (#DB)** — Transfers program control to a debug procedure or task when a debug event occurs.
  - **Breakpoint exception (#BP)** — See breakpoint instruction (INT3) below.
  - **Breakpoint-address registers (DR0 through DR3)** — Specifies the addresses of up to 4 breakpoints.
  - **Debug status register (DR6)** — Reports the conditions that were in effect when a debug or breakpoint exception was generated.
  - **Debug control register (DR7)** — Specifies the forms of memory or I/O access that cause breakpoints to be generated.
  - **Trap flag, TSS** — Generates a debug exception (#DB) when an attempt is made to switch to a task with the T flag set in its TSS.
  - **RF (resume) flag, EFLAGS register** — Suppresses multiple exceptions to the same instruction.
  - **TF (trap) flag, EFLAGS register** — Generates a debug exception (#DB) after every execution of an instruction.
  - **Breakpoint instruction (INT3)** — Generates a breakpoint exception (#BP) that transfers program control to the debugger procedure or task. This instruction is an alternative way to set instruction breakpoints. It is especially useful when more than four breakpoints are desired, or when breakpoints are being placed in the source code.
  - **Last branch recording facilities** — Store branch records in the **last branch record (LBR)** stack MSRs for the most recent taken branches, interrupts, and/or exceptions in MSRs. A branch record consist of a branch-from and a branch-to instruction address. Send branch records out on the system bus as branch trace messages (BTMs).

#### Debug Exceptions

- The Intel 64 and IA-32 architectures dedicate two interrupt vectors to handling debug exceptions: vector 1 (debug exception, #DB) and vector 3 (breakpoint exception, #BP). The following sections describe how these exceptions are generated and typical exception handler operations.

<p align="center"><img src="https://i.imgur.com/U1P42xk.png"  width="500px" height="auto"></p>

- Debug registers are privileged resources; a MOV instruction that accesses these registers can only be executed in real-address mode, in SMM or in protected mode at a CPL of 0.
- An attempt to read or write the debug registers from any other privilege level generates a general-protection exception (#GP).

##### Debug Address Registers (DR0-DR3)

- Each of the debug-address registers (DR0 through DR3) holds the **32-bit linear address** of a breakpoint
- Breakpoint comparisons are made before physical address translation occurs. The contents of debug register DR7 further specifies breakpoint conditions.

#### Debug Registers DR4 and DR5

- Debug registers DR4 and DR5 are reserved when debug extensions are enabled (when the DE flag in control register CR4 is set) and attempts to reference the DR4 and DR5 registers cause invalid-opcode exceptions (#UD).
- When debug extensions are not enabled (when the DE flag is clear), these registers are aliased to debug registers DR6 and DR7.

#### Debug Status Register (DR6)

- The debug status register (DR6) reports debug conditions that were sampled at the time the last debug exception was generated.
- Updates to this register only occur when an exception is generated. The flags in this register show the following information:
  - **B0 through B3 (breakpoint condition detected) flags (bits 0 through 3)** — Indicates (when set) that its associated breakpoint condition was met when a debug exception was generated.
  - **BD (debug register access detected) flag (bit 13)** — Indicates that the next instruction in the instruction stream accesses one of the debug registers (DR0 through DR7). This flag is enabled when the GD (general detect) flag in debug control register DR7 is set.
  - **BS (single step) flag (bit 14)** — Indicates (when set) that the debug exception was triggered by the singlestep execution mode (enabled with the TF flag in the EFLAGS register). The single-step mode is the highest priority debug exception. When the BS flag is set, any of the other debug status bits also may be set.
  - **BT (task switch) flag (bit 15)** — Indicates (when set) that the debug exception resulted from a task switch where the T flag (debug trap flag) in the TSS of the target task was set.
  - **RTM (restricted transactional memory) flag (bit 16)** — Indicates (when clear) that a #DB or #BP occurred inside an RTM region while advanced debugging of RTM transactional regions was enabled. This bit is set for any other debug exception (including all those that occur when advanced debugging of RTM transactional regions is not enabled). This bit is always 1 if the processor does not support RTM.

### Chapter 22 Architecture Compatibility

#### Model-Specific Registers

- The Pentium processor introduced a set of model-specific registers (**MSRs**) for use in controlling hardware functions and performance monitoring.

#### Memory Type Range Registers

- Memory type range registers (**MTRRs**) are a new feature introduced into the IA-32 in the Pentium Pro processor. MTRRs allow the processor to optimize memory operations for different types of memory, such as RAM, ROM, frame buffer memory, and memory-mapped I/O.
- MTRRs are MSRs that contain an internal map of how physical address ranges are mapped to various types of memory. The processor uses this internal memory map to determine the **cacheability** of various physical memory locations and the optimal method of accessing memory locations.
- For example, if a memory location is specified in an MTRR as write-through memory, the processor handles accesses to this location as follows. It reads data from that location in lines and caches the read data or maps all writes to that location to the bus and updates the cache to maintain cache coherency.
- In mapping the physical address space with MTRRs, the processor recognizes five types of memory: _uncacheable (UC), uncacheable, speculatable, write-combining (WC), write-through (WT), write-protected (WP), and writeback (WB)_.

### Chapter 23 Introduction To Virtual Machine Extensions

#### Virtual Machine Architecture

- Virtual Machine Monitor (VMM) aka (Hypervisor) act as host and has full control of the processor(s) and the hardware (physical memory, interrupt management and I/O).
- It provides the guest or the (Virtual Machine) with an abstraction of a virtual processor, allowing it to think it is execute directly on the LP.

#### Introduction To VMX Operation

- Processor support for virtualization is provided by a form of processor operation called VMX (Virtual Machine eXtensions).
- There are two kind of VMX operations: 
  - VMX root operation (VMM will generally run here).
  - VMX non-root operation (VM will generally run here).
- The main differences between these two modes is that in root mode, a new set of new instructions (VMX instructions) is available and that the values that can be loaded into certain control registers are limited.

#### Life Cycle of VMM Software

- The following items summarize the life cycle of a VMM and its guest software as well as the interactions between them:
  - Software enters VMX operation by executing a VMXON instruction.
  - Using VM entries, a VMM can then turn guests into VMs (one at a time). The VMM effects a VM entry using instructions VMLAUNCH and VMRESUME; it regains control using VM exits.
  - VM exits transfer control to an entry point specified by the VMM. The VMM can take action appropriate to the cause of the VM exit and can then return to the VM using a VM entry.
  - Eventually, the VMM may decide to shut itself down and leave VMX operation. It does so by executing the VMXOFF instruction.

<p align="center"><img src="https://i.imgur.com/Vpvq9Gi.png"  width="400px" height="auto"></p>

#### Virtual-machine Control Structure

- VMCS is a data structure which control the behavior of processor in VMX non-root mode and control VMX transitions.
- Access to the VMCS is managed through a component of processor state called the VMCS pointer (one per LP).
- The VMCS pointer (64-bit) contain the physical address of the the VMCS region and should be aligned to a 4-KB boundary.
- The VMCS pointer is read and written using the instructions VMPTRST and VMPTRLD.
- The VMM configures a VMCS using the VMREAD, VMWRITE, and VMCLEAR instructions.
- A VMM can use a different VMCS for each VM that it supports.
- For a VM with multiple LPs(LPs) or Virtual CPUs (vCPUs), the VMM can use a different VMCS for each vCPU.

#### Discovering Support For VMX

- Before system software enters into VMX operation, it must discover the presence of VMX support in the processor.
- This is achieved by executing cpuid (1) and checking if ECX.VMX (bit 5) = 1, them VMX operations is supported.
- The VMX architecture is designed to be extensible so that future processors in VMX operation can support additional features not present in first-generation implementations of the VMX architecture.
- The availability of extensible VMX features is reported to software using a set of VMX capability MSRs.

#### Enabling And Entering VMX Operation

- Before system software can enter VMX operation, it enables VMX by setting **CR4.VMXE[bit 13] = 1**.
- VMXON causes an invalid-opcode exception (#UD) if executed with CR4.VMXE = 0.
- Once in VMX operation, it is not possible to clear CR4.VMXE. CR4.VMXE can be cleared outside of VMX operation after executing of VMXOFF.
- VMXON is also controlled by the IA32_FEATURE_CONTROL MSR (MSR address 3AH). The relevant bits of the MSR are:
  - Bit 0 is the lock bit: If this bit is 0, VMXON causes a #GP exception. If the lock bit is 1, WRMSR to this MSR causes a #GP exception.
  - Bit 1 enables VMXON in SMX operation: If this bit is 0, execution of VMXON in SMX operation causes a #GP exception.
  - Bit 2 enables VMXON outside SMX operation. If this bit is 0, execution of VMXON outside SMX operation causes a #GP exception.
- To enable VMX support in a platform, **BIOS must set bit 1, bit 2, or both , as well as the lock bit**.
- Before executing VMXON, software should allocate a naturally aligned **4-KByte** region of memory that a LP may use to support VMX operation. This region is called the **VMXON region**. The address of the VMXON region (the VMXON pointer) is provided in an operand to VMXON.

#### Restrictions On VMX Operation

- VMX operation places restrictions on processor operation:
  - In VMX operation, processors may fix certain bits in **CR0** and **CR4** to specific values and not support other values. VMXON fails if any of these bits contains an unsupported value.
  - Any attempt to set one of these bits to an unsupported value while in VMX operation using any of the CLTS, LMSW, or MOV CR instructions causes a #GP exception. VM entry or VM exit cannot set any of these bits to an unsupported value.
- Software should consult the VMX capability:
  - MSRs **IA32_VMX_CR0_FIXED0** and **IA32_VMX_CR0_FIXED1** to determine how bits are fixed for CR0.
  - MSRs **IA32_VMX_CR4_FIXED0** and **IA32_VMX_CR4_FIXED1** to determine how bits are fixed for CR4.
- The first processors to support VMX operation require that the following bits be 1 in VMX operation: **CR0.PE, CR0.NE, CR0.PG, and CR4.VMXE**.
- The restrictions on CR0.PE and CR0.PG imply that VMX operation is supported only in paged protected mode. Therefore, guest software cannot be run in unpaged protected mode or in real-address mode.
- Later processors support a VM-execution control called “unrestricted guest”. If this control is 1, CR0.PE and CR0.PG may be 0 in VMX non-root operation (even if the capability MSR IA32_VMX_CR0_FIXED0 reports otherwise). Such processors allow guest software to run in unpaged protected mode or in real-address mode.

### Chapter 24 Virtual Machine Control Structures

#### Overview

- At any given time, at most one of the active VMCSs is the current VMCS.
- The VMLAUNCH, VMREAD, VMRESUME, and VMWRITE instructions operate only on the current VMCS.
- The VMCS link pointer field in the current VMCS is itself the address of a VMCS.
- If VM entry is performed successfully with the 1-setting of the “VMCS shadowing” VM-execution control, the VMCS referenced by the VMCS link pointer field becomes active on the LP. The identity of the current VMCS does not change.
- The launch state of a VMCS determines which VM-entry instruction should be used with that VMCS.
- Executing a VMPTRLD makes the VMCS is both active and current on the LP.
- Executing a VMCLEAR makes the VMCS neither active nor current on the LP.
- Executing a VMLAUNCH instruction requires a VMCS whose launch state is “clear” and makes the launch state "launched".
- Executing a VMRESUME instruction requires a VMCS whose launch state is “launched”.

<p align="center"><img src="https://i.imgur.com/RFdaRU8.png"  width="500px" height="auto"></p>

#### Format Of The VMCS Region

- A VMCS region comprises up tp 4KB. To determine the exact size of the VMCS region, check VMX capability MSR IA32_VMX_BASIC.
- The format of a VMCS is structured as below:

| Byte Offset | Contents                                                           |
| ----------- | ------------------------------------------------------------------ |
| 0           | Bits 30:0: VMCS revision identifier, Bit 31: shadow-VMCS indicator |
| 4           | VMX-abort indicator                                                |
| 8           | VMCS data (implementation-specific format)                         |

- The first 4 bytes of the VMCS region contain the VMCS revision identifier at bits 30:0. This helps to avoid using a VMCS region formatted for one processor on a processor that uses a different format.
- Software can discover the VMCS revision identifier that a processor uses by reading the VMX capability MSR IA32_VMX_BASIC.
- VMPTRLD fails if its operand references a VMCS region whose VMCS revision identifier differs from that used by the processor.
- Bit 31 of this 4-byte region indicates whether the VMCS is a shadow VMCS.
- Software can discover support for this setting by reading the VMX capability MSR IA32_VMX_PROCBASED_CTLS2
- VMPTRLD fails if the shadow-VMCS indicator is set and the processor does not support the 1-setting of the “VMCS shadowing” VM-execution control.
- The next 4 bytes of the VMCS region are used for the VMX-abort indicator. The contents of these bits do not control processor operation in any way. A LP writes a non-zero value into these bits if a VMX abort occurs. Software may also write into this field.
- The remainder of the VMCS region is used for VMCS data (those parts of the VMCS that control VMX non-root operation and the VMX transitions). The format of these data is implementation-specific.
- To ensure proper behavior in VMX operation, software should maintain the VMCS region and related structures in writeback cacheable memory, check the VMX capability MSR IA32_VMX_BASIC.

#### Organization Of VMCS Data

The VMCS data are organized into six logical groups:
- Guest-state area: Processor state is saved into the guest-state area on VM exits and loaded from there on VM entries.
- Host-state area: Processor state is loaded from the host-state area on VM exits.
- VM-execution control fields: These fields control processor behavior in VMX non-root operation. They determine in part the causes of VM exits.
- VM-exit control fields: These fields control VM exits.
- VM-entry control fields: These fields control VM entries.
- VM-exit information fields: These fields receive information on VM exits and describe the cause and the nature of VM exits. On some processors, these fields are read-only.

#### VMCS Layout

- The VMCS layout is available in a form of table here: [VMCS Layout.pdf](../master/VMCS-Layout.pdf)

#### VMCS Types: Ordinary And Shadow

- Every VMCS is either an **ordinary VMCS** or a **shadow VMCS**. A VMCS’s type is determined by the shadow-VMCS indicator in the VMCS region.
- A shadow VMCS differs from an ordinary VMCS in two ways: - An ordinary VMCS can be used for VM entry but a shadow VMCS cannot. Attempts to perform VM entry when the current VMCS is a shadow VMCS fail. - The VMREAD and VMWRITE instructions can be used in VMX non-root operation to access a shadow VMCS but not an ordinary VMCS.

#### Software Use of Virtual-Machine Control Structures

- To ensure proper processor behavior, software should observe certain guidelines when using an active VMCS: - No VMCS should ever be active on more than one LP - Software should not modify the shadow-VMCS indicator (see Table 24-1) in the VMCS region of a VMCS that is active. - Software should use the VMREAD and VMWRITE instructions to access the different fields in the current VMCS.

#### VMREAD, VMWRITE, and Encodings of VMCS Fields

- Every field of the VMCS is associated with a 32-bit value that is its **encoding**. The encoding is provided in an operand to VMREAD and VMWRITE when software wishes to read or write that field.
- The structure of the 32-bit encodings of the VMCS components is determined principally by the width of the fields and their function in the VMCS.
  <p align="center"><img src="https://i.imgur.com/Ve19xHd.png"  width="600px" height="auto"></p>

#### Initializing a VMCS

- Software should initialize fields in a VMCS (using VMWRITE) before using the VMCS for VM entry.
- A processor maintains some VMCS information that cannot be modified with the VMWRITE instruction; this includes a VMCS’s launch state. Such information may be stored in the VMCS data portion of a VMCS region. Because the format of this information is **implementation-specific**, there is no way for software to know, when it first allocates a region of memory for use as a VMCS region, how the processor will determine this information from the contents of the memory region.
- VMCLEAR should be executed for a VMCS before it is used for VM entry for the first time.
- VMLAUNCH should be used for the first VM entry using a VMCS after VMCLEAR has been executed for that VMCS.
- VMRESUME should be used for any subsequent VM entry using a VMCS (until the next execution of VMCLEAR for the VMCS)

#### VMXON Region

- The amount of memory required for the VMXON region is the same as that required for a VMCS region. This size is implementation specific and can be determined by consulting the VMX capability **MSR IA32_VMX_BASIC**.
- Software can determine a processor’s physical-address width by executing CPUID with **80000008H** in EAX. The physical-address width is returned in bits 7:0 of EAX.
- If IA32_VMX_BASIC[48] is read as 1, the VMXON pointer must not set any bits in the range 63:32.
- Before executing VMXON, software should write the **VMCS revision identifier** to the VMXON region. (Specifically, it should write the 31-bit VMCS revision identifier to bits 30:0 of the first 4 bytes of the VMXON region; bit 31 should be cleared to 0.) => `__readmsr(MSR_IA32_VMX_BASIC~0x480);`
- Software should use a separate region for each LP and should not access or modify the VMXON region of a LP between execution of VMXON and VMXOFF on that LP. Doing otherwise may lead to unpredictable behavior.
- The VMXON region should be zeroed prior to executing vmxon.
  <p align="center"><img src="https://i.imgur.com/fyqHlHu.png"  width="600px" height="auto"></p>

### Chapter 25 VMX Non-Root Operation

#### Instructions That Cause VM Exits Unconditionally

- The following instructions cause VM exits when they are executed in VMX non-root operation: - CPUID, GETSEC, INVD, and XSETBV. - INVEPT, INVVPID, VMCALL, VMCLEAR, VMLAUNCH, VMPTRLD, VMPTRST, VMRESUME, VMXOFF, and VMXON.

#### Instructions That Cause VM Exits Conditionally

- Instructions cause VM exits in VMX non-root operation depending on the setting of the VM-execution controls. - CLTS - ENCLS - HLT - IN, INS/INSB/INSW/INSD, OUT, OUTS/OUTSB/OUTSW/OUTSD. - INVLPG - INVPCID - LGDT, LIDT, LLDT, LTR, SGDT, SIDT, SLDT, STR - LMSW - MONITOR - MOV from CR3/CR8, MOV to CR0/1/3/4/8 - MOV DR - MWAIT - PAUSE - RDMSR, WRMSR - RDPMC - RDRAND, RDSEED - RDTSC, RDTSCP - RSM - VMREAD, VMWRITE - WBINVD - XRSTORS, XSAVES

#### Other Causes Of VM-Exits

- In addition to VM exits caused by instruction execution, the following events can cause VM exits:
- **Exceptions**: - (faults, traps, and aborts) cause VM exits based on the exception bitmap). - If an exception occurs, its vector (in the range 0–31) is used to select a bit in the exception bitmap. - If the bit is 1, a VM exit occurs; if the bit is 0, the exception is delivered normally through the guest IDT.
- **Triple fault**: - A VM exit occurs if the LP encounters an exception while attempting to call the double-fault handler and that exception itself does not cause a VM exit due to the exception bitmap.
- **External interrupts**: - causes a VM exit if the _external-interrupt exiting_ VM-execution control is 1. - Otherwise, the interrupt is delivered normally through the IDT.
- **Non-maskable interrupts (NMIs)**: - causes a VM exit if the _NMI exiting_ VM-execution control is 1. - Otherwise, it is delivered using descriptor 2 of the IDT.
- **INIT signals**: - cause VM exits. A LP performs none of the operations normally associated with these events. - Such exits do not modify register state or clear pending events as they would outside of VMX operation.
- **Start-up IPIs (SIPIs)**: - cause VM exits. If a LP is not in the wait-for-SIPI activity state when a SIPI arrives, no VM exit occurs and the SIPI is discarded.
- **Task switches**: - are not allowed in VMX non-root operation. Any attempt to effect a task switch in VMX non-root operation causes a VM exit.
- **System-management interrupts (SMIs)**: - If the logical processor is using the dual-monitor treatment of SMIs and system-management mode (SMM), SMIs cause SMM VM exits.
- **VMX-preemption timer**: - A VM exit occurs when the timer counts down to zero.

### Chapter 28 VMX Support For Address Translation

#### VIRTUAL PROCESSOR IDENTIFIERS (VPIDS)

- The original architecture for VMX operation required VMX transitions to flush the TLBs and paging-structure caches.
- This ensured that translations cached for the old linear-address space would not be used after the transition.
- **Virtual-processor identifiers (VPIDs)** introduce to VMX operation a facility by which a logical processor may cache
  information for multiple linear-address spaces.
- When VPIDs are used, VMX transitions may retain cached information and the logical processor switches to a different linear-address space.

#### THE EXTENDED PAGE TABLE MECHANISM (EPT)

- The **extended page-table mechanism (EPT)** is a feature that can be used to support the virtualization of physical
  memory.
- When EPT is in use, certain addresses that would normally be treated as physical addresses (and used to access memory are instead treated as guest-physical addresses.
- Guest-physical addresses are translated by traversing a set of EPT paging structures to produce physical addresses that are used to access memory.

#### EPT Overview

- The translation from guest-physical addresses to physical addresses is determined by a set of EPT paging structures.
- The EPT paging structures are similar to those used to translate linear addresses while the processor is in IA-32e mode.
- Assume, for example, that `CR4.PAE = CR4.PSE = 0`. The translation of a 32-bit linear address then operates as follows:
  - Bits 31:22 of the linear address select an entry in the guest page directory located at the guest-physical address in CR3. The guest-physical address of the guest page-directory entry (PDE) is translated through EPT to determine the guest PDE’s physical address.
  - Bits 21:12 of the linear address select an entry in the guest page table located at the guest-physical address in the guest PDE. The guest-physical address of the guest page-table entry (PTE) is translated through EPT to determine the guest PTE’s physical address.
  - Bits 11:0 of the linear address is the offset in the page frame located at the guest-physical address in the guest PTE. The guest-physical address determined by this offset is translated through EPT to determine the physical address to which the original linear address translates.
- EPT specifies the privileges that software is allowed when accessing the address. Attempts at disallowed accesses are called **EPT violations** and cause VM exits.
- A processor uses EPT to translate guest-physical addresses only when those addresses are used to access memory.

#### EPT Translation Mechanism

- The EPT translation mechanism uses only bits 47:0 of each guest-physical address.
- It uses a page-walk length of 4, meaning that at most 4 EPT paging-structure entries are accessed to translate a guest-physical address.
- There are different setups of EPT:
  - EPT Page-Directory-Pointer-Table Entry (PDPTE) that Maps a 1-GByte Page
  - EPT Page-Directory Entry (PDE) that Maps a 2-MByte Page

### Chapter 30 VMX Instruction Reference

#### Overview

- The virtual-machine extensions (VMX) includes five instructions that manage the virtual-machine control structure (VMCS, four instructions that manage VMX operation, two TLB-management instructions, and two instructions for use by guest software.
- The behavior of the VMCS-maintenance instructions is summarized below:
  - **VMPTRLD** — This instruction takes a single 64-bit source operand that is in memory. It makes the referenced VMCS active and current, loading the current-VMCS pointer with this operand and establishes the current VMCS based on the contents of VMCS-data area in the referenced VMCS region. Because this makes the referenced VMCS active, a logical processor may start maintaining on the processor some of the VMCS data for the VMCS.
  - **VMPTRST** — This instruction takes a single 64-bit destination operand that is in memory. The current-VMCS pointer is stored into the destination operand.
  - **VMCLEAR** — This instruction takes a single 64-bit operand that is in memory. The instruction sets the launch state of the VMCS referenced by the operand to “clear”, renders that VMCS inactive, and ensures that data for the VMCS have been written to the VMCS-data area in the referenced VMCS region. If the operand is the same as the current-VMCS pointer, that pointer is made invalid.
  - **VMREAD** — This instruction reads a component from a VMCS (the encoding of that field is given in a register operand) and stores it into a destination operand that may be a register or in memory.
  - **VMWRITE** — This instruction writes a component to a VMCS (the encoding of that field is given in a register operand) from a source operand that may be a register or in memory.
- The behavior of the VMX management instructions is summarized below:
  - **VMLAUNCH** — This instruction launches a virtual machine managed by the VMCS. A VM entry occurs, transferring control to the VM.
  - **VMRESUME** — This instruction resumes a virtual machine managed by the VMCS. A VM entry occurs, transferring control to the VM.
  - **VMXOFF** — This instruction causes the processor to leave VMX operation.
  - **VMXON** — This instruction takes a single 64-bit source operand that is in memory. It causes a logical processor to enter VMX root operation and to use the memory referenced by the operand to support VMX operation.
- The behavior of the VMX-specific TLB-management instructions is summarized below:
  - **INVEPT** — This instruction invalidates entries in the TLBs and paging-structure caches that were derived from extended page tables (EPT).
  - **INVVPID** — This instruction invalidates entries in the TLBs and paging-structure caches based on a VirtualProcessor Identifier (VPID).

## Appendix A VMX Capability Reporting Facility

### RESERVED CONTROLS AND DEFAULT SETTINGS

- Certain VMX controls are reserved and must be set to a specific value (0 or 1) determined by the processor.
- The specific value to which a reserved control must be set is its **default setting**.
- Software can discover the default setting of a reserved control by consulting the appropriate VMX capability MSR
- Default settings partition the various controls into the following classes:
  - **Always-flexible**. These have never been reserved.
  - **Default0**. These are (or have been) reserved with a default setting of 0.
  - **Default1**. They are (or have been) reserved with a default setting of 1.

### VMX-FIXED BITS IN CR0

- Each bit in CR0 is either fixed to 0 (with value 0 in both MSRs), fixed to 1 (1 in both MSRs), or flexible (0 in `IA32_VMX_CR0_FIXED0` and 1 in `IA32_VMX_CR0_FIXED1`).

### VMX-FIXED BITS IN CR4

- Each bit in CR4 is either fixed to 0 (with value 0 in both MSRs), fixed to 1 (1 in both MSRs), or flexible (0 in `IA32_VMX_CR4_FIXED0` and 1 in `IA32_VMX_CR4_FIXED1`).
