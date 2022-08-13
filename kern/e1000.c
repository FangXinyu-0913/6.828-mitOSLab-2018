#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here
int PCI_E1000_Driver(struct pci_func *pcif)
{
    pci_func_enable(pcif);
    cprintf("pci_e1000_driver reg_base[0] %x reg_size[0] %x\n", pcif->reg_base[0], pcif->reg_size[0]);
    //将E1000的I/O空间映射到虚拟地址MMIOBASE之上
    //MMIO_MAP_REGION中在建立映射插入页目录表项和页表项时会设置相关权限位来标志这是一个内存映射I/O
    e1000_base = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    // cprintf("E1000 STATUS: %x\n",  e1000_base[E1000_STATUS/4]);
    return 0;
}
