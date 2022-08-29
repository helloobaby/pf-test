#include<ntifs.h>
#include<ntddk.h>
#include<wdm.h>
#include<intrin.h>

#include"ntos_sdk.h"
extern "C" {
#include"kernel-hook/khook/khook/hk.h"
}
typedef ULONG64 u64;
typedef ULONG32 u32;

template<typename... types>
void print(types... args)
{
	DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, args...);
}
void log_thread(PVOID StartContext);

extern "C" {
	NTSYSAPI PVOID RtlPcToFileHeader(
		PVOID PcValue,
		PVOID* BaseOfImage
	);
	/** 向前声明 */
	NTKERNELAPI
		UCHAR* PsGetProcessImageFileName(__in PEPROCESS Process);
	void hk_pf(void);
	void hk_pf_handler();
	u64 get_current_prcb();
	void asm_swapgs();
	PVOID ori_pf;

}
u64 ntbase;
u64 gfault_address;
u64 irql;
PEPROCESS ep;
HANDLE gkthread;
u64 mutex;
u64 current_prcb;


extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT dobj, PUNICODE_STRING preg)
{
	NTSTATUS status = STATUS_SUCCESS;
	ntbase = (u64)RtlPcToFileHeader(NtCreateFile, (PVOID*)&ntbase);
	if (!ntbase) {
		print("[-]error : ntbase not found\n");
		return STATUS_UNSUCCESSFUL;
	}

	status = HkDetourFunction((PVOID)(ntbase + Offset_KiPageFault), hk_pf, &ori_pf);
	if (!NT_SUCCESS(status))
		return status;

	status = PsCreateSystemThread(&gkthread, THREAD_ALL_ACCESS, NULL, NULL, NULL, log_thread, NULL);

	if (!NT_SUCCESS(status))
	{
		HkRestoreFunction((PVOID)(ntbase + Offset_KiPageFault), ori_pf);
		return status;
	}

	// 内核页面是只读的，并且pte中的dirty bit为0
	//
	//
	KIRQL old_irql = KeRaiseIrqlToDpcLevel();
	__writecr0(__readcr0() & (~0x10000));
	*(u32*)(ntbase + Offset_NtBuildNumber) = 0x12345678; 
	__writecr0(__readcr0() | (0x10000));
	KeLowerIrql(old_irql);
	__invlpg((void*) (ntbase + Offset_NtBuildNumber));

	print("[-]info : dv exit\n");
	return status;
}

void hk_pf_handler()
{
	u64 gs_base = __readmsr(0xC0000101);
	if((gs_base & 0xffff000000000000) == 0)
		asm_swapgs();

	print("[-]print\n");

	current_prcb = get_current_prcb();
	gfault_address = __readcr2();
	irql = __readcr8();
	ep = IoGetCurrentProcess();			
	mutex = true;
	if ((gs_base & 0xffff000000000000) == 0)
		asm_swapgs();
}

void log_thread(PVOID StartContext)
{
	while (1) {
		PAGED_CODE();


		//如果mutex等于1的话,赋0
		while (InterlockedCompareExchange64((volatile LONG64*)&mutex, 0, 1) == 0)
		{
			KeStallExecutionProcessor(0x2710u);
		}
		print("[-]fault address %llx\n", gfault_address);
		print("[-]irql %llx\n", irql);
		print("[-]current prcb block %p\n", current_prcb);
		if (MmIsAddressValid(ep))
		{
			const char* fn = (const char*)PsGetProcessImageFileName(ep);
			print("[-]process %s\n", fn);
		}
	}
}