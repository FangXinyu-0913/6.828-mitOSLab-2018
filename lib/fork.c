// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) ROUNDDOWN(utf->utf_fault_va, PGSIZE);//对齐到4K边界
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if (!(err & FEC_WR)) {
		panic("pgfault error: not writing a page.");
	}
	if (!(uvpt[PGNUM(utf->utf_fault_va)] & PTE_COW))
		panic("pgfault error: not a COW page.");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	
	if ((r=sys_page_alloc(0, (void*)PFTEMP, PTE_U|PTE_W)) < 0)
		panic("pgfault error: %e", r);
	memmove((void*)PFTEMP, addr, PGSIZE);

	if ((r=sys_page_map(0, (void*)PFTEMP, 0, addr, PTE_U|PTE_W)) < 0)
		panic("pgfault error: %e", r);
	if ((r=sys_page_unmap(0, (void*)PFTEMP)) < 0)
		panic("pgfault error: %e", r);

	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn) //复制映射关系
{
	// LAB 4: Your code here.
	//panic("duppage not implemented");
	int err;
	int curEnvId = sys_getenvid();
	//如果该页是可写页或COW页且该页不共享
	if(uvpt[pn]&(PTE_W|PTE_COW) && !(uvpt[pn]&PTE_SHARE)) //lab5练习8修改处
	{	
		//复制映射关系,并标记子进程中该页为COW页
		err = sys_page_map(curEnvId,(void*)(pn*PGSIZE),envid,(void*)(pn*PGSIZE),PTE_P|PTE_U|PTE_COW);
		if(err<0)
		//panic("duppage:sys_page_map failed when copy mappings from W/COW page - %e\n", err);
			return err;
		//标记父进程为COW页
		err = sys_page_map(curEnvId,(void*)(pn*PGSIZE),curEnvId,(void*)(pn*PGSIZE),PTE_P|PTE_U|PTE_COW);
		if(err <0)
			return err;
	}
	else //该页是只读页或共享页
	{	
		int perm = uvpt[pn]&PTE_SHARE ? uvpt[pn]&PTE_SYSCALL : PTE_P|PTE_U; //lab5练习8修改处
		err = sys_page_map(curEnvId,(void*)(pn*PGSIZE),envid,(void*)(pn*PGSIZE),perm);
		//err = sys_page_map(curEnvId,(void*)(pn*PGSIZE),envid,(void*)(pn*PGSIZE),(uvpt[n]<<20)>>20);
		if(err <0)
		//panic("duppage:sys_page_map failed when copy mappings from R-ONLY page - %e\n", err);
			return err;
	}	
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	// 1. 父进程调用`set_pgfault_handler()`，将`pgfault()`设置为页面错误处理函数
	int err;
	extern void _pgfault_upcall(void);

	set_pgfault_handler(pgfault);
	// 2. 父进程调用`sys_exofork()`创建子进程
	envid_t chld_id = sys_exofork();
	if (chld_id == 0) { 
		// child
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	// 3. 映射
	uintptr_t addr;
	for (addr = UTEXT; addr < USTACKTOP; addr += PGSIZE) {
		// ???
		if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & (PTE_P | PTE_U))) {
			duppage(chld_id, PGNUM(addr));
		}
	}

	// 4. 父进程为子进程设置异常栈和注册页面错误处理程序入口
	if ((err=sys_page_alloc(chld_id, (void*)(UXSTACKTOP-PGSIZE), PTE_U|PTE_W)) < 0)
		panic("fork error: %e", err);
	if ((err=sys_env_set_pgfault_upcall(chld_id, _pgfault_upcall)) < 0)
		panic("fork error: %e", err);
	
	// 5. 父进程设置子进程的状态为runnable，子进程运行
	if ((err=sys_env_set_status(chld_id, ENV_RUNNABLE)) < 0)
		panic("fork error: %e", err);
	
	return chld_id;
	// panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
