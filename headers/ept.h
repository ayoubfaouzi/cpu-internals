/// See: Extended-Page-Table Pointer (EPTP)
union EptPointer {
    ULONG64 all;
    struct
    {
        ULONG64 memory_type : 3;                     //!< [0:2]
        ULONG64 page_walk_length : 3;                //!< [3:5]
        ULONG64 enable_accessed_and_dirty_flags : 1; //!< [6]
        ULONG64 reserved1 : 5;                       //!< [7:11]
        ULONG64 pml4_address : 36;                   //!< [12:48-1]
        ULONG64 reserved2 : 16;                      //!< [48:63]
    } fields;
};