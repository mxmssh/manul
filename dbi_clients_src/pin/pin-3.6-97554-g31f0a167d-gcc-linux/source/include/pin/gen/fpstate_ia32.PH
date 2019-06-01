//Groups: @ingroup\s+(API_REF|KNOBS|IMG_BASIC_API|INS_BASIC_API|INS_INST_API|INS_BASIC_API_GEN_IA32|INS_BASIC_API_IA32|INS_MOD_API_GEN_IA32|SEC_BASIC_API|RTN_BASIC_API|REG_BASIC_API|REG_CPU_GENERIC|REG_CPU_IA32|TRACE_BASIC_API|BBL_BASIC_API|SYM_BASIC_API|MISC_PRINT|MISC_PARSE|KNOB_API|KNOB_BASIC|KNOB_PRINT|LOCK|PIN_CONTROL|TRACE_VERSION_API|BUFFER_API|PROTO_API|PIN_PROCESS_API|PIN_THREAD_API|PIN_SYSCALL_API|WINDOWS_SYSCALL_API_UNDOC|DEBUG_API|ERROR_FILE_BASIC|TYPE_BASE|INSTLIB|ALARM|CODECACHE_API|CHILD_PROCESS_API|UTILS|MISC|CONTEXT_API|PHYSICAL_CONTEXT_API|PIN_CALLBACKS|EXCEPTION_API|APPDEBUG_API|STOPPED_THREAD_API|BUFFER_API|PROTO|INST_ARGS|DEPRECATED_PIN_API|INTERNAL_EXCEPTION_PRIVATE_UNDOCUMENTED|PIN_THREAD_PRIVATE|CHILD_PROCESS_INTERNAL|BBL_BASIC|ROGUE_BASIC_API|MESSAGE_TYPE|MESSAGE_BASIC|ERRFILE|MISC_BASIC|ITC_INST_API|CONTEXT_API_UNDOC|EXCEPTION_API_UNDOC|UNDOCUMENTED_PIN_API|OPIN|TRACE_VERSIONS
/* PIN API */

/* THIS FILE IS AUTOMAGICALLY GENERATED - DO NOT CHANGE DIRECTLY*/


typedef UTIL::FXSAVE_IA32 FXSAVE;

                                                                  /* DO NOT EDIT */
struct XSAVE_HEADER
{
    UINT64 _mask;
    UINT64 _xcomp_bv;
    UINT64 _reserved[6];
};

                                                                  /* DO NOT EDIT */
const size_t VSTATE_PADDING = 128;

                                                                  /* DO NOT EDIT */
struct FPSTATE
{
    // fxsave_legacy is applicable on all supported CPUs
    FXSAVE fxsave_legacy; //                                                    512  bytes at offset 0

    // The following are only applicable on processors that support XSAVE
    struct XSTATE
    {
        XSAVE_HEADER _extendedHeader; //                                        64   bytes at offset 512
        UINT8 _ymmUpper[8*16];  // upper 128 bits of ymm0-ymm7                  128  bytes at offset 576
        UINT8 _pad[8*16];       // reserved - no ymm8-ymm15 on IA-32            128  bytes at offset 704
    } _xstate;

    // The following is only applicable on processors that support AVX512
    // So far there are 832 bytes of data but the next bulk of data begins at offset 1088.
    UINT8 _reserved[VSTATE_PADDING]; //                                         128  bytes at offset 832


    // The _vstate is comprised of five parts.
    //   1. BND registers BND0-BND3
    //   2. BND control and status registers
    //   3. The Opmask registers k0-k7
    //   4. The upper 256 bits of zmm0-zmm7 (and padding for zmm8-zmm15)
    //   5. reserved - no zmm16-zmm31 on IA32
    struct VSTATE
    {
        UINT8  _bndRegs[64];    // BND registers BND0-BND3.                     64   bytes at offset 960
        UINT8  _bndCSR[64];     // BND control and status registers.            64   bytes at offset 1024
        UINT64 _kmasks[8];      // 8 64-bit k (opmask) registers                64   bytes at offset 1088
        UINT8  _zmmUpper[8*32]; // upper 256 bits of zmm0-zmm7                  256  bytes at offset 1152
        UINT8  _pad256[8*32];   // reserved - no zmm8-zmm15 on IA-32            256  bytes at offset 1408
        UINT8  _pad1024[16*64]; //                                              1024 bytes at offset 1664
    } _vstate;

    // Total of 2688 bytes.
};

                                                                  /* DO NOT EDIT */
const size_t FPSTATE_SIZE_FXSAVE = sizeof(FXSAVE);

                                                                  /* DO NOT EDIT */
const size_t FPSTATE_SIZE_XSAVE_AVX = sizeof(FXSAVE) + sizeof(FPSTATE::XSTATE);

                                                                  /* DO NOT EDIT */
const size_t FPSTATE_SIZE_XSAVE_AVX512 =
        sizeof(FXSAVE) + sizeof(FPSTATE::XSTATE) + VSTATE_PADDING + sizeof(FPSTATE::VSTATE);

                                                                  /* DO NOT EDIT */
const size_t FPSTATE_SIZE = sizeof(FPSTATE);

                                                                  /* DO NOT EDIT */
const size_t FPSTATE_ALIGNMENT = 64;

                                                                  /* DO NOT EDIT */
const size_t FP_STATE_EXTENDED_HEADER_SIZE = sizeof(XSAVE_HEADER);

                                                                  /* DO NOT EDIT */
const UINT64 X87_CLASS_BIT = 0x1;

                                                                  /* DO NOT EDIT */
const UINT64 SSE_CLASS_BIT = 0x2;

                                                                  /* DO NOT EDIT */
const UINT64 AVX_CLASS_BIT = 0x4;

                                                                  /* DO NOT EDIT */
const UINT64 BNDREGS_CLASS_BIT = 0x8;

                                                                  /* DO NOT EDIT */
const UINT64 BNDCSR_CLASS_BIT = 0x10;

                                                                  /* DO NOT EDIT */
const UINT64 OPMASK_CLASS_BIT = 0x20;

                                                                  /* DO NOT EDIT */
const UINT64 ZMM_HI256_CLASS_BIT = 0x40;

                                                                  /* DO NOT EDIT */
const UINT64 HI16_ZMM_CLASS_BIT = 0x80;

                                                                  /* DO NOT EDIT */

