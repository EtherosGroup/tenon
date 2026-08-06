#include "kernel.h"
#include "demo.h"

/**
 * test By DeepSeek v4 pro
 */
static void mm_verify(void)
{
    serial_println("===== MM Verification =====");

    u64 free_before = pmm_free_pages();

    u64 p1 = pmm_alloc_page();
    if (!p1)
    {
        serial_println("[FAIL] pmm_alloc_page returned 0");
        return;
    }
    serial_print("[PASS] pmm_alloc_page -> ");
    serial_print_hex(p1);
    serial_println("");

    u64 free_after_alloc = pmm_free_pages();
    if (free_after_alloc != free_before - 1)
    {
        serial_println("[FAIL] free count mismatch after alloc");
        return;
    }
    serial_println("[PASS] free count --");

    pmm_free_page(p1);
    if (pmm_free_pages() != free_before)
    {
        serial_println("[FAIL] free count not restored after free");
        return;
    }
    serial_println("[PASS] pmm_free_page -> free count restored");

    u64 p2 = pmm_alloc_page();
    if (!p2)
    {
        serial_println("[FAIL] re-alloc after free failed");
        return;
    }
    serial_println("[PASS] re-alloc after free");

    u64 pa = pmm_alloc_page();
    if (!pa)
    {
        serial_println("[FAIL] pmm_alloc for vmm test");
        return;
    }
    u64 va = KERNEL_HEAP_START + 0x100000;
    u64 flags = PTE_PRESENT | PTE_WRITABLE;

    vmm_map_page(&kernel_as, va, pa, flags);

    u64 got_pa = vmm_get_mapping(&kernel_as, va);
    if (got_pa != pa)
    {
        serial_println("[FAIL] vmm_get_mapping mismatch");
        serial_print("  expected: ");
        serial_print_hex(pa);
        serial_println("");
        serial_print("  got:      ");
        serial_print_hex(got_pa);
        serial_println("");
        return;
    }
    serial_println("[PASS] vmm_map_page + vmm_get_mapping");

    vmm_unmap_page(&kernel_as, va);
    if (vmm_get_mapping(&kernel_as, va) != 0)
    {
        serial_println("[FAIL] vmm_unmap_page: mapping still present");
        return;
    }
    serial_println("[PASS] vmm_unmap_page -> mapping cleared");
    pmm_free_page(pa);

    void *a = kmalloc(0);
    if (a != null)
    {
        serial_println("[FAIL] kmalloc(0) should return null");
        return;
    }
    serial_println("[PASS] kmalloc(0) -> null");

    u8 *b = (u8 *)kmalloc(128);
    if (!b)
    {
        serial_println("[FAIL] kmalloc(128) returned null");
        return;
    }
    serial_print("[PASS] kmalloc(128) -> ");
    serial_print_hex((u64)b);
    serial_println("");

    for (int i = 0; i < 128; i++)
    {
        b[i] = (u8)(i & 0xFF);
    }
    for (int i = 0; i < 128; i++)
    {
        if (b[i] != (u8)(i & 0xFF))
        {
            serial_println("[FAIL] kmalloc: data corruption");
            return;
        }
    }
    serial_println("[PASS] kmalloc: write/read verify");

    kfree(b);
    serial_println("[PASS] kfree");

    void *c = kmalloc(64);
    if (!c)
    {
        serial_println("[FAIL] kmalloc(64) after free");
        return;
    }
    serial_println("[PASS] kmalloc(64) re-alloc after free");

    void *d = krealloc(c, 256);
    if (!d)
    {
        serial_println("[FAIL] krealloc to larger size");
        return;
    }
    serial_println("[PASS] krealloc(64->256)");

    void *e = krealloc(null, 32);
    if (!e)
    {
        serial_println("[FAIL] krealloc(null, 32)");
        return;
    }
    serial_println("[PASS] krealloc(null, 32) -> alloc");

    kfree(d);
    kfree(e);
    serial_println("[PASS] cleanup done");

    serial_println("[PIT] pit 2000ms sleep test");
    sleep_ms(2000);
    serial_println("[PASS] pit test passed");

    serial_println("===== ALL TESTS PASSED =====");
}
void start_kernel(u32 magic, u64 info_ptr)
{
    serial_print("\n");
    serial_println("Tenon v0.0.1");

    pmm_init(magic, info_ptr);
    vmm_init();
    kheap_init();

    vfs_init();

    vfs_sb_type *root_sb = ramfs_create_sb();
    mufs_init();
    mufs_mount(root_sb, "system", MUFST_SMP, 1);

    mufs_set_var("temp", "/system/temp/");
    mufs_set_var("home", "/system/home/");
    mufs_set_var("config", "/system/config/");

    vfs_mkdir("/system/temp");
    vfs_mkdir("/system/home");
    vfs_mkdir("/system/config");
    vfs_mkdir("/system/kernel");

    pci_scan();

    ata_init(ATA_PRIMARY_DATA, ATA_PRIMARY_CTRL, ATA_DRIVE_MASTER, "hda");
    ata_init(ATA_PRIMARY_DATA, ATA_PRIMARY_CTRL, ATA_DRIVE_SLAVE,  "hdb");
    ata_init(ATA_SECONDARY_DATA, ATA_SECONDARY_CTRL, ATA_DRIVE_MASTER, "hdc");
    ata_init(ATA_SECONDARY_DATA, ATA_SECONDARY_CTRL, ATA_DRIVE_SLAVE,  "hdd");

    serial_print("[BLOCK] ");
    serial_print_dec(block_device_count);
    serial_println(" device(s) registered");

    multiboot_tag_fb_type fb_tag;
    bool fb_valid;
    pmm_get_fb_tag(&fb_tag, &fb_valid);
    if (fb_valid && fb_tag.fb_type == 1)
    {
        fb_init(fb_tag.fb_addr, fb_tag.fb_width, fb_tag.fb_height,
            fb_tag.fb_pitch, fb_tag.fb_bpp,
            fb_tag.fb_red_field_position, fb_tag.fb_red_mask_size,
            fb_tag.fb_green_field_position, fb_tag.fb_green_mask_size,
            fb_tag.fb_blue_field_position, fb_tag.fb_blue_mask_size);
        terminal_init(fb_tag.fb_width, fb_tag.fb_height, COLOR_WHITE, COLOR_BLACK);
    }
    else
    {
        serial_println("[FB] No framebuffer tag, serial-only fallback");
    }

    pit_init(1000);
    idt_init();
    tss_init();
    task_init();

    demo();

    keyboard_ring_init();
    pic_init();
    pic_unmask(0);
    pic_unmask(1);
    asm_sti();

    mm_verify();

    for (;;)
    {
        asm_hlt();
    }
}