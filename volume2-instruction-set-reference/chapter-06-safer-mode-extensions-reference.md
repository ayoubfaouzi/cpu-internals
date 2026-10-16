# Chapter 6 Safer Mode Extensions Reference

#### Overview

- Provide a programming interface for system software to establish a measured environment within the platform to support trust decisions by end users.
- The measurement and protection mechanisms used by a measured environment are supported by the capabilities of an Intel TXT.

#### SMX functionality

- SMX functionality is provided in an Intel 64 processor through the **GETSEC** instruction via leaf functions.
- Software can detect support for SMX operation using the ```CPUID.01H.ECX[Bit 6] == 1```
