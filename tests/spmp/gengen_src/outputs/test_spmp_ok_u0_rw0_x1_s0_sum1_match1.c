
/*
 * outputs/test_spmp_ok_u0_rw0_x1_s0_sum1_match1.c
 * Generated from gen_spmp_test.cc and test_spmp_ok_1.cc_skel.
 * 
 * This test program is expected to start executed from M mode.
 * That will be easier for us to deal with spmp/pmp exception for test.
 * 
 * Remarks:
 * - Shared regions are not covered and will be tested separately
 */

/*
 * Macros from encoding.h
 */
#define MSTATUS_MPP         0x00001800
#define SSTATUS_SPP         0x00000100

#define PMP_R     0x01
#define PMP_W     0x02
#define PMP_X     0x04
#define PMP_A     0x18
#define PMP_L     0x80
#define PMP_SHIFT 2

#define PMP_TOR   0x08
#define PMP_NA4   0x10
#define PMP_NAPOT 0x18

#define SPMP_S    0x80
#define SSTATUS_SUM         0x00040000

#define CSR_SPMPCFG0 0x1a0
#define CSR_SPMPCFG1 0x1a1
#define CSR_SPMPCFG2 0x1a2
#define CSR_SPMPCFG3 0x1a3
#define CSR_SPMPADDR0 0x1b0
#define CSR_SPMPADDR1 0x1b1
#define CSR_SPMPADDR2 0x1b2
#define CSR_SPMPADDR3 0x1b3
#define CSR_SPMPADDR4 0x1b4
#define CSR_SPMPADDR5 0x1b5
#define CSR_SPMPADDR6 0x1b6
#define CSR_SPMPADDR7 0x1b7
#define CSR_SPMPADDR8 0x1b8
#define CSR_SPMPADDR9 0x1b9
#define CSR_SPMPADDR10 0x1ba
#define CSR_SPMPADDR11 0x1bb
#define CSR_SPMPADDR12 0x1bc
#define CSR_SPMPADDR13 0x1bd
#define CSR_SPMPADDR14 0x1be
#define CSR_SPMPADDR15 0x1bf

#define TEST_RW 1
#define TEST_FETCH 1

#define CAUSE_LOAD_ACCESS 0x5
#define CAUSE_STORE_ACCESS 0x7

#define CAUSE_FETCH_PAGE_FAULT 0xc
#define CAUSE_LOAD_PAGE_FAULT 0xd
#define CAUSE_STORE_PAGE_FAULT 0xf

typedef unsigned long reg_t;
typedef unsigned long uintptr_t;

/*
 * functions from syscalls.c
 */
#if PRINTF_SUPPORTED
int printf(const char* fmt, ...);
#else
#define printf(...)
#endif

void __attribute__((noreturn)) tohost_exit(uintptr_t code);
void exit(int code);

/*
 * local status
 */
#define TEST_MEM_START 0x200000
#define TEST_MEM_END 0x240000
#define U_MEM_END (TEST_MEM_END + 0x10000)
#define S_MEM_END (0x280000 + 0x10000)
#define FAKE_ADDRESS 0x10000000

static const unsigned long expected_rw_fail = 1;

__attribute ((section(".data_smode"), noinline))
static unsigned actual_rw_fail = 0;

static const unsigned long expected_x_fail = 1;

/* Note: the test code will try to access x_fail in s-mode, this will not be prevented by PMP
         As PMP gives all permission up.
*/
__attribute ((section(".data_smode"), noinline))
static unsigned actual_x_fail = 0;

static void checkTestResult(void);

/*
 * override syscalls.c.
 *  currently simply skip to nexp instruction
 */
uintptr_t handle_trap(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
    if (epc >= TEST_MEM_START && epc < TEST_MEM_END) {
        asm volatile ("nop");
        actual_x_fail = 1;
        checkTestResult();
    //} else if (cause == CAUSE_LOAD_ACCESS || cause == CAUSE_STORE_ACCESS) {
    } else if (cause == CAUSE_LOAD_PAGE_FAULT || cause == CAUSE_STORE_PAGE_FAULT) {
        reg_t addr;
        asm volatile ("csrr %0, mtval\n" : "=r"(addr));
        printf("addr = 0x%x\n", addr);
        if (addr >= TEST_MEM_START && addr < TEST_MEM_END) {
            actual_rw_fail = 1;
            return epc + 4;
        }
        
        if (addr == FAKE_ADDRESS) {
            asm volatile ("nop");
            asm volatile ("nop");
            checkTestResult();
        }
    } else if (cause == CAUSE_LOAD_ACCESS || cause == CAUSE_STORE_ACCESS) {
        reg_t addr;
        asm volatile ("csrr %0, mtval\n" : "=r"(addr));
        if (addr == FAKE_ADDRESS) {
            asm volatile ("nop");
            asm volatile ("nop");
            checkTestResult();
        }
    }
    
    printf("cause = %ld, epc = 0x%lx\n", cause, epc);
    tohost_exit(1337);
}


// switch (eret) to U mode and resume next PC
static void switch_to_U() {
#if 0 //switch to U mode from M mode
    reg_t tmp;
    asm volatile (
            "li %0, %1\n"
            "\tcsrc mstatus, t0\n"
            "\tla %0, try_access_umode \n"
            "\tcsrw mepc, %0\n"
            "\tli sp, %2\n"
            "\tmret\n"
            : "=r"(tmp) : "n"(MSTATUS_MPP), "n"(U_MEM_END) : "memory");
#else //switch to U mode from S mode
    reg_t tmp;
    asm volatile (
            "li %0, %1\n"
            "\tcsrc sstatus, t0\n"
            "\tla %0, try_access_umode \n"
            "\tcsrw sepc, %0\n"
            "\tli sp, %2\n"
            "\tsret\n"
            : "=r"(tmp) : "n"(SSTATUS_SPP), "n"(U_MEM_END) : "memory");
#endif
}

static void switch_mode() {
#if 0
    switch_to_U();
#endif
}

static void switch_smode() {
#if 0
    reg_t tmp;
    asm volatile (
            "li %0, %1\n"
            "\tcsrc mstatus, t0\n"
            "\tla %0, smode_main\n"
            "\tcsrw mepc, %0\n"
            "\tli sp, %2\n"
            "\tmret\n"
            : "=r"(tmp) : "n"(MSTATUS_MPP), "n"(S_MEM_END) : "memory");
#else
	reg_t tmp;

	asm volatile (
	"csrr %0, mstatus\n"
	: "=r"(tmp)::"memory"); 

	tmp = tmp & ~(MSTATUS_MPP); // clear MPP bits
	tmp = tmp | (0x1 << 11);    // set MPP to S-mode

	asm volatile (
	"csrw mstatus, %0\n"
	::"r"(tmp) :"memory"); 

	//clear satp (to disable paging)
	asm volatile (
	"csrw satp, x0\n"
	:::"memory"); 

    	asm volatile (
            "\tla %0, smode_main\n"
            "\tcsrw mepc, %0\n"
            "\tli sp, %1\n"
            "\tmret\n"
            : "=r"(tmp) : "n"(S_MEM_END) : "memory");

#endif
}

__attribute ((section(".text_test_foo"), noinline))
static void target_foo() {
    asm volatile ("nop");
    
    actual_x_fail = 0;
}

/*
 * avoid to access actual_x_fail lies in M mode
 */
__attribute ((section(".text_test_foo"), noinline))
static void target_foo_umode() {
    asm volatile ("nop");
}

__attribute ((section(".data_test_arr"), aligned(8)))
static volatile unsigned char target_arr[100] = {
        1,2,3,4,5,6,7,8,
};

/*
 * On processor_t::reset():
 *  - set_csr(CSR_PMPADDR0, ~reg_t(0));
 *    set_csr(CSR_PMPCFG0, PMP_R | PMP_W | PMP_X | PMP_NAPOT);
 */
static void set_cfg() {
    
    // Give all the memory accessible to the S/U modes
    //asm volatile ("csrw pmpaddr0, %0\n" :: "r"(~reg_t(0)) : "memory");
    
    asm volatile ("csrw pmpaddr0, %0\n" :: "r"((0x400000>>2)-1) : "memory");
    /* We can not give permission to access all addresses because we rely on fake addr for trap */

    asm volatile ("csrw pmpcfg0,  %0\n" :: "r"(PMP_R | PMP_W | PMP_X | PMP_NAPOT) : "memory");

    // currently dummy since tlb flushed when set_csr on mseccfg
    asm volatile ("fence.i \n");
}

// from pmp_ok() side,W/R/X is similar
__attribute ((section(".text_smode")))
__attribute ((noinline))
static void try_access() {
#if TEST_RW
    target_arr[0] += 1;
    //const unsigned long delta = 0x1020304005060708UL;
#define DELTA 0x1020304005060708UL
    *(long *)target_arr += DELTA;

    if (*(long *)target_arr != 0x0807060504030201UL + DELTA + 1) {
        actual_rw_fail = 1;
    }
#endif

#if TEST_FETCH
    actual_x_fail = 1;  // reset inside target_foo()
    target_foo();
#endif
}

// in case mml set, printf cannot be used in U mode
__attribute ((section(".text_umode")))
void try_access_umode() {
#if TEST_RW
    target_arr[0] += 1;
//    const unsigned long delta = 0x1020304005060708UL;
//    *(long *)target_arr += delta;

//    if (*(long *)target_arr != 0x0807060504030201UL + delta + 1) {
//        actual_rw_fail = 1;
//    }
#endif

#if TEST_FETCH
    target_foo_umode();
#endif
    
    /*
     * switch to M mode by invoking a write access fault for special address.
     */ 
    *(char *)(FAKE_ADDRESS) = 1;
}

// S-mode text
__attribute ((section(".text_smode")))
void smode_main() {
	/* configure sPMP */

   	/*
   	 * Here @spmp_addr_offset:int@ is to create an address mismatch
   	 * And @create_spmp_cfg:int@ is to create cfg mismatch.
   	 */
#if 0
   	asm volatile ("csrw 0x1b4, %0 \n" :: "r"(S_MEM_END >> 2) : "memory"); //spmpaddr4
   	asm volatile ("csrw 0x1b3, %0 \n" :: "r"(U_MEM_END >> 2) : "memory"); //spmpaddr3
   	asm volatile ("csrw 0x1b2, %0 \n" :: "r"(TEST_MEM_END >> 2) : "memory"); //spmpaddr2
   	asm volatile ("csrw 0x1b1, %0 \n" :: "r"((TEST_MEM_START + 0) >> 2) : "memory"); //spmpaddr1

   	asm volatile ("csrw 0x1b0, %0 \n" :: "r"((TEST_MEM_START >> 3) - 1) : "memory"); //spmpaddr0
    
    	//FIXME: legacy code which gives the M-mode memory permissions 
	//       It works because sPMP allows access (from S-mode) by default
	reg_t cfg0 = (PMP_R | PMP_W | PMP_X | PMP_NAPOT);

	reg_t cfg1 = (PMP_R | PMP_W | PMP_X | SPMP_S | PMP_TOR);   // for S_MEM

    	cfg0 |= (PMP_R | PMP_W | PMP_X | PMP_TOR) << 24;    // for U_MEM
#if 1
    	cfg0 |= ((0 ? (PMP_R | PMP_W)  : 0) | (1 ? PMP_X : 0) | PMP_TOR | (0 ? SPMP_S : 0)) << 16;
#endif   
    	asm volatile ("csrw 0x1a0, %0 \n"
    	            :
    	            : "r"(cfg0)
    	            : "memory");
#else
   	asm volatile ("csrw 0x1b3, %0 \n" :: "r"(S_MEM_END >> 2) : "memory"); //spmpaddr3
   	asm volatile ("csrw 0x1b2, %0 \n" :: "r"(U_MEM_END >> 2) : "memory"); //spmpaddr2
   	asm volatile ("csrw 0x1b1, %0 \n" :: "r"(TEST_MEM_END >> 2) : "memory"); //spmpaddr1
   	asm volatile ("csrw 0x1b0, %0 \n" :: "r"((TEST_MEM_START + 0) >> 2) : "memory"); //spmpaddr0

	//reg_t cfg0 = (PMP_R | PMP_W | PMP_X | PMP_NAPOT) << 24; // for S_MEM

	/* This is a shared R/W/X regions because we do not have R/W/X for S-mode only */
	reg_t cfg0 = (!PMP_R | !PMP_W | !PMP_X | SPMP_S | PMP_TOR) << 24;   // for S_MEM

    	cfg0 |= (PMP_R | PMP_W | PMP_X | PMP_TOR) << 16;    // for U_MEM

#if 1
    	cfg0 |= ((0 ? (PMP_R | PMP_W)  : 0) | (1 ? PMP_X : 0) | PMP_TOR | (0 ? SPMP_S : 0)) << 8;
#endif   
    	asm volatile ("csrw 0x1a0, %0 \n"
    	            :
    	            : "r"(cfg0)
    	            : "memory");
#endif

	//configure SUM bit
#if 1
	//set SUM
    	asm volatile (
    	        "li t0, 0x00040000\n"
    	        "\tcsrs sstatus, t0\n"
    	        ::: "t0", "memory");
#else
	//clear SUM
    	asm volatile (
    	        "li t0, 0x00040000\n"
    	        "\tcsrc sstatus, t0\n"
    	        :::"t0", "memory");
#endif

	/* Test */
    	switch_mode();  // in case swith to u mode, branch to try_access_umode directly
    	try_access();

	/* End */
    	checkTestResult();
}

static void checkTestResult() {
    int ret = 0;
    if (expected_rw_fail != actual_rw_fail) {
        ret += 1;
        printf("Read/write test fail, expected %d, actual %d.\n", expected_rw_fail, actual_rw_fail);
    }

    if (expected_x_fail != actual_x_fail) {
        ret += 2;
        printf("Fetch test fail, expected %d, actual %d.\n", expected_x_fail, actual_x_fail);
    }
    
    printf("Test done, exit %d.\n", ret);
    
    exit(ret); 
}

int main() {
    // assert in M mode
    set_cfg();

    switch_smode();

    return 0; // assert 0
}

