// User-level IPC library routines

#include <inc/lib.h>

// Receive a value via IPC and return it.
// If 'pg' is nonnull, then any page sent by the sender will be mapped at
//	that address.
// If 'from_env_store' is nonnull, then store the IPC sender's envid in
//	*from_env_store.
// If 'perm_store' is nonnull, then store the IPC sender's page permission
//	in *perm_store (this is nonzero iff a page was successfully
//	transferred to 'pg').
// If the system call fails, then store 0 in *fromenv and *perm (if
//	they're nonnull) and return the error.
// Otherwise, return the value sent by the sender
//
// Hint:
//   Use 'thisenv' to discover the value and who sent it.
//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
//   as meaning "no page".  (Zero is not the right value, since that's
//   a perfectly valid place to map a page.)
int32_t
ipc_recv(envid_t *from_env_store, void *pg, int *perm_store)
{
	// LAB 4: Your code here.
	// panic("ipc_recv not implemented");

	// int err;
	// if ((err=sys_ipc_recv(pg)) < 0) {
	// 	if (from_env_store) *from_env_store = 0;
	// 	if (perm_store) *perm_store = 0;
	// 	return err;
	// }
	// if (from_env_store)
	// 	*from_env_store = thisenv->env_ipc_from;
	// if (perm_store) {
	// 	if (pg) *perm_store = thisenv->env_ipc_perm;
	// 	else *perm_store = 0;
	// }
	// return thisenv->env_ipc_value;

	int result;
	
	if((result = sys_ipc_recv(pg? pg: (void*)UTOP) < 0)) {
		if(from_env_store)
			*from_env_store = 0;
		if(perm_store)
			*perm_store = 0;
		return result;
	}
	if(from_env_store)
		*from_env_store = thisenv->env_ipc_from;
	if(perm_store)
		*perm_store = thisenv->env_ipc_perm;
	//cprintf("ipc_recv from %08x to %08x value %d\n",thisenv->env_ipc_from, thisenv->env_id, thisenv->env_ipc_value);
	return thisenv->env_ipc_value;
}


// Send 'val' (and 'pg' with 'perm', if 'pg' is nonnull) to 'toenv'.
// This function keeps trying until it succeeds.
// It should panic() on any error other than -E_IPC_NOT_RECV.
//
// Hint:
//   Use sys_yield() to be CPU-friendly.
//   If 'pg' is null, pass sys_ipc_try_send a value that it will understand
//   as meaning "no page".  (Zero is not the right value.)
void
ipc_send(envid_t to_env, uint32_t val, void *pg, int perm)
{
	// LAB 4: Your code here.
	// int err;
	// void *srcva = (void*)UTOP+1;
	// if (pg) srcva = pg;

	// do {
	// 	err = sys_ipc_try_send(to_env, val, pg, perm);
	// 	if (err < 0 && err != -E_IPC_NOT_RECV)
	// 		panic("ipc_send error: %e", err);
	// 	sys_yield();
	// } while (err);
	// panic("ipc_send not implemented");

	// LAB 4: Your code here.
	int result;

	do {
		result = sys_ipc_try_send(to_env, val, pg? pg: (void*)UTOP, perm);
		if (result != 0) {
			sys_yield();
		}
	} while(result == -E_IPC_NOT_RECV);
	if (result != 0) {
		panic("ipc_send failed: %d", result);
	}
}


// Find the first environment of the given type.  We'll use this to
// find special environments.
// Returns 0 if no such environment exists.
envid_t
ipc_find_env(enum EnvType type)
{
	int i;
	for (i = 0; i < NENV; i++)
		if (envs[i].env_type == type)
			return envs[i].env_id;
	return 0;
}
