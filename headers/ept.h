// ===========================================================================
// EPT Pointer (EPTP)
// Intel SDM Vol. 3C, Section 28.2.2 - "Format of the EPTP"
//
// The EPTP is a 64-bit value written to the VMCS field EPTP (encoding 0x201A)
// via VMWRITE. It tells the CPU where the EPT PML4 table resides in host-
// physical memory, and configures global EPT behavior for the current VMCS.
//
// Every logical processor in a VM has its own VMCS, and therefore its own
// EPTP. In practice, multiple vCPUs sharing the same guest physical address
// space will point their EPTPs at the same PML4 table, differing only in
// per-vCPU fields if any.
//
// The EPTP is loaded on every VM-entry and takes effect immediately for all
// EPT page walks performed while the guest is running.
// ===========================================================================
typedef union _EPTP
{
    ULONG64 All;
    struct
    {
        //
        // [Bits 2:0] EPT MEMORY TYPE for the EPT paging structures themselves.
        // Specifies the memory type the CPU uses when accessing the EPT tables
        // (PML4, PDPT, PD, PT) during a page walk — NOT the memory type of the
        // guest-physical pages being mapped.
        //
        // Valid values:
        //   0 = Uncacheable (UC) — EPT table accesses bypass all caches.
        //                          Use only for debugging or non-cacheable RAM.
        //   6 = Write-Back  (WB) — EPT table accesses use the cache normally.
        //                          This is the correct value for normal operation
        //                          and should always be used when the PML4 table
        //                          resides in regular WB host-physical RAM.
        //
        // All other values (1, 2, 3, 4, 5, 7) are reserved and will cause an
        // EPT Misconfiguration VM-exit. Query supported memory types via
        // IA32_VMX_EPT_VPID_CAP MSR (0x48C), bits 8 (UC) and 14 (WB).
        //
        UINT64 MemoryType : 3;

        //
        // [Bits 5:3] EPT PAGE WALK LENGTH (minus one)
        // Specifies the number of levels in the EPT paging structure, encoded
        // as (desired_levels - 1).
        //
        // Valid values:
        //   3 (binary 011) → 4-level EPT walk (PML4 → PDPT → PD → PT)
        //                    Supports up to 256 TB of GPA space.
        //                    This is the standard and most widely used mode.
        //
        //   4 (binary 100) → 5-level EPT walk (PML5 → PML4 → PDPT → PD → PT)
        //                    Extends GPA space to 128 PB (petabytes).
        //                    Requires CPU support — check CPUID.07H:ECX[16]
        //                    (LA57) for guest linear address support, and
        //                    IA32_VMX_EPT_VPID_CAP MSR (0x48C) for EPT
        //                    5-level walk enumeration before enabling.
        //                    Supported on Ice Lake (2019) and later processors.
        //
        // Any value not enumerated as supported in IA32_VMX_EPT_VPID_CAP
        // will cause a VM-entry failure (VMCS invalid).
        //
        // Set to 3 for standard 4-level EPT, or 4 if your hypervisor needs
        // to expose more than 256 TB of GPA space to the guest.
        //
        UINT64 PageWalkLength : 3;

        //
        // [Bit 6] ENABLE ACCESSED AND DIRTY FLAGS
        // When set to 1, the CPU will set the Accessed bit in EPT entries
        // when they are used during a page walk, and the Dirty bit in EPT
        // leaf entries (PTE or large-page PDE/PDPTE) on the first write to
        // the mapped guest-physical page.
        //
        // This feature must be supported by the CPU before enabling it.
        // Check IA32_VMX_EPT_VPID_CAP MSR (0x48C) bit 21 before setting this.
        //
        // When ENABLED (1):
        //   - EPT A/D bits in all EPT table entries become active.
        //   - The CPU maintains Accessed/Dirty state autonomously.
        //   - Hypervisor can track which GPAs have been accessed or dirtied
        //     without taking EPT Violation VM-exits for every access.
        //   - Essential for efficient live migration dirty page tracking and
        //     guest memory working set estimation.
        //   - Page Modification Logging (PML) requires this bit to be set.
        //
        // When DISABLED (0):
        //   - Accessed/Dirty bits in EPT entries are ignored by hardware.
        //   - Software should keep those bits as 0 to avoid reserved-bit
        //     EPT Misconfiguration faults on some implementations.
        //
        UINT64 EnableAccessedAndDirtyFlags : 1;

        //
        // [Bit 7] ENABLE SUPERVISOR SHADOW STACK (CET)
        // Controls whether supervisor shadow-stack pages are enforced via EPT
        // as part of Intel CET (Control-flow Enforcement Technology) support
        // in VMX operation.
        //
        // When set to 1, EPT leaf entries may designate pages as supervisor
        // shadow-stack pages via their SPP (Sub-Page Permission) semantics,
        // and the CPU enforces CET shadow-stack access rules at the EPT level.
        //
        // Requires:
        //   - CPU support for CET in VMX (check CPUID and VMX capability MSRs)
        //   - CR4.CET must be set in the host
        //
        // Most hypervisors set this to 0 unless explicitly implementing
        // CET-based control flow integrity for guest supervisor code.
        //
        UINT64 EnableSupervisorShadowStack : 1;

        //
        // [Bits 11:8] RESERVED (Must Be Zero)
        // These bits are reserved and must be written as 0.
        // Writing non-zero values here will cause a VM-entry failure
        // (the VMCS will be deemed invalid during VM-entry checks).
        //
        UINT64 Reserved1 : 4;

        //
        // [Bits (N-1):12] HOST-PHYSICAL ADDRESS of EPT PML4 TABLE
        // Specifies the 4 KB-aligned host-physical address of the root EPT
        // PML4 table for this VMCS. This is the entry point for all EPT
        // page walks performed while the guest runs under this VMCS.
        //
        // Requirements:
        //   - Must be 4 KB aligned (bits 11:0 are covered by the fields above,
        //     and the CPU treats bits 11:0 of the table address as 0).
        //   - Must be within MAXPHYADDR. Bits above MAXPHYADDR must be zero,
        //     otherwise VM-entry will fail.
        //   - The PML4 table must be in host-physical memory accessible to
        //     the VMX hardware (i.e., not in device memory or MMIO space).
        //   - The table must remain valid and accessible for the entire
        //     lifetime of the VMCS. Freeing it while the VMCS is active
        //     will cause silent memory corruption or a fatal EPT violation.
        //
        // Obtain the physical address of your PML4 table via MmGetPhysicalAddress()
        // on Windows or virt_to_phys() on Linux.
        //
        // To switch a guest to a different EPT context (e.g., for EPTP switching
        // or nested virtualization), update this field via VMWRITE and execute
        // INVEPT to invalidate stale TLB entries derived from the old PML4.
        //
        UINT64 PhysicalAddress : 36;

        //
        // [Bits 51:N] RESERVED (Must Be Zero)
        // Upper bits of the physical address field beyond MAXPHYADDR.
        // Must be zero. Width depends on the CPU's MAXPHYADDR value.
        // See notes on PhysicalAddress above.
        //
        UINT64 Reserved2 : 4;

        //
        // [Bits 63:52] RESERVED (Must Be Zero)
        // Reserved by Intel. Must be written as 0.
        // Unlike the ignored bits in EPT table entries, these bits in the
        // EPTP are NOT available for software use and must remain zero.
        //
        UINT64 Reserved3 : 12;

    } Fields;

} EPTP, *PEPTP;

static_assert(sizeof(EPTP) == 8, "EPTP must be 8 bytes");


// ===========================================================================
// EPT PML4 Entry (Page Map Level 4 Entry)
// Intel SDM Vol. 3C, Table 28-1
//
// The PML4 is the top-level table in the EPT hierarchy.
// Each PML4E covers a 512 GB region of Guest Physical Address (GPA) space.
// There are 512 entries per table, spanning the full 256 TB GPA space.
//
// This entry is ALWAYS non-leaf: it points to an EPT Page Directory Pointer
// Table (PDPT). It never maps a page directly and has no PageSize bit.
//
// Referenced via the EPT Pointer (EPTP) stored in the VMCS, bits 51:12.
// ===========================================================================
typedef union _EPT_PML4E
{
    ULONG64 All;
    struct
    {
        //
        // [Bit 0] READ
        // Allows read access to the 512 GB GPA region controlled by this entry.
        //
        // If this bit is 0, the entry is considered NOT PRESENT, regardless of
        // the Write and Execute bits. Any GPA access in this region will trigger
        // an EPT Violation VM-exit (exit reason 48).
        //
        // This bit also acts as the presence bit: the CPU checks it first during
        // the EPT page walk before evaluating any other field in this entry.
        //
        UINT64 Read : 1;

        //
        // [Bit 1] WRITE
        // Allows write access to the 512 GB GPA region controlled by this entry.
        //
        // If Read=0 and Write=1, the entry is still treated as not present.
        // Write=1 is only meaningful when Read=1.
        //
        // Note: Unlike CR3-based paging where W=0 implies read-only for CPL3,
        // EPT write protection applies uniformly across all privilege levels
        // unless overridden by guest paging.
        //
        UINT64 Write : 1;

        //
        // [Bit 2] EXECUTE (Supervisor)
        // Allows instruction fetches from the 512 GB GPA region.
        //
        // When Mode-Based Execute Control (MBEC) is DISABLED (VMCS
        // secondary proc-based control bit 22 = 0), this bit controls execute
        // access for ALL modes (user and supervisor).
        //
        // When MBEC is ENABLED, this bit controls execute access for
        // SUPERVISOR-MODE only. User-mode execute is controlled separately
        // by ExecuteForUserMode (bit 10).
        //
        UINT64 Execute : 1;

        //
        // [Bits 7:3] RESERVED (Must Be Zero)
        // These bits are reserved by Intel and MUST be set to 0.
        //
        // Writing a non-zero value to any of these bits will cause an
        // EPT Misconfiguration VM-exit (exit reason 49) on the next GPA
        // translation that walks through this entry. This is a fatal
        // hypervisor error and will prevent the guest from running.
        //
        UINT64 Reserved1 : 5;

        //
        // [Bit 8] ACCESSED
        // Set to 1 by the CPU when this entry is used during an EPT page walk,
        // indicating that the covered GPA range has been accessed.
        //
        // This bit is only valid when EPT Accessed and Dirty (A/D) flags are
        // ENABLED via EPTP bit 6. If A/D is disabled:
        //   - This bit is ignored by hardware on reads.
        //   - It MUST be 0 when writing, otherwise an EPT Misconfiguration
        //     VM-exit may occur on some processor implementations.
        //
        // Useful for hypervisor memory tracking (e.g., working set estimation,
        // live migration dirty tracking).
        //
        UINT64 Accessed : 1;

        //
        // [Bit 9] IGNORED
        // Ignored by hardware. Available for software use.
        //
        // Hypervisors commonly repurpose ignored bits to store metadata, such
        // as a flag indicating that this PML4E points to a hypervisor-managed
        // table that should not be freed or modified by guest-driven operations.
        //
        UINT64 Ignored1 : 1;

        //
        // [Bit 10] EXECUTE FOR USER MODE (MBEC)
        // Controls execute access specifically for USER-MODE code.
        //
        // Only meaningful when Mode-Based Execute Control (MBEC) is ENABLED
        // via VMCS Secondary Processor-Based VM-Execution Controls bit 22.
        //
        // When MBEC is ENABLED:
        //   - Execute      (bit 2)  = execute permission for supervisor-mode
        //   - ExecuteForUserMode    = execute permission for user-mode
        //
        // When MBEC is DISABLED, this bit is IGNORED by hardware and should
        // be kept 0 to avoid compatibility issues on future implementations.
        //
        // Used by Windows HVCI (Hypervisor-Protected Code Integrity) and other
        // VBS (Virtualization-Based Security) features to enforce that only
        // validated kernel pages are executable at the EPT level.
        //
        UINT64 ExecuteForUserMode : 1;

        //
        // [Bit 11] IGNORED
        // Ignored by hardware. Available for software use.
        //
        // See Ignored1 (bit 9) for usage notes.
        //
        UINT64 Ignored2 : 1;

        //
        // [Bits (N-1):12] PHYSICAL ADDRESS of PDPT (Page Frame Number)
        // Holds the host-physical address of the next-level EPT table:
        // the Page Directory Pointer Table (PDPT) for this 512 GB region.
        //
        // The full host-physical address is derived as:
        //   PhysicalAddress << 12   (i.e., 4 KB aligned)
        //
        // The field width is determined by MAXPHYADDR (N), which is the
        // maximum physical address width supported by the CPU. Query it via:
        //   CPUID.80000008H:EAX[7:0]
        // Typical values: 36 (older), 40, 46, or 52 bits.
        //
        // All bits above MAXPHYADDR and below 52 MUST be zero. Violation
        // causes an EPT Misconfiguration VM-exit.
        //
        // The PDPT pointed to by this field must itself be 4 KB aligned and
        // located in host-physical memory accessible to the VMX hardware.
        //
        UINT64 PhysicalAddress : 36;

        //
        // [Bits 51:N] RESERVED (Must Be Zero)
        // These bits represent the upper portion of the physical address field
        // that exceeds MAXPHYADDR on the current CPU.
        //
        // They MUST be zero. If MAXPHYADDR == 52, this field has zero width
        // and is not present. The width here assumes N=48 as a common case;
        // adjust the bitfield split if targeting CPUs with different MAXPHYADDR.
        //
        UINT64 Reserved2 : 4;

        //
        // [Bits 63:52] IGNORED
        // Ignored by hardware. Available for software use.
        //
        // Commonly used by hypervisors to store per-entry software state, such
        // as a lock bit for concurrent table modifications, a "pinned" flag to
        // prevent reclamation, or a generation counter for TLB shootdown logic.
        //
        UINT64 Ignored3 : 12;

    } Fields;

} EPT_PML4E, *PEPT_PML4E;

static_assert(sizeof(EPT_PML4E) == 8, "EPT_PML4E must be 8 bytes");


// ===========================================================================
// EPT Page Directory Pointer Table Entry (PDPTE)
// Intel SDM Vol. 3C, Table 28-2 and Table 28-3
//
// The PDPT is the level-3 table in the EPT hierarchy. Each PDPTE covers a
// 1 GB region of Guest Physical Address (GPA) space. There are 512 entries
// per table, and each PML4E points to one PDPT.
//
// Unlike the PML4E, the PDPTE can be either:
//
//   NON-LEAF (PageSize = 0):
//     Points to the next-level table, an EPT Page Directory (PD).
//     Bits 5:3 and bit 6 are reserved-must-be-zero.
//     PhysicalAddress is 4 KB aligned and points to a PD table.
//
//   LEAF / LARGE PAGE (PageSize = 1):
//     Maps a 1 GB host-physical page directly.
//     Bits 5:3 specify the EPT memory type for the mapped page.
//     Bit 6 controls whether the guest PAT is ignored.
//     Bit 9 becomes the Dirty flag (requires EPT A/D enabled via EPTP bit 6).
//     PhysicalAddress must be 1 GB aligned (bits 29:12 are reserved-must-be-zero).
//
// Both forms share the same bits 2:0 (R/W/X), bit 7 (PageSize), bit 8
// (Accessed), bit 10 (ExecuteForUserMode), and the upper reserved/ignored
// fields. They diverge in bits 6:3 and the PhysicalAddress alignment.
//
// Two distinct structs are defined below and wrapped in a single union to
// allow clean access to either interpretation.
// ===========================================================================


// ---------------------------------------------------------------------------
// NON-LEAF PDPTE — points to an EPT Page Directory (PD)
// Intel SDM Vol. 3C, Table 28-2
// ---------------------------------------------------------------------------
typedef union _EPT_PDPTE_NONLEAF
{
    ULONG64 All;
    struct
    {
        //
        // [Bit 0] READ
        // Allows read access to the 1 GB GPA region covered by this entry.
        //
        // If 0, the entry is NOT PRESENT. Any GPA access in this region
        // causes an EPT Violation VM-exit (exit reason 48), regardless of
        // the Write and Execute bits.
        //
        UINT64 Read : 1;

        //
        // [Bit 1] WRITE
        // Allows write access to the 1 GB GPA region covered by this entry.
        //
        // Only meaningful when Read = 1. If Read = 0, the entry is not
        // present and Write is irrelevant.
        //
        UINT64 Write : 1;

        //
        // [Bit 2] EXECUTE (Supervisor)
        // Allows instruction fetches from the 1 GB GPA region.
        //
        // When MBEC is DISABLED (VMCS Secondary Proc-Based Control bit 22 = 0),
        // this controls execute access for ALL privilege levels.
        //
        // When MBEC is ENABLED, this controls supervisor-mode execute only.
        // User-mode execute is controlled by ExecuteForUserMode (bit 10).
        //
        UINT64 Execute : 1;

        //
        // [Bits 6:3] RESERVED (Must Be Zero)
        // In a non-leaf PDPTE, bits 6:3 are reserved and must be 0.
        //
        // This includes the MemoryType (bits 5:3) and IgnorePAT (bit 6)
        // fields that are present in the leaf form. Those fields have no
        // meaning here since this entry does not map a page directly.
        //
        // Writing non-zero values to these bits causes an EPT Misconfiguration
        // VM-exit (exit reason 49).
        //
        UINT64 Reserved1 : 4;

        //
        // [Bit 7] PAGE SIZE — must be 0 for non-leaf
        // Determines whether this entry is a leaf (1 GB page) or non-leaf.
        //
        // For this struct, PageSize MUST be 0. If set to 1, the CPU treats
        // this as a leaf PDPTE and interprets bits 5:3 as MemoryType, bit 6
        // as IgnorePAT, and PhysicalAddress as a 1 GB-aligned page frame.
        // Use EPT_PDPTE_LEAF for that case.
        //
        UINT64 PageSize : 1;

        //
        // [Bit 8] ACCESSED
        // Set to 1 by the CPU when this entry is used during an EPT page walk.
        //
        // Only active when EPT A/D flags are enabled via EPTP bit 6.
        // When A/D is disabled, this bit is ignored and should be kept 0.
        //
        UINT64 Accessed : 1;

        //
        // [Bit 9] IGNORED
        // Ignored by hardware in non-leaf entries. Available for software use.
        //
        // Note: in the leaf form, this bit becomes the DIRTY flag. Keep this
        // in mind if you ever promote a non-leaf entry to a 1 GB leaf, since
        // any software value stored here would be misinterpreted as Dirty.
        //
        UINT64 Ignored1 : 1;

        //
        // [Bit 10] EXECUTE FOR USER MODE (MBEC)
        // Controls execute access for user-mode code when Mode-Based Execute
        // Control (MBEC) is enabled via VMCS Secondary Proc-Based Controls
        // bit 22.
        //
        // When MBEC is DISABLED, this bit is ignored and should be kept 0.
        // When MBEC is ENABLED:
        //   - Execute (bit 2)       = supervisor-mode execute permission
        //   - ExecuteForUserMode    = user-mode execute permission
        //
        UINT64 ExecuteForUserMode : 1;

        //
        // [Bit 11] IGNORED
        // Ignored by hardware. Available for software use.
        //
        UINT64 Ignored2 : 1;

        //
        // [Bits (N-1):12] HOST-PHYSICAL ADDRESS of EPT Page Directory (PD)
        // Points to the 4 KB-aligned host-physical address of the next-level
        // EPT Page Directory table for this 1 GB region.
        //
        // Requirements:
        //   - Must be 4 KB aligned (bits 11:0 implicitly zero).
        //   - All bits above MAXPHYADDR must be zero, otherwise an EPT
        //     Misconfiguration VM-exit occurs.
        //   - The PD table must remain valid for the lifetime of this entry.
        //
        UINT64 PhysicalAddress : 36;

        //
        // [Bits 51:N] RESERVED (Must Be Zero)
        // Bits above MAXPHYADDR within the physical address field.
        // Must be zero. Width depends on the CPU's MAXPHYADDR value.
        //
        UINT64 Reserved2 : 4;

        //
        // [Bits 63:52] IGNORED
        // Ignored by hardware. Available for software-defined metadata.
        //
        UINT64 Ignored3 : 12;

    } Fields;

} EPT_PDPTE_NONLEAF, *PEPT_PDPTE_NONLEAF;


// ---------------------------------------------------------------------------
// LEAF PDPTE — maps a 1 GB host-physical page directly
// Intel SDM Vol. 3C, Table 28-3
// ---------------------------------------------------------------------------
typedef union _EPT_PDPTE_LEAF
{
    ULONG64 All;
    struct
    {
        //
        // [Bit 0] READ
        // Allows read access to the 1 GB page mapped by this entry.
        // If 0, the entry is NOT PRESENT and causes an EPT Violation VM-exit.
        //
        UINT64 Read : 1;

        //
        // [Bit 1] WRITE
        // Allows write access to the 1 GB page mapped by this entry.
        // Only meaningful when Read = 1.
        //
        UINT64 Write : 1;

        //
        // [Bit 2] EXECUTE (Supervisor)
        // Allows instruction fetches from the 1 GB page.
        // See Execute in EPT_PDPTE_NONLEAF for MBEC interaction.
        //
        UINT64 Execute : 1;

        //
        // [Bits 5:3] EPT MEMORY TYPE
        // Specifies the caching memory type for accesses to the mapped 1 GB
        // host-physical page.
        //
        // Valid values:
        //   0 = Uncacheable (UC) — all accesses bypass caches entirely.
        //                          Use for MMIO or device-mapped regions.
        //   1 = Write Combining (WC) — writes are buffered and combined.
        //                          Useful for framebuffers or DMA buffers.
        //   4 = Write Through (WT) — reads cached, writes go to memory.
        //   5 = Write Protected (WP) — reads cached, writes cause a bus cycle.
        //   6 = Write Back (WB) — fully cached. Use for normal guest RAM.
        //   2, 3, 7 = Reserved — triggers EPT Misconfiguration VM-exit.
        //
        // The effective memory type seen by the guest is determined by
        // combining this field with the guest PAT entry, unless IgnorePAT
        // (bit 6) is set, in which case only this field applies.
        //
        // Query supported memory types via IA32_VMX_EPT_VPID_CAP (0x48C).
        //
        UINT64 MemoryType : 3;

        //
        // [Bit 6] IGNORE PAT
        // Controls whether the guest PAT (Page Attribute Table) MSR is
        // factored into the effective memory type for this page.
        //
        // When 1 (IgnorePAT enabled):
        //   Effective memory type = MemoryType (bits 5:3) only.
        //   The guest PAT entry for this page is completely ignored.
        //
        // When 0 (IgnorePAT disabled):
        //   Effective memory type = combination of MemoryType and guest PAT,
        //   resolved according to the memory type precedence rules in the
        //   Intel SDM Vol. 3A, Section 11.5.2 ("Precedence of Cache Controls").
        //   In practice, the more restrictive type wins (e.g., UC overrides WB).
        //
        // Most hypervisors set IgnorePAT = 1 for simplicity, especially for
        // MMIO regions where UC must be guaranteed regardless of guest PAT.
        //
        UINT64 IgnorePAT : 1;

        //
        // [Bit 7] PAGE SIZE — must be 1 for leaf
        // Marks this entry as a leaf that directly maps a 1 GB page.
        // Must be set to 1 for this struct. If 0, the CPU treats this as a
        // non-leaf entry pointing to a PD. Use EPT_PDPTE_NONLEAF in that case.
        //
        UINT64 PageSize : 1;

        //
        // [Bit 8] ACCESSED
        // Set to 1 by the CPU when this entry is used during an EPT page walk
        // (i.e., any access to the mapped 1 GB GPA range).
        //
        // Only active when EPT A/D flags are enabled via EPTP bit 6.
        // When A/D is disabled, must be kept 0.
        //
        UINT64 Accessed : 1;

        //
        // [Bit 9] DIRTY
        // Set to 1 by the CPU on the first write to any GPA within the
        // mapped 1 GB page.
        //
        // Only active when EPT A/D flags are enabled via EPTP bit 6.
        // When A/D is disabled, this bit is ignored and must be kept 0.
        //
        // Useful for hypervisor dirty tracking (e.g., live migration).
        // The hypervisor can clear this bit and use INVEPT to force the CPU
        // to re-dirty it on the next write, enabling incremental dirty logging
        // without taking EPT Violation VM-exits on every write.
        //
        UINT64 Dirty : 1;

        //
        // [Bit 10] EXECUTE FOR USER MODE (MBEC)
        // Same semantics as in EPT_PDPTE_NONLEAF. Controls user-mode execute
        // permission when MBEC is enabled. Ignored when MBEC is disabled.
        //
        UINT64 ExecuteForUserMode : 1;

        //
        // [Bit 11] IGNORED
        // Ignored by hardware. Available for software use.
        //
        UINT64 Ignored1 : 1;

        //
        // [Bits 29:12] RESERVED (Must Be Zero) — 1 GB alignment requirement
        // A 1 GB page frame must be 1 GB aligned (2^30 bytes), meaning bits
        // 29:0 of the host-physical address are implicitly zero. Bits 29:12
        // therefore fall within the PhysicalAddress field and must be written
        // as 0. A non-zero value here triggers an EPT Misconfiguration VM-exit.
        //
        UINT64 Reserved1 : 18;

        //
        // [Bits (N-1):30] HOST-PHYSICAL ADDRESS of the 1 GB PAGE FRAME
        // The host-physical address of the 1 GB page mapped by this entry.
        //
        // Requirements:
        //   - Must be 1 GB aligned. Bits 29:0 of the address are implicitly 0.
        //     Bits 29:12 within this entry are reserved-must-be-zero (see above).
        //   - All bits above MAXPHYADDR must be zero.
        //   - The page must be valid host-physical memory for the entire
        //     lifetime of this entry.
        //
        // The 1 GB page size is best used for large contiguous guest RAM
        // regions (e.g., identity-mapped GPA = HPA ranges) where using 4 KB
        // or 2 MB mappings would cause excessive TLB pressure. Hypervisors
        // like KVM and Hyper-V promote 512 contiguous 2 MB entries into a
        // single 1 GB entry when possible (large page promotion).
        //
        UINT64 PhysicalAddress : 18;

        //
        // [Bits 51:N] RESERVED (Must Be Zero)
        // Bits above MAXPHYADDR within the physical address field.
        // Must be zero.
        //
        UINT64 Reserved2 : 4;

        //
        // [Bits 63:52] IGNORED
        // Ignored by hardware. Available for software-defined metadata.
        //
        UINT64 Ignored2 : 12;

    } Fields;

} EPT_PDPTE_LEAF, *PEPT_PDPTE_LEAF;


// ---------------------------------------------------------------------------
// EPT_PDPTE — unified union wrapping both leaf and non-leaf forms
//
// Use the PageSize bit to determine which interpretation is active:
//   entry.NonLeaf.Fields.PageSize == 0 → access via entry.NonLeaf.Fields
//   entry.Leaf.Fields.PageSize    == 1 → access via entry.Leaf.Fields
//
// The raw 64-bit value is accessible via entry.All for bulk operations
// such as zeroing, copying, or atomic compare-exchange.
// ---------------------------------------------------------------------------
typedef union _EPT_PDPTE
{
    ULONG64       All;
    EPT_PDPTE_NONLEAF NonLeaf;
    EPT_PDPTE_LEAF    Leaf;

} EPT_PDPTE, *PEPT_PDPTE;

static_assert(sizeof(EPT_PDPTE)          == 8, "EPT_PDPTE must be 8 bytes");
static_assert(sizeof(EPT_PDPTE_NONLEAF)  == 8, "EPT_PDPTE_NONLEAF must be 8 bytes");
static_assert(sizeof(EPT_PDPTE_LEAF)     == 8, "EPT_PDPTE_LEAF must be 8 bytes");

// ===========================================================================
// EPT Page Directory Entry (PDE)
// Intel SDM Vol. 3C, Table 28-4 and Table 28-5
//
// The PD is the level-2 table in the EPT hierarchy. Each PDE covers a
// 2 MB region of Guest Physical Address (GPA) space. There are 512 entries
// per table, and each PDPTE (with PageSize=0) points to one PD.
//
// Like the PDPTE, the PDE can be either:
//
//   NON-LEAF (PageSize = 0):
//     Points to the next-level table, an EPT Page Table (PT).
//     Bits 6:3 are reserved-must-be-zero.
//     PhysicalAddress is 4 KB aligned and points to a PT table.
//
//   LEAF / LARGE PAGE (PageSize = 1):
//     Maps a 2 MB host-physical page directly.
//     Bits 5:3 specify the EPT memory type for the mapped page.
//     Bit 6 controls whether the guest PAT is ignored.
//     Bit 9 becomes the Dirty flag (requires EPT A/D enabled via EPTP bit 6).
//     PhysicalAddress must be 2 MB aligned (bits 20:12 are reserved-must-be-zero).
//     Bits 58:52 carry the same advanced per-page controls as the PTE
//     (VerifyGuestPaging, PagingWrite, SPP, SuppressVE).
//
// Both forms share bits 2:0 (R/W/X), bit 7 (PageSize), bit 8 (Accessed),
// bit 10 (ExecuteForUserMode), and the upper reserved/ignored fields.
// They diverge in bits 6:3 and the PhysicalAddress alignment.
//
// Two distinct structs are defined below and wrapped in a single union to
// allow clean access to either interpretation.
// ===========================================================================


// ---------------------------------------------------------------------------
// NON-LEAF PDE — points to an EPT Page Table (PT)
// Intel SDM Vol. 3C, Table 28-4
// ---------------------------------------------------------------------------
typedef union _EPT_PDE_NONLEAF
{
    ULONG64 All;
    struct
    {
        //
        // [Bit 0] READ
        // Allows read access to the 2 MB GPA region covered by this entry.
        //
        // If 0, the entry is NOT PRESENT. Any GPA access in this region
        // causes an EPT Violation VM-exit (exit reason 48), regardless of
        // the Write and Execute bits.
        //
        UINT64 Read : 1;

        //
        // [Bit 1] WRITE
        // Allows write access to the 2 MB GPA region covered by this entry.
        // Only meaningful when Read = 1.
        //
        UINT64 Write : 1;

        //
        // [Bit 2] EXECUTE (Supervisor)
        // Allows instruction fetches from the 2 MB GPA region.
        //
        // When MBEC is DISABLED (VMCS Secondary Proc-Based Controls bit 22 = 0),
        // this controls execute access for ALL privilege levels.
        //
        // When MBEC is ENABLED, this controls supervisor-mode execute only.
        // User-mode execute is controlled by ExecuteForUserMode (bit 10).
        //
        UINT64 Execute : 1;

        //
        // [Bits 6:3] RESERVED (Must Be Zero)
        // In a non-leaf PDE, bits 6:3 are reserved and must be 0.
        //
        // This includes the MemoryType (bits 5:3) and IgnorePAT (bit 6)
        // fields that are present in the leaf form. Those fields have no
        // meaning here since this entry does not map a page directly.
        //
        // Writing non-zero values to these bits causes an EPT Misconfiguration
        // VM-exit (exit reason 49).
        //
        UINT64 Reserved1 : 4;

        //
        // [Bit 7] PAGE SIZE — must be 0 for non-leaf
        // Determines whether this entry is a leaf (2 MB page) or non-leaf.
        //
        // For this struct, PageSize MUST be 0. If set to 1, the CPU treats
        // this as a leaf PDE and interprets bits 5:3 as MemoryType, bit 6
        // as IgnorePAT, and PhysicalAddress as a 2 MB-aligned page frame.
        // Use EPT_PDE_LEAF for that case.
        //
        UINT64 PageSize : 1;

        //
        // [Bit 8] ACCESSED
        // Set to 1 by the CPU when this entry is used during an EPT page walk.
        //
        // Only active when EPT A/D flags are enabled via EPTP bit 6.
        // When A/D is disabled, this bit is ignored and should be kept 0.
        //
        UINT64 Accessed : 1;

        //
        // [Bit 9] IGNORED
        // Ignored by hardware in non-leaf entries. Available for software use.
        //
        // Note: in the leaf form, this bit becomes the DIRTY flag. Keep this
        // in mind if you ever promote a non-leaf entry to a 2 MB leaf, since
        // any software value stored here would be misinterpreted as Dirty.
        //
        UINT64 Ignored1 : 1;

        //
        // [Bit 10] EXECUTE FOR USER MODE (MBEC)
        // Controls execute access for user-mode code when Mode-Based Execute
        // Control (MBEC) is enabled via VMCS Secondary Proc-Based Controls
        // bit 22. Ignored when MBEC is disabled.
        //
        // When MBEC is ENABLED:
        //   - Execute (bit 2)    = supervisor-mode execute permission
        //   - ExecuteForUserMode = user-mode execute permission
        //
        UINT64 ExecuteForUserMode : 1;

        //
        // [Bit 11] IGNORED
        // Ignored by hardware. Available for software use.
        //
        UINT64 Ignored2 : 1;

        //
        // [Bits (N-1):12] HOST-PHYSICAL ADDRESS of EPT Page Table (PT)
        // Points to the 4 KB-aligned host-physical address of the next-level
        // EPT Page Table for this 2 MB region.
        //
        // Requirements:
        //   - Must be 4 KB aligned (bits 11:0 implicitly zero).
        //   - All bits above MAXPHYADDR must be zero, otherwise an EPT
        //     Misconfiguration VM-exit occurs.
        //   - The PT must remain valid for the lifetime of this entry.
        //
        UINT64 PhysicalAddress : 36;

        //
        // [Bits 51:N] RESERVED (Must Be Zero)
        // Bits above MAXPHYADDR within the physical address field.
        // Must be zero. Width depends on the CPU's MAXPHYADDR value.
        //
        UINT64 Reserved2 : 4;

        //
        // [Bits 63:52] IGNORED
        // Ignored by hardware. Available for software-defined metadata.
        //
        UINT64 Ignored3 : 12;

    } Fields;

} EPT_PDE_NONLEAF, *PEPT_PDE_NONLEAF;


// ---------------------------------------------------------------------------
// LEAF PDE — maps a 2 MB host-physical page directly
// Intel SDM Vol. 3C, Table 28-5
// ---------------------------------------------------------------------------
typedef union _EPT_PDE_LEAF
{
    ULONG64 All;
    struct
    {
        //
        // [Bit 0] READ
        // Allows read access to the 2 MB page mapped by this entry.
        // If 0, the entry is NOT PRESENT and causes an EPT Violation VM-exit.
        //
        UINT64 Read : 1;

        //
        // [Bit 1] WRITE
        // Allows write access to the 2 MB page mapped by this entry.
        // Only meaningful when Read = 1.
        //
        UINT64 Write : 1;

        //
        // [Bit 2] EXECUTE (Supervisor)
        // Allows instruction fetches from the 2 MB page.
        // See Execute in EPT_PDE_NONLEAF for MBEC interaction.
        //
        UINT64 Execute : 1;

        //
        // [Bits 5:3] EPT MEMORY TYPE
        // Specifies the caching memory type for host-physical accesses to
        // the mapped 2 MB page.
        //
        // Valid values:
        //   0 = Uncacheable (UC) — all accesses bypass caches entirely.
        //                          Use for MMIO or device-mapped regions.
        //   1 = Write Combining (WC) — writes buffered and merged before
        //                          flush. Suitable for framebuffers or
        //                          streaming write workloads.
        //   4 = Write Through (WT) — reads cached, writes go to memory.
        //   5 = Write Protected (WP) — reads cached, writes bypass cache.
        //   6 = Write Back (WB)   — fully cached. Use for normal guest RAM.
        //   2, 3, 7 = Reserved    — triggers EPT Misconfiguration VM-exit.
        //
        // The effective memory type is the combination of this field and
        // the guest PAT entry, unless IgnorePAT (bit 6) is set.
        // Query supported memory types via IA32_VMX_EPT_VPID_CAP (0x48C).
        //
        UINT64 MemoryType : 3;

        //
        // [Bit 6] IGNORE PAT
        // Controls whether the guest PAT MSR is factored into the effective
        // memory type for this 2 MB page.
        //
        // When 1 (IgnorePAT enabled):
        //   Effective memory type = MemoryType (bits 5:3) only.
        //   Guest PAT entry for this GPA is completely ignored.
        //
        // When 0 (IgnorePAT disabled):
        //   Effective memory type = combination of MemoryType and guest PAT,
        //   resolved per Intel SDM Vol. 3A Section 11.5.2.
        //
        UINT64 IgnorePAT : 1;

        //
        // [Bit 7] PAGE SIZE — must be 1 for leaf
        // Marks this entry as a leaf that directly maps a 2 MB page.
        // Must be set to 1 for this struct. If 0, the CPU treats this as a
        // non-leaf entry pointing to a PT. Use EPT_PDE_NONLEAF in that case.
        //
        UINT64 PageSize : 1;

        //
        // [Bit 8] ACCESSED
        // Set to 1 by the CPU when this entry is used during an EPT page walk
        // (i.e., any access to the mapped 2 MB GPA range).
        //
        // Only active when EPT A/D flags are enabled via EPTP bit 6.
        // When A/D is disabled, must be kept 0.
        //
        UINT64 Accessed : 1;

        //
        // [Bit 9] DIRTY
        // Set to 1 by the CPU on the first write to any GPA within the
        // mapped 2 MB page.
        //
        // Only active when EPT A/D flags are enabled via EPTP bit 6.
        // When A/D is disabled, this bit is ignored and must be kept 0.
        //
        // After clearing Dirty, execute INVEPT to invalidate cached
        // translations so the CPU re-sets the bit on the next write.
        //
        UINT64 Dirty : 1;

        //
        // [Bit 10] EXECUTE FOR USER MODE (MBEC)
        // Same semantics as in EPT_PDE_NONLEAF. Controls user-mode execute
        // permission when MBEC is enabled. Ignored when MBEC is disabled.
        //
        UINT64 ExecuteForUserMode : 1;

        //
        // [Bit 11] IGNORED
        // Ignored by hardware. Available for software use.
        //
        UINT64 Ignored1 : 1;

        //
        // [Bits 20:12] RESERVED (Must Be Zero) — 2 MB alignment requirement
        // A 2 MB page frame must be 2 MB aligned (2^21 bytes), meaning bits
        // 20:0 of the host-physical address are implicitly zero. Bits 20:12
        // therefore fall within the PhysicalAddress field and must be written
        // as 0. A non-zero value here triggers an EPT Misconfiguration VM-exit.
        //
        // This is the key structural difference between the PDE leaf and the
        // PDPTE leaf: the PDPTE leaf reserves bits 29:12 (18 bits) for 1 GB
        // alignment, while the PDE leaf reserves only bits 20:12 (9 bits)
        // for 2 MB alignment.
        //
        UINT64 Reserved1 : 9;

        //
        // [Bits (N-1):21] HOST-PHYSICAL ADDRESS of the 2 MB PAGE FRAME
        // The host-physical address of the 2 MB page mapped by this entry.
        //
        // Requirements:
        //   - Must be 2 MB aligned. Bits 20:0 of the address are implicitly 0.
        //     Bits 20:12 within this entry are reserved-must-be-zero (above).
        //   - All bits above MAXPHYADDR must be zero.
        //   - The page must remain valid host-physical memory for the entire
        //     lifetime of this entry.
        //
        // 2 MB EPT mappings are the most commonly used large page size in
        // hypervisors. They offer a good balance between TLB coverage and
        // mapping granularity, and are the primary target for large page
        // promotion (collapsing 512 contiguous 4 KB PTEs into one 2 MB PDE).
        //
        UINT64 PhysicalAddress : 27;

        //
        // [Bits 51:N] RESERVED (Must Be Zero)
        // Bits above MAXPHYADDR within the physical address field.
        // Must be zero. Width depends on the CPU's MAXPHYADDR value,
        // queried via CPUID.80000008H:EAX[7:0].
        //
        UINT64 Reserved2 : 4;

        //
        // [Bits 57:52] IGNORED
        // Ignored by hardware. Available for software use.
        //
        UINT64 Ignored2 : 6;

        //
        // [Bit 58] VERIFY GUEST PAGING
        // When set, the CPU verifies that the guest paging structures used
        // to translate this GPA are consistent with EPT permissions.
        //
        // Part of the Guest-Paging Verification (GPV) feature. Leave as 0
        // unless implementing advanced nested paging consistency checks.
        // See EPT_PTE for full documentation on this field.
        //
        UINT64 VerifyGuestPaging : 1;

        //
        // [Bit 59] PAGING WRITE
        // Indicates that this page may be written by the CPU as part of
        // guest page table walks (to set Accessed/Dirty bits in guest paging
        // structures). Allows hypervisors to distinguish guest page table
        // writes from normal data writes when using write-protected EPT
        // mappings for guest page table tracking.
        //
        // Part of the Guest-Paging Verification feature. Leave as 0 unless
        // explicitly implementing guest page table write interception.
        // See EPT_PTE for full documentation on this field.
        //
        UINT64 PagingWrite : 1;

        //
        // [Bit 60] IGNORED
        // Ignored by hardware. Available for software use.
        //
        UINT64 Ignored3 : 1;

        //
        // [Bit 61] SUB-PAGE WRITE PERMISSIONS (SPP)
        // When set, write permissions for 128-byte sub-regions within this
        // 2 MB page are controlled via the Sub-Page Permission Table (SPPT).
        //
        // Note: SPP at the 2 MB level means the SPPT must cover all 128-byte
        // sub-regions within the entire 2 MB range, which is a significant
        // SPPT memory overhead compared to using SPP at the 4 KB PTE level.
        // In practice, SPP is more commonly applied at the PTE level after
        // splitting a 2 MB large page into 512 x 4 KB PTEs.
        //
        // Requires CPU support (IA32_VMX_EPT_VPID_CAP bit 37) and VMCS
        // Secondary Proc-Based Controls bit 23.
        // See EPT_PTE for full documentation on this field.
        //
        UINT64 SubPageWritePermissions : 1;

        //
        // [Bit 62] IGNORED
        // Ignored by hardware. Available for software use.
        //
        UINT64 Ignored4 : 1;

        //
        // [Bit 63] SUPPRESS #VE (Virtualization Exception)
        // When set, EPT violations on this 2 MB page do NOT generate a
        // Virtualization Exception (#VE) in the guest, even when the
        // "#VE on EPT violations" VMCS control is active. Instead they
        // cause a normal EPT Violation VM-exit (exit reason 48).
        //
        // Requires CPU support (IA32_VMX_EPT_VPID_CAP bit 32) and VMCS
        // Secondary Proc-Based Controls bit 18.
        // See EPT_PTE for full documentation on this field.
        //
        UINT64 SuppressVE : 1;

    } Fields;

} EPT_PDE_LEAF, *PEPT_PDE_LEAF;


// ---------------------------------------------------------------------------
// EPT_PDE — unified union wrapping both leaf and non-leaf forms
//
// Use the PageSize bit to determine which interpretation is active:
//   entry.NonLeaf.Fields.PageSize == 0 → access via entry.NonLeaf.Fields
//   entry.Leaf.Fields.PageSize    == 1 → access via entry.Leaf.Fields
//
// The raw 64-bit value is accessible via entry.All for bulk operations
// such as zeroing, copying, or atomic compare-exchange.
// ---------------------------------------------------------------------------
typedef union _EPT_PDE
{
    ULONG64        All;
    EPT_PDE_NONLEAF NonLeaf;
    EPT_PDE_LEAF    Leaf;

} EPT_PDE, *PEPT_PDE;

static_assert(sizeof(EPT_PDE)          == 8, "EPT_PDE must be 8 bytes");
static_assert(sizeof(EPT_PDE_NONLEAF)  == 8, "EPT_PDE_NONLEAF must be 8 bytes");
static_assert(sizeof(EPT_PDE_LEAF)     == 8, "EPT_PDE_LEAF must be 8 bytes");


// ===========================================================================
// EPT Page Table Entry (PTE)
// Intel SDM Vol. 3C, Table 28-6
//
// The PT is the level-1 (innermost) table in the EPT hierarchy. Each PTE
// maps a single 4 KB guest-physical page to a 4 KB host-physical page frame.
// There are 512 entries per table, and each PDE (with PageSize=0) points to
// one PT.
//
// The PTE is ALWAYS a leaf entry. There is no PageSize bit and no non-leaf
// variant at this level. Every valid PTE with Read=1 directly maps a 4 KB
// host-physical page.
//
// As a leaf entry, the PTE carries the full set of per-page attributes:
//   - MemoryType (bits 5:3): cache behavior for the mapped page
//   - IgnorePAT  (bit 6):    whether guest PAT is factored in
//   - Accessed   (bit 8):    set by CPU on any access (requires EPT A/D)
//   - Dirty      (bit 9):    set by CPU on first write (requires EPT A/D)
//   - SPP        (bit 61):   sub-page write permissions (requires SPP support)
//   - SVE        (bit 63):   suppress #VE for this page (requires #VE support)
//
// The PTE is the most frequently accessed EPT structure at runtime. On a
// cold 4-level EPT walk the CPU performs up to 20 memory accesses:
//   4 (PML4) + 4 (PDPT) + 4 (PD) + 4 (PT) + 1 (final GPA access) = ~20
// TLB and EPT caching reduce this to near-zero in steady state.
// ===========================================================================
typedef union _EPT_PTE
{
    ULONG64 All;
    struct
    {
        //
        // [Bit 0] READ
        // Allows read access to the 4 KB page mapped by this entry.
        //
        // If 0, the entry is NOT PRESENT. Any access to the mapped GPA
        // causes an EPT Violation VM-exit (exit reason 48), regardless of
        // the Write and Execute bits.
        //
        // This is the presence bit at the PTE level. The CPU checks it first
        // before evaluating any other field.
        //
        UINT64 Read : 1;

        //
        // [Bit 1] WRITE
        // Allows write access to the 4 KB page mapped by this entry.
        //
        // Only meaningful when Read = 1. A write to a GPA whose PTE has
        // Write = 0 causes an EPT Violation VM-exit.
        //
        // Common use cases for Write = 0:
        //   - Copy-on-write (CoW) page sharing between VMs or vCPUs.
        //   - Dirty tracking: mark clean pages as read-only, promote to
        //     read-write on the first write EPT violation.
        //   - IOMMU / DMA protection: prevent guest DMA from writing to
        //     pages that are currently being inspected by the hypervisor.
        //
        UINT64 Write : 1;

        //
        // [Bit 2] EXECUTE (Supervisor)
        // Allows instruction fetches from the 4 KB page.
        //
        // When MBEC is DISABLED (VMCS Secondary Proc-Based Controls bit 22 = 0),
        // this controls execute access for ALL privilege levels (user and
        // supervisor).
        //
        // When MBEC is ENABLED, this controls supervisor-mode execute only.
        // User-mode execute is controlled separately by ExecuteForUserMode
        // (bit 10). This split allows the hypervisor to enforce that guest
        // kernel pages are executable only in kernel mode, enabling
        // hypervisor-level SMEP enforcement independent of guest CR4.SMEP.
        //
        UINT64 Execute : 1;

        //
        // [Bits 5:3] EPT MEMORY TYPE
        // Specifies the caching memory type for host-physical accesses to
        // the mapped 4 KB page.
        //
        // Valid values:
        //   0 = Uncacheable (UC) — all accesses bypass caches.
        //                          Mandatory for MMIO or device memory regions.
        //                          Prevents speculative reads from device
        //                          registers that have read side-effects.
        //   1 = Write Combining (WC) — writes buffered and merged before
        //                          being flushed to memory. Suitable for
        //                          framebuffers and streaming write workloads
        //                          where cache coherency is not required.
        //   4 = Write Through (WT) — reads served from cache, writes go to
        //                          both cache and memory simultaneously.
        //   5 = Write Protected (WP) — reads cached, writes bypass cache and
        //                          trigger a bus write cycle. Rarely used.
        //   6 = Write Back (WB)   — fully cached, writeback on eviction.
        //                          Use for all normal guest RAM pages.
        //   2, 3, 7 = Reserved    — triggers EPT Misconfiguration VM-exit
        //                          (exit reason 49). Never use these values.
        //
        // The effective memory type seen by the guest is the combination of
        // this field and the guest PAT entry, resolved per Intel SDM Vol. 3A
        // Section 11.5.2, unless IgnorePAT (bit 6) overrides this.
        // In general, the more restrictive type takes precedence (UC wins).
        //
        // Query supported memory types via IA32_VMX_EPT_VPID_CAP (0x48C)
        // before using WC or other non-WB/UC types.
        //
        UINT64 MemoryType : 3;

        //
        // [Bit 6] IGNORE PAT
        // Controls whether the guest PAT (Page Attribute Table) MSR is
        // factored into the effective memory type for this page.
        //
        // When 1 (IgnorePAT enabled):
        //   Effective memory type = MemoryType (bits 5:3) only.
        //   The guest PAT entry for this GPA is completely ignored.
        //   Use this when you need to guarantee a specific memory type
        //   regardless of how the guest has configured its PAT MSR.
        //
        // When 0 (IgnorePAT disabled):
        //   Effective memory type = combination of MemoryType and the guest
        //   PAT entry, resolved per Intel SDM memory type precedence rules.
        //
        // Most hypervisors set IgnorePAT = 1 for all MMIO pages to guarantee
        // UC, and IgnorePAT = 0 (or 1 with WB) for normal RAM pages to allow
        // the guest to control caching behavior via its own PAT.
        //
        UINT64 IgnorePAT : 1;

        //
        // [Bit 7] RESERVED (Must Be Zero)
        // At the PTE level there is no PageSize bit — the PTE is always a
        // leaf. Bit 7 is therefore reserved and must be written as 0.
        //
        // Writing 1 here causes an EPT Misconfiguration VM-exit.
        //
        UINT64 Reserved1 : 1;

        //
        // [Bit 8] ACCESSED
        // Set to 1 by the CPU when this PTE is used during an EPT page walk,
        // meaning the corresponding 4 KB GPA has been accessed (read, write,
        // or fetch).
        //
        // Only active when EPT A/D flags are enabled via EPTP bit 6. When
        // A/D is disabled, this bit is ignored by hardware and should be 0.
        //
        // The hypervisor can clear this bit to detect subsequent accesses,
        // useful for working set estimation, memory pressure balancing, or
        // determining which guest pages are hot vs. cold.
        // After clearing, execute INVEPT to ensure the CPU re-walks the EPT
        // and re-sets the bit on next access rather than using a cached entry.
        //
        UINT64 Accessed : 1;

        //
        // [Bit 9] DIRTY
        // Set to 1 by the CPU on the first write to the mapped 4 KB page.
        //
        // Only active when EPT A/D flags are enabled via EPTP bit 6. When
        // A/D is disabled, this bit is ignored and must be kept 0.
        //
        // Primary use cases:
        //   - Live migration: enumerate dirty pages since the last iteration
        //     by scanning PTEs with Dirty = 1 and clearing them between rounds.
        //     Page Modification Logging (PML) automates this at scale.
        //   - Copy-on-write: detect the first write to a shared page and
        //     trigger a copy before allowing the write to proceed.
        //   - Checkpoint/restore: identify pages modified since the last
        //     memory snapshot.
        //
        // After clearing Dirty, execute INVEPT to invalidate cached
        // translations so the CPU re-sets the bit on the next write.
        //
        UINT64 Dirty : 1;

        //
        // [Bit 10] EXECUTE FOR USER MODE (MBEC)
        // Controls execute access for user-mode code when Mode-Based Execute
        // Control (MBEC) is enabled via VMCS Secondary Proc-Based Controls
        // bit 22. Ignored when MBEC is disabled.
        //
        // When MBEC is ENABLED:
        //   - Execute (bit 2)    = supervisor-mode execute permission
        //   - ExecuteForUserMode = user-mode execute permission
        //
        // This is the primary mechanism used by:
        //   - Windows HVCI (Hypervisor-Protected Code Integrity): ensures
        //     only validated kernel pages are executable in supervisor mode.
        //   - Hypervisor-enforced W^X (Write XOR Execute): prevent any page
        //     from being simultaneously writable and executable, independent
        //     of what the guest OS does with its own page tables.
        //
        UINT64 ExecuteForUserMode : 1;

        //
        // [Bit 11] IGNORED
        // Ignored by hardware. Available for software-defined metadata.
        //
        // Hypervisors commonly use this bit to mark pages with special
        // handling, e.g., "this page belongs to a hypervisor structure and
        // must not be remapped or freed by guest-driven operations."
        //
        UINT64 Ignored1 : 1;

        //
        // [Bits (N-1):12] HOST-PHYSICAL ADDRESS of the 4 KB PAGE FRAME
        // The 4 KB-aligned host-physical address of the page mapped by
        // this entry.
        //
        // Requirements:
        //   - Must be 4 KB aligned (bits 11:0 of the address are implicitly 0,
        //     covered by the fields above).
        //   - All bits above MAXPHYADDR must be zero. Violation causes an EPT
        //     Misconfiguration VM-exit.
        //   - The page must remain valid host-physical memory for the entire
        //     lifetime of this PTE. Freeing or remapping the backing page
        //     while the PTE is live causes silent memory corruption or a guest
        //     security vulnerability.
        //
        // To obtain the PFN from a virtual address on Windows:
        //   MmGetPhysicalAddress(va).QuadPart >> PAGE_SHIFT
        // On Linux:
        //   virt_to_phys(va) >> PAGE_SHIFT
        //
        UINT64 PhysicalAddress : 36;

        //
        // [Bits 51:N] RESERVED (Must Be Zero)
        // Bits above MAXPHYADDR within the physical address field.
        // Must be zero. Width depends on the CPU's MAXPHYADDR value,
        // queried via CPUID.80000008H:EAX[7:0].
        //
        UINT64 Reserved2 : 4;

        //
        // [Bits 57:52] IGNORED
        // Ignored by hardware. Available for software use.
        //
        UINT64 Ignored2 : 6;

        //
        // [Bit 58] VERIFY GUEST PAGING
        // When set, the CPU verifies that the guest paging structures used
        // to translate this GPA are consistent with EPT permissions.
        //
        // This is part of the Guest-Paging Verification (GPV) feature,
        // available when "EPT-based sub-page write permissions" is supported.
        // Consult IA32_VMX_EPT_VPID_CAP and the VMCS EPT-violation
        // #VE information area for details.
        //
        // Most hypervisors leave this as 0 unless implementing advanced
        // nested paging consistency checks.
        //
        UINT64 VerifyGuestPaging : 1;

        //
        // [Bit 59] PAGING WRITE
        // When set, indicates that this page may be written by the CPU as
        // part of guest page table walks (i.e., to set Accessed/Dirty bits
        // in guest paging structures). This allows the hypervisor to
        // distinguish guest page table writes from normal data writes when
        // using write-protected EPT mappings for guest page table tracking.
        //
        // Part of the Guest-Paging Verification feature. Leave as 0 unless
        // explicitly implementing guest page table write interception.
        //
        UINT64 PagingWrite : 1;

        //
        // [Bit 60] IGNORED
        // Ignored by hardware. Available for software use.
        //
        UINT64 Ignored3 : 1;

        //
        // [Bit 61] SUB-PAGE WRITE PERMISSIONS (SPP)
        // When set to 1, write permissions for this 4 KB page are controlled
        // at 128-byte sub-page granularity via the Sub-Page Permission Table
        // (SPPT), rather than by the Write bit (bit 1) of this PTE.
        //
        // This allows the hypervisor to protect individual 128-byte regions
        // within a 4 KB page — for example, to intercept writes to specific
        // fields within a guest data structure without trapping on the entire
        // page.
        //
        // Requirements:
        //   - CPU must support SPP: check IA32_VMX_EPT_VPID_CAP bit 37.
        //   - "Sub-page write permissions for EPT" must be enabled in the
        //     VMCS Secondary Proc-Based VM-Execution Controls (bit 23).
        //   - The SPPT must be set up and its pointer written to the VMCS
        //     SPPTP field before enabling this bit on any PTE.
        //   - The Write bit (bit 1) should be 1 when SPP is active;
        //     SPP overrides write permission at sub-page granularity.
        //
        // When this bit is 0, write permissions are governed normally by
        // the Write bit (bit 1) and this field is ignored.
        //
        UINT64 SubPageWritePermissions : 1;

        //
        // [Bit 62] IGNORED
        // Ignored by hardware. Available for software use.
        //
        UINT64 Ignored4 : 1;

        //
        // [Bit 63] SUPPRESS #VE (Virtualization Exception)
        // When set to 1, EPT violations on this page do NOT generate a
        // Virtualization Exception (#VE, vector 20) in the guest, even when
        // the "#VE on EPT violations" VMCS control is active.
        // Instead, they cause a normal EPT Violation VM-exit (exit reason 48).
        //
        // When set to 0 and the #VE VMCS control is enabled, EPT violations
        // on this page inject a #VE directly into the guest without a VM-exit,
        // allowing the guest itself (or a paravirtual shim) to handle the fault.
        //
        // Requirements:
        //   - CPU must support #VE: check IA32_VMX_EPT_VPID_CAP bit 32.
        //   - "EPT-violation #VE" must be enabled in VMCS Secondary
        //     Proc-Based VM-Execution Controls (bit 18).
        //   - The #VE information area must be configured in the VMCS.
        //
        // Common use case: set SVE = 0 on pages where the hypervisor wants
        // the guest to handle its own EPT faults (e.g., in a nested
        // virtualization or introspection scenario), and SVE = 1 on
        // sensitive pages where the hypervisor must always intercept faults.
        //
        UINT64 SuppressVE : 1;

    } Fields;

} EPT_PTE, *PEPT_PTE;

static_assert(sizeof(EPT_PTE) == 8, "EPT_PTE must be 8 bytes");
