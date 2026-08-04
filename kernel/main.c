#include "kernel.h"

static void task_a(void) {
    for (;;) {
        kprint("A");
    }
}

static void task_b(void) {
    for (;;) {
        kprint("B");
    }
}

static void mm_verify(void) {
    kprintln("===== MM Verification =====");

    /* --- PMM: alloc/free --- */
    ku64 free_before = pmm_free_pages();

    ku64 p1 = pmm_alloc_page();
    if (!p1) { kprintln("[FAIL] pmm_alloc_page returned 0"); return; }
    kprint("[PASS] pmm_alloc_page -> "); kprint_hex(p1); kprintln("");

    ku64 free_after_alloc = pmm_free_pages();
    if (free_after_alloc != free_before - 1) {
        kprintln("[FAIL] free count mismatch after alloc");
        return;
    }
    kprintln("[PASS] free count --");

    pmm_free_page(p1);
    if (pmm_free_pages() != free_before) {
        kprintln("[FAIL] free count not restored after free");
        return;
    }
    kprintln("[PASS] pmm_free_page -> free count restored");

    ku64 p2 = pmm_alloc_page();
    if (!p2) { kprintln("[FAIL] re-alloc after free failed"); return; }
    kprintln("[PASS] re-alloc after free");

    /* --- VMM: map / unmap / get_mapping --- */
    ku64 pa = pmm_alloc_page();
    if (!pa) { kprintln("[FAIL] pmm_alloc for vmm test"); return; }
    ku64 va = KERNEL_HEAP_START + 0x100000;
    ku64 flags = PTE_PRESENT | PTE_WRITABLE;

    vmm_map_page(&kernel_as, va, pa, flags);

    ku64 got_pa = vmm_get_mapping(&kernel_as, va);
    if (got_pa != pa) {
        kprintln("[FAIL] vmm_get_mapping mismatch");
        kprint("  expected: "); kprint_hex(pa); kprintln("");
        kprint("  got:      "); kprint_hex(got_pa); kprintln("");
        return;
    }
    kprintln("[PASS] vmm_map_page + vmm_get_mapping");

    vmm_unmap_page(&kernel_as, va);
    if (vmm_get_mapping(&kernel_as, va) != 0) {
        kprintln("[FAIL] vmm_unmap_page: mapping still present");
        return;
    }
    kprintln("[PASS] vmm_unmap_page -> mapping cleared");
    pmm_free_page(pa);

    /* --- KHEAP: alloc/free/realloc --- */
    void *a = kmalloc(0);
    if (a != null) { kprintln("[FAIL] kmalloc(0) should return null"); return; }
    kprintln("[PASS] kmalloc(0) -> null");

    ku8 *b = (ku8 *)kmalloc(128);
    if (!b) { kprintln("[FAIL] kmalloc(128) returned null"); return; }
    kprint("[PASS] kmalloc(128) -> "); kprint_hex((ku64)b); kprintln("");

    for (int i = 0; i < 128; i++) b[i] = (ku8)(i & 0xFF);
    for (int i = 0; i < 128; i++) {
        if (b[i] != (ku8)(i & 0xFF)) {
            kprintln("[FAIL] kmalloc: data corruption");
            return;
        }
    }
    kprintln("[PASS] kmalloc: write/read verify");

    kfree(b);
    kprintln("[PASS] kfree");

    void *c = kmalloc(64);
    if (!c) { kprintln("[FAIL] kmalloc(64) after free"); return; }
    kprintln("[PASS] kmalloc(64) re-alloc after free");

    void *d = krealloc(c, 256);
    if (!d) { kprintln("[FAIL] krealloc to larger size"); return; }
    kprintln("[PASS] krealloc(64->256)");

    void *e = krealloc(null, 32);
    if (!e) { kprintln("[FAIL] krealloc(null, 32)"); return; }
    kprintln("[PASS] krealloc(null, 32) -> alloc");

    kfree(d);
    kfree(e);
    kprintln("[PASS] cleanup done");

    kprintln("[PIT] pit 2000ms sleep test");
    sleep_ms(2000);
    kprintln("[PASS] pit test passed");

    kprintln("===== ALL TESTS PASSED =====");
}

void start_kernel(ku32 magic, ku64 info_ptr)
{
    kprint("\n");
    kprintln("Tenon v0.0.1");

    // mm
    pmm_init(magic, info_ptr);
    vmm_init();
    kheap_init();
    
    // pit
    pit_init(1000);

    idt_init();
    task_init();

    task_create(task_a, "a");
    task_create(task_b, "b");

    pic_init();
    pic_unmask(0); // 开PIT
    pic_unmask(1); // 开键盘
    asm_sti();

    mm_verify();

    for (;;) {
        asm_hlt();
    }
}