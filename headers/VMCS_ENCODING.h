// Based on Intel 64 and IA-32 Architectures SDM - Appendix B (Field Encoding in VMCS)
typedef enum _VMCS_ENCODING {
	VIRTUAL_PROCESSOR_ID		= 0x00000000,
	POSTED_INTR_NV			= 0x00000002,
	GUEST_ES_SELECTOR		= 0x00000800,
	GUEST_CS_SELECTOR		= 0x00000802,
	GUEST_SS_SELECTOR		= 0x00000804,
	GUEST_DS_SELECTOR		= 0x00000806,
	GUEST_FS_SELECTOR		= 0x00000808,
	GUEST_GS_SELECTOR		= 0x0000080a,
	GUEST_LDTR_SELECTOR		= 0x0000080c,
	GUEST_TR_SELECTOR		= 0x0000080e,
	GUEST_INTR_STATUS		= 0x00000810,
	GUEST_PML_INDEX			= 0x00000812,
	HOST_ES_SELECTOR		= 0x00000c00,
	HOST_CS_SELECTOR		= 0x00000c02,
	HOST_SS_SELECTOR		= 0x00000c04,
	HOST_DS_SELECTOR		= 0x00000c06,
	HOST_FS_SELECTOR		= 0x00000c08,
	HOST_GS_SELECTOR		= 0x00000c0a,
	HOST_TR_SELECTOR		= 0x00000c0c,
	IO_BITMAP_A			= 0x00002000,
	IO_BITMAP_A_HIGH		= 0x00002001,
	IO_BITMAP_B			= 0x00002002,
	IO_BITMAP_B_HIGH		= 0x00002003,
	MSR_BITMAP			= 0x00002004,
	MSR_BITMAP_HIGH			= 0x00002005,
	VM_EXIT_MSR_STORE_ADDR		= 0x00002006,
	VM_EXIT_MSR_STORE_ADDR_HIGH	= 0x00002007,
	VM_EXIT_MSR_LOAD_ADDR		= 0x00002008,
	VM_EXIT_MSR_LOAD_ADDR_HIGH	= 0x00002009,
	VM_ENTRY_MSR_LOAD_ADDR		= 0x0000200a,
	VM_ENTRY_MSR_LOAD_ADDR_HIGH	= 0x0000200b,
	PML_ADDRESS			= 0x0000200e,
	PML_ADDRESS_HIGH		= 0x0000200f,
	TSC_OFFSET			= 0x00002010,
	TSC_OFFSET_HIGH			= 0x00002011,
	VIRTUAL_APIC_PAGE_ADDR		= 0x00002012,
	VIRTUAL_APIC_PAGE_ADDR_HIGH	= 0x00002013,
	APIC_ACCESS_ADDR		= 0x00002014,
	APIC_ACCESS_ADDR_HIGH		= 0x00002015,
	POSTED_INTR_DESC_ADDR		= 0x00002016,
	POSTED_INTR_DESC_ADDR_HIGH	= 0x00002017,
	EPT_POINTER			= 0x0000201a,
	EPT_POINTER_HIGH		= 0x0000201b,
	EOI_EXIT_BITMAP0		= 0x0000201c,
	EOI_EXIT_BITMAP0_HIGH		= 0x0000201d,
	EOI_EXIT_BITMAP1		= 0x0000201e,
	EOI_EXIT_BITMAP1_HIGH		= 0x0000201f,
	EOI_EXIT_BITMAP2		= 0x00002020,
	EOI_EXIT_BITMAP2_HIGH		= 0x00002021,
	EOI_EXIT_BITMAP3		= 0x00002022,
	EOI_EXIT_BITMAP3_HIGH		= 0x00002023,
	VMREAD_BITMAP			= 0x00002026,
	VMREAD_BITMAP_HIGH		= 0x00002027,
	VMWRITE_BITMAP			= 0x00002028,
	VMWRITE_BITMAP_HIGH		= 0x00002029,
	XSS_EXIT_BITMAP			= 0x0000202C,
	XSS_EXIT_BITMAP_HIGH		= 0x0000202D,
	ENCLS_EXITING_BITMAP		= 0x0000202E,
	ENCLS_EXITING_BITMAP_HIGH	= 0x0000202F,
	TSC_MULTIPLIER			= 0x00002032,
	TSC_MULTIPLIER_HIGH		= 0x00002033,
	GUEST_PHYSICAL_ADDRESS		= 0x00002400,
	GUEST_PHYSICAL_ADDRESS_HIGH	= 0x00002401,
	VMCS_LINK_POINTER		= 0x00002800,
	VMCS_LINK_POINTER_HIGH		= 0x00002801,
	GUEST_IA32_DEBUGCTL		= 0x00002802,
	GUEST_IA32_DEBUGCTL_HIGH	= 0x00002803,
	GUEST_IA32_PAT			= 0x00002804,
	GUEST_IA32_PAT_HIGH		= 0x00002805,
	GUEST_IA32_EFER			= 0x00002806,
	GUEST_IA32_EFER_HIGH		= 0x00002807,
	GUEST_IA32_PERF_GLOBAL_CTRL	= 0x00002808,
	GUEST_IA32_PERF_GLOBAL_CTRL_HIGH= 0x00002809,
	GUEST_PDPTR0			= 0x0000280a,
	GUEST_PDPTR0_HIGH		= 0x0000280b,
	GUEST_PDPTR1			= 0x0000280c,
	GUEST_PDPTR1_HIGH		= 0x0000280d,
	GUEST_PDPTR2			= 0x0000280e,
	GUEST_PDPTR2_HIGH		= 0x0000280f,
	GUEST_PDPTR3			= 0x00002810,
	GUEST_PDPTR3_HIGH		= 0x00002811,
	GUEST_BNDCFGS			= 0x00002812,
	GUEST_BNDCFGS_HIGH		= 0x00002813,
	HOST_IA32_PAT			= 0x00002c00,
	HOST_IA32_PAT_HIGH		= 0x00002c01,
	HOST_IA32_EFER			= 0x00002c02,
	HOST_IA32_EFER_HIGH		= 0x00002c03,
	HOST_IA32_PERF_GLOBAL_CTRL	= 0x00002c04,
	HOST_IA32_PERF_GLOBAL_CTRL_HIGH	= 0x00002c05,
	PIN_BASED_VM_EXEC_CONTROL	= 0x00004000,
	CPU_BASED_VM_EXEC_CONTROL	= 0x00004002,
	EXCEPTION_BITMAP		= 0x00004004,
	PAGE_FAULT_ERROR_CODE_MASK	= 0x00004006,
	PAGE_FAULT_ERROR_CODE_MATCH	= 0x00004008,
	CR3_TARGET_COUNT		= 0x0000400a,
	VM_EXIT_CONTROLS		= 0x0000400c,
	VM_EXIT_MSR_STORE_COUNT		= 0x0000400e,
	VM_EXIT_MSR_LOAD_COUNT		= 0x00004010,
	VM_ENTRY_CONTROLS		= 0x00004012,
	VM_ENTRY_MSR_LOAD_COUNT		= 0x00004014,
	VM_ENTRY_INTR_INFO_FIELD	= 0x00004016,
	VM_ENTRY_EXCEPTION_ERROR_CODE	= 0x00004018,
	VM_ENTRY_INSTRUCTION_LEN	= 0x0000401a,
	TPR_THRESHOLD			= 0x0000401c,
	SECONDARY_VM_EXEC_CONTROL	= 0x0000401e,
	PLE_GAP				= 0x00004020,
	PLE_WINDOW			= 0x00004022,
	VM_INSTRUCTION_ERROR		= 0x00004400,
	VM_EXIT_REASON			= 0x00004402,
	VM_EXIT_INTR_INFO		= 0x00004404,
	VM_EXIT_INTR_ERROR_CODE		= 0x00004406,
	IDT_VECTORING_INFO_FIELD	= 0x00004408,
	IDT_VECTORING_ERROR_CODE	= 0x0000440a,
	VM_EXIT_INSTRUCTION_LEN		= 0x0000440c,
	VMX_INSTRUCTION_INFO		= 0x0000440e,
	GUEST_ES_LIMIT			= 0x00004800,
	GUEST_CS_LIMIT			= 0x00004802,
	GUEST_SS_LIMIT			= 0x00004804,
	GUEST_DS_LIMIT			= 0x00004806,
	GUEST_FS_LIMIT			= 0x00004808,
	GUEST_GS_LIMIT			= 0x0000480a,
	GUEST_LDTR_LIMIT		= 0x0000480c,
	GUEST_TR_LIMIT			= 0x0000480e,
	GUEST_GDTR_LIMIT		= 0x00004810,
	GUEST_IDTR_LIMIT		= 0x00004812,
	GUEST_ES_AR_BYTES		= 0x00004814,
	GUEST_CS_AR_BYTES		= 0x00004816,
	GUEST_SS_AR_BYTES		= 0x00004818,
	GUEST_DS_AR_BYTES		= 0x0000481a,
	GUEST_FS_AR_BYTES		= 0x0000481c,
	GUEST_GS_AR_BYTES		= 0x0000481e,
	GUEST_LDTR_AR_BYTES		= 0x00004820,
	GUEST_TR_AR_BYTES		= 0x00004822,
	GUEST_INTERRUPTIBILITY_INFO	= 0x00004824,
	GUEST_ACTIVITY_STATE		= 0X00004826,
	GUEST_SYSENTER_CS		= 0x0000482A,
	VMX_PREEMPTION_TIMER_VALUE	= 0x0000482E,
	HOST_IA32_SYSENTER_CS		= 0x00004c00,
	CR0_GUEST_HOST_MASK		= 0x00006000,
	CR4_GUEST_HOST_MASK		= 0x00006002,
	CR0_READ_SHADOW			= 0x00006004,
	CR4_READ_SHADOW			= 0x00006006,
	CR3_TARGET_VALUE0		= 0x00006008,
	CR3_TARGET_VALUE1		= 0x0000600a,
	CR3_TARGET_VALUE2		= 0x0000600c,
	CR3_TARGET_VALUE3		= 0x0000600e,
	EXIT_QUALIFICATION		= 0x00006400,
	GUEST_LINEAR_ADDRESS		= 0x0000640a,
	GUEST_CR0			= 0x00006800,
	GUEST_CR3			= 0x00006802,
	GUEST_CR4			= 0x00006804,
	GUEST_ES_BASE			= 0x00006806,
	GUEST_CS_BASE			= 0x00006808,
	GUEST_SS_BASE			= 0x0000680a,
	GUEST_DS_BASE			= 0x0000680c,
	GUEST_FS_BASE			= 0x0000680e,
	GUEST_GS_BASE			= 0x00006810,
	GUEST_LDTR_BASE			= 0x00006812,
	GUEST_TR_BASE			= 0x00006814,
	GUEST_GDTR_BASE			= 0x00006816,
	GUEST_IDTR_BASE			= 0x00006818,
	GUEST_DR7			= 0x0000681a,
	GUEST_RSP			= 0x0000681c,
	GUEST_RIP			= 0x0000681e,
	GUEST_RFLAGS			= 0x00006820,
	GUEST_PENDING_DBG_EXCEPTIONS	= 0x00006822,
	GUEST_SYSENTER_ESP		= 0x00006824,
	GUEST_SYSENTER_EIP		= 0x00006826,
	HOST_CR0			= 0x00006c00,
	HOST_CR3			= 0x00006c02,
	HOST_CR4			= 0x00006c04,
	HOST_FS_BASE			= 0x00006c06,
	HOST_GS_BASE			= 0x00006c08,
	HOST_TR_BASE			= 0x00006c0a,
	HOST_GDTR_BASE			= 0x00006c0c,
	HOST_IDTR_BASE			= 0x00006c0e,
	HOST_IA32_SYSENTER_ESP		= 0x00006c10,
	HOST_IA32_SYSENTER_EIP		= 0x00006c12,
	HOST_RSP			= 0x00006c14,
	HOST_RIP			= 0x00006c16,
} VMCS_ENCODING;


typedef enum _VMX_EXIT_REASON {
    // ==========================================
    // Exceptions and Interrupts
    // ==========================================
    EXIT_REASON_EXCEPTION_NMI       = 0,  // Hardware exception or Non-Maskable Interrupt (NMI)
    EXIT_REASON_EXTERNAL_INTERRUPT  = 1,  // External interrupt arrived
    EXIT_REASON_TRIPLE_FAULT        = 2,  // Guest encountered a triple fault
    EXIT_REASON_INIT_SIGNAL         = 3,  // INIT signal received
    EXIT_REASON_SIPI_SIGNAL         = 4,  // Startup-IPI (SIPI) received
    EXIT_REASON_OTHER_SMI           = 6,  // System Management Interrupt (SMI)

    // ==========================================
    // Window Exiting
    // ==========================================
    EXIT_REASON_INTERRUPT_WINDOW    = 7,  // Guest is now ready to take external interrupts
    EXIT_REASON_NMI_WINDOW          = 8,  // Guest is now ready to take NMIs

    // ==========================================
    // State and CPUID
    // ==========================================
    EXIT_REASON_TASK_SWITCH         = 9,  // Task switch attempted by guest
    EXIT_REASON_CPUID               = 10, // CPUID instruction executed

    // ==========================================
    // Standard Instructions
    // ==========================================
    EXIT_REASON_HLT                 = 12, // HLT (Halt) instruction executed
    EXIT_REASON_INVD                = 13, // INVD (Invalidate Cache) instruction executed
    EXIT_REASON_INVLPG              = 14, // INVLPG (Invalidate TLB Entry) instruction executed
    EXIT_REASON_RDPMC               = 15, // RDPMC (Read Performance Monitoring Counters) executed
    EXIT_REASON_RDTSC               = 16, // RDTSC (Read Time-Stamp Counter) executed

    // ==========================================
    // VMX Instructions
    // ==========================================
    EXIT_REASON_VMCALL              = 18, // VMCALL executed (Hypercall from guest)
    EXIT_REASON_VMCLEAR             = 19, // VMCLEAR executed
    EXIT_REASON_VMLAUNCH            = 20, // VMLAUNCH executed
    EXIT_REASON_VMPTRLD             = 21, // VMPTRLD executed
    EXIT_REASON_VMPTRST             = 22, // VMPTRST executed
    EXIT_REASON_VMREAD              = 23, // VMREAD executed
    EXIT_REASON_VMRESUME            = 24, // VMRESUME executed
    EXIT_REASON_VMWRITE             = 25, // VMWRITE executed
    EXIT_REASON_VMOFF               = 26, // VMXOFF executed
    EXIT_REASON_VMON                = 27, // VMXON executed

    // ==========================================
    // Register Access & I/O
    // ==========================================
    EXIT_REASON_CR_ACCESS           = 28, // Control-register accesses (CR0, CR3, CR4, CR8)
    EXIT_REASON_DR_ACCESS           = 29, // Debug-register accesses (DR0-DR7)
    EXIT_REASON_IO_INSTRUCTION      = 30, // I/O instruction executed (IN, OUT, INS, OUTS)
    EXIT_REASON_MSR_READ            = 31, // RDMSR instruction executed
    EXIT_REASON_MSR_WRITE           = 32, // WRMSR instruction executed

    // ==========================================
    // Errors and Traps
    // ==========================================
    EXIT_REASON_INVALID_STATE       = 33, // VM-entry failure due to invalid guest state
    EXIT_REASON_MSR_LOAD_FAIL       = 34, // VM-entry failure during MSR loading
    EXIT_REASON_MWAIT_INSTRUCTION   = 36, // MWAIT instruction executed
    EXIT_REASON_MONITOR_TRAP_FLAG   = 37, // Monitor trap flag (MTF) triggered
    EXIT_REASON_MONITOR_INSTRUCTION = 39, // MONITOR instruction executed
    EXIT_REASON_PAUSE_INSTRUCTION   = 40, // PAUSE instruction executed
    EXIT_REASON_MCE_DURING_VMENTRY  = 41, // Machine-check event (MCE) during VM-entry

    // ==========================================
    // APIC, Memory, and EPT
    // ==========================================
    EXIT_REASON_TPR_BELOW_THRESHOLD = 43, // Task Priority Register (TPR) dropped below threshold
    EXIT_REASON_APIC_ACCESS         = 44, // APIC memory access attempted
    EXIT_REASON_EOI_INDUCED         = 45, // Virtualized End-of-Interrupt (EOI)
    EXIT_REASON_GDTR_IDTR           = 46, // Access to GDTR or IDTR descriptor tables
    EXIT_REASON_LDTR_TR             = 47, // Access to LDTR or TR descriptor tables
    EXIT_REASON_EPT_VIOLATION       = 48, // EPT violation (guest accessed restricted physical memory)
    EXIT_REASON_EPT_MISCONFIG       = 49, // EPT misconfiguration (invalid EPT paging structures)
    EXIT_REASON_INVEPT              = 50, // INVEPT instruction executed
    EXIT_REASON_RDTSCP              = 51, // RDTSCP instruction executed
    EXIT_REASON_PREEMPTION_TIMER    = 52, // VMX preemption timer expired
    EXIT_REASON_INVVPID             = 53, // INVVPID instruction executed

    // ==========================================
    // Advanced & Newer Features
    // ==========================================
    EXIT_REASON_WBINVD              = 54, // WBINVD instruction executed
    EXIT_REASON_XSETBV              = 55, // XSETBV instruction executed
    EXIT_REASON_APIC_WRITE          = 56, // APIC write attempted
    EXIT_REASON_RDRAND              = 57, // RDRAND instruction executed
    EXIT_REASON_INVPCID             = 58, // INVPCID instruction executed
    EXIT_REASON_VMFUNC              = 59, // VMFUNC instruction executed (VM functions)
    EXIT_REASON_ENCLS               = 60, // ENCLS instruction executed (Intel SGX)
    EXIT_REASON_RDSEED              = 61, // RDSEED instruction executed
    EXIT_REASON_PML_FULL            = 62, // Page-Modification Log (PML) is full
    EXIT_REASON_XSAVES              = 63, // XSAVES instruction executed
    EXIT_REASON_XRSTORS             = 64, // XRSTORS instruction executed

    // ==========================================
    // Wait / Bus / TDX (Confidential Computing)
    // ==========================================
    EXIT_REASON_UMWAIT              = 67, // UMWAIT instruction executed
    EXIT_REASON_TPAUSE              = 68, // TPAUSE instruction executed
    EXIT_REASON_BUS_LOCK            = 74, // Bus lock detected
    EXIT_REASON_NOTIFY              = 75, // Notify window exit (mitigates tight guest loops)
    EXIT_REASON_SEAMCALL            = 76, // SEAMCALL instruction (TDX module call)
    EXIT_REASON_TDCALL              = 77, // TDCALL instruction (TDX guest call)

    // ==========================================
    // Immediate MSR Exits
    // ==========================================
    EXIT_REASON_MSR_READ_IMM        = 84, // RDMSR instruction executed (immediate variant)
    EXIT_REASON_MSR_WRITE_IMM       = 85  // WRMSR instruction executed (immediate variant)
} VMX_EXIT_REASON;
