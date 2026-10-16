# Chapter 6 Procedure Calls, Interrupts, and Exceptions

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
