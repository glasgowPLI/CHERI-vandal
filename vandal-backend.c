#include <cheriintrin.h>
#include "setjmpstatic.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifndef VANDAL_RO_LIMIT 
#define VANDAL_RO_LIMIT 16
#endif

#ifndef VANDAL_FILL_PATTERN
#define VANDAL_FILL_PATTERN ((void*)0xdeaddeaddeaddeadUL)
#endif

#ifndef VANDAL_WARN
#define VANDAL_WARN(msg, cap) fprintf(stderr, "Vandal: " msg " (%#p)\n", cap)
#endif

#define ARM_CAP_OTYPE_LPB ((cheri_otype_t)2)
#define ARM_CAP_OTYPE_LB ((cheri_otype_t)3)

static jmp_buf * jbuf;

static bool check_cap(void * cap) {
    if(!cheri_is_valid(cap) || (cap == &jbuf) || (cap == stderr) || (cheri_address_get(cap) > 0xffff00000000UL)) return false;

    int perms = cheri_perms_get(cap);
    
    if(cheri_is_sealed(cap)) {
	switch(cheri_type_get(cap)) {
	    case CHERI_OTYPE_SENTRY:
                //VANDAL_WARN("found sentry", cap);
		return false;
	    case ARM_CAP_OTYPE_LPB:
                VANDAL_WARN("found LPB-sealed cap", cap);
		return false;
	    case ARM_CAP_OTYPE_LB:
                VANDAL_WARN("found LB-sealed cap", cap);
		return false;
	}
        if(perms & ARM_CAP_PERMISSION_BRANCH_SEALED_PAIR) {
            VANDAL_WARN("found sealed cap with permission BRANCH SEALED PAIR", cap);
        }
        return false;
    }

    if(perms & CHERI_PERM_UNSEAL) {
        VANDAL_WARN("found unsealed cap with permission UNSEAL", cap);
    }

    if(perms & CHERI_PERM_EXECUTE && !cheri_is_subset(cap, cheri_pcc_get())) {
        VANDAL_WARN("found unsealed cap with permission EXECUTE", cap);
    }

    return true;
}

static void vandalise_cap(void * cap, vaddr_t stack_guard, int ro_depth) {
    if(!check_cap(cap)) return;

    if(cheri_base_get(cap) == cheri_base_get(__builtin_cheri_stack_get()) && cheri_address_get(cap) < stack_guard) {
	ssize_t len = cheri_base_get(cap) + cheri_length_get(cap) - stack_guard;
	if(len <= 0) return;
        cap = cheri_bounds_set(cheri_address_set(cap, stack_guard), len);
    }
    
    jmp_buf thisbuf, *oldbuf;
    oldbuf = jbuf;
    jbuf = &thisbuf;


    if(cheri_length_get(cap) < 16) {
        if(cheri_perms_get(cap) & CHERI_PERM_STORE && !_setjmp(*jbuf)) {
	    int i;
	    char * fill = cheri_offset_set(cap,0);
	    for(i = 0; i < cheri_length_get(cap); i++) fill[i] = 0xaa;
	}
	jbuf = oldbuf;
	return;
    }

    void ** base = cheri_offset_set(cap,0);
    void ** limit = cheri_offset_set(cap, cheri_length_get(cap));
    void ** volatile scan = cheri_align_up(base, 16);
    void ** scanlimit = cheri_align_down(limit, 16);

    switch(cheri_perms_get(cap) & (CHERI_PERM_LOAD|CHERI_PERM_LOAD_CAP|CHERI_PERM_STORE)) {
	case 0:
	case CHERI_PERM_LOAD:
	case CHERI_PERM_LOAD_CAP:
            /* Can't recurse or overwrite */
	    break;
        case (CHERI_PERM_LOAD|CHERI_PERM_LOAD_CAP|CHERI_PERM_STORE):
	    /* Overwrite, then recurse */
   	    if(_setjmp(*jbuf)) {
		// TODO First try to treat the rest of the page as read-only
		scan = cheri_align_up(scan + 1, 4096); // Try next page
            }
            while(scan < scanlimit) {
	        register void * tmp = *scan;
	        *scan++ = VANDAL_FILL_PATTERN;// Overwriting first prevents non-termination
	        if(!cheri_is_subset(tmp,cap)) vandalise_cap(tmp, stack_guard, ro_depth);
	    }
            if(!cheri_is_aligned(base,16) && !_setjmp(*jbuf)) {
	        int i;
	        char * fill = (void*)base;
	        for(i = 0; i < (16 - ((vaddr_t)base & 0xf)); i++) fill[i] = 0xaa;
            }
            if(scanlimit < limit && !_setjmp(*jbuf)) {
	        int i;
	        char * fill = (void*)scanlimit;
	        for(i = 0; i < ((vaddr_t)limit & 0xf); i++) fill[i] = 0xaa;
	    }
	    break;;
	case (CHERI_PERM_LOAD|CHERI_PERM_LOAD_CAP):
	    /* Can't overwrite, just recurse */
	    if(ro_depth >= VANDAL_RO_LIMIT) break; // Avoid non-termination due to a RO loop
            if(_setjmp(*jbuf)) scan = cheri_align_up(scan + 1, 4096); // Try next page
	    while(scan < scanlimit) {
		register void * tmp = *scan++;
                if(!cheri_is_subset(tmp,cap)) vandalise_cap(tmp, stack_guard, ro_depth + 1);
	    }
	    break;
	default:
	    /* Can overwrite but not recurse */
	    if(_setjmp(*jbuf)) scan = cheri_align_up(scan + 1, 4096); // Try next page
	    while(scan < scanlimit) *scan++ = VANDAL_FILL_PATTERN;
            if(!cheri_is_aligned(base,16) && !_setjmp(*jbuf)) {
	        int i;
	        char * fill = (void*)base;
	        for(i = 0; i < (16 - ((vaddr_t)base & 0xf)); i++) fill[i] = 0xaa;
            }
            if(scanlimit < limit && !_setjmp(*jbuf)) {
                memset(scanlimit, 0xaa, (vaddr_t)limit & 0xf); 
	        int i;
	        char * fill = (void*)scanlimit;
	        for(i = 0; i < ((vaddr_t)limit & 0xf); i++) fill[i] = 0xaa;
            }
    }
    jbuf = oldbuf;
}

static __attribute__((__noinline__)) void vandal_hook_badsegv(void) {}

/* Hack -- Sometimes we will find a capability to an unmapped page, or a RW capability
 * to a RO page. In these cases we catch SIGSEGV and move on */
static void segv_recover(int sig, siginfo_t * info, void * dat) {
    if(info->si_code != SEGV_ACCERR) vandal_hook_badsegv();
    _longjmp(*jbuf, 1);
}

/* Attack via an array of capabilites.
 * The 'protected' stack region (`BASE(CSP)` to `stack_guard`) will remain unaffected.
 *
 * The attack is performed and the value 0 is returned.
 *
 * targets     - array of target capabilities 
 * n           - number of target capabilities
 * stack_guard - end of 'protected' stack region */
int vandalise_arr(void ** targets, int n, vaddr_t stack_guard) {
    struct sigaction segv_orig, segv_rec = { .sa_sigaction = &segv_recover, .sa_flags = SA_SIGINFO|SA_NODEFER };
    sigaction(SIGSEGV, &segv_rec, &segv_orig);
    while(n-- > 0) {
        vandalise_cap(targets[n], stack_guard, 0);
    }
    sigaction(SIGSEGV, &segv_orig, NULL);
    return 0;
}


