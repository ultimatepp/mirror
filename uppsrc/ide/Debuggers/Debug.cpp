#include "Debuggers.h"

#ifdef PLATFORM_WIN32

#define STATUS_WX86_CONTINUE             0x4000001D
#define STATUS_WX86_SINGLE_STEP          0x4000001E
#define STATUS_WX86_BREAKPOINT           0x4000001F
#define STATUS_WX86_EXCEPTION_CONTINUE   0x40000020
#define STATUS_WX86_EXCEPTION_LASTCHANCE 0x40000021
#define STATUS_WX86_EXCEPTION_CHAIN      0x40000022

#define LLOG(x)  // DLOG(x)

String Pdb::Hex(adr_t a)
{
	return Format64Hex(a);
}

void Pdb::Error(const char *s)
{
	String txt = "Error!&";
	if(s)
		txt << s << "&";
	LLOG("ERROR: " << DeQtf(GetLastErrorMessage()));
	Exclamation(txt + DeQtf(GetLastErrorMessage()));
	running = false;
	Stop();
}

void Pdb::UnloadModuleSymbols()
{
	for(int i = 0; i < module.GetCount(); i++) {
		ModuleInfo& f = module[i];
		if(f.symbols) {
			SymUnloadModule64(hProcess, f.base);
			LLOG("Unloaded symbols for " << f.path << ' ' << Hex(module[i].base) << '/' << hProcess);
		}
	}
}

int  Pdb::FindModuleIndex(adr_t base)
{
	for(int i = 0; i < module.GetCount(); i++)
		if(module[i].base == base)
			return i;
	return -1;
}

void Pdb::LoadModuleInfo()
{
	fninfo_cache.Clear();

	ModuleInfo f;
	dword cb = 1;
	HMODULE  h;
	if(!EnumProcessModules(hProcess, &h, sizeof(HMODULE), &cb)) {
		Error();
		return;
	}
	int n = cb / sizeof(HMODULE);
	Buffer<HMODULE> m(n);
	if(!EnumProcessModules(hProcess, m, cb, &cb)) {
		Error();
		return;
	}
	Vector<ModuleInfo> nm;
	for (int i = 0; i < n; i++) {
		MODULEINFO mi;
		if(GetModuleInformation(hProcess, m[i], &mi, sizeof(mi))) {
			ModuleInfo& f = nm.Add();
			f.base = (adr_t)mi.lpBaseOfDll;
			f.size = mi.SizeOfImage;
			int q = FindModuleIndex(f.base);
			if(q >= 0) {
				ModuleInfo& of = module[q];
				f.path = of.path;
				f.symbols = of.symbols;
				of.symbols = false;
				LLOG("Stable " << Hex(f.base) << " (" << Hex(f.size) << "): " << f.path);
			}
			else {
				char name[MAX_PATH];
				if(GetModuleFileNameEx(hProcess, m[i], name, MAX_PATH)) {
					f.path = name;
					adr_t w = (adr_t)SymLoadModule64(hProcess, NULL, name, 0, f.base, f.size);
					if(w) {
						LLOG("Loading symbols " << name << ' ' << Hex(f.base) << '/' << hProcess << " returned base " << Hex(w));
						f.symbols = true;
						LoadGlobals(w);
					}
				}
				LLOG(Hex(f.base) << " (" << Hex(f.size) << "): " << f.path);
			}
		}
	}
	UnloadModuleSymbols();
	module = pick(nm);
	refreshmodules = false;
}

bool Pdb::AddBp(adr_t address)
{
	LLOG("AddBp: " << Hex(address));
	if(bp_set.Find(address) >= 0)
		return true;
	byte prev;
	if(!ReadProcessMemory(hProcess, (LPCVOID) address, &prev, 1, NULL))
		return false;
	LLOG("ReadProcessMemory OK");
	byte int3 = 0xcc;
	if(!WriteProcessMemory(hProcess, (LPVOID) address, &int3, 1, NULL))
		return false;
	LLOG("WriteProcessMemory OK");
//	FlushInstructionCache (hProcess, (LPCVOID)address, 1);
	bp_set.Put(address, prev);
	return true;
}

bool Pdb::RemoveBp(adr_t address)
{
	int pos = bp_set.Find(address);
	if(pos < 0)
		return true;
	if(!WriteProcessMemory(hProcess, (LPVOID)address, &bp_set[pos], 1, NULL))
		return false;
	FlushInstructionCache(hProcess, (LPCVOID)address, 1);
	bp_set.Unlink(pos);
	return true;
}

bool Pdb::RemoveBp()
{
	for(int i = bp_set.GetCount(); --i >= 0;)
		if(!bp_set.IsUnlinked(i)) {
			adr_t address = bp_set.GetKey(i);
			if(!WriteProcessMemory(hProcess, (LPVOID)address, &bp_set[i], 1, NULL))
				return false;
			FlushInstructionCache(hProcess, (LPCVOID)address, 1);
			bp_set.Unlink(i);
		}
	bp_set.Clear();
	return true;
}

void Pdb::SyncFrameButtons()
{
	int ii = framelist.GetIndex();
	frame_down.Enable(!lock && ii >= 0 && ii < framelist.GetCount() - 1);
	frame_up.Enable(!lock && ii > 0);
}

void Pdb::Lock()
{
	if(lock == 0) {
		IdeHidePtr();
		IdeDebugLock();
		watches.Disable();
		locals.Disable();
		framelist.Disable();
		SyncFrameButtons();
		frame_down.Disable();
		frame_up.Disable();
		dlock.Show();
		IdeSetBar();
	}
	lock++;
}

void Pdb::Unlock()
{
	lock--;
	if(lock == 0) {
		IdeDebugUnLock();
		watches.Enable();
		locals.Enable();
		framelist.Enable();
		SyncFrameButtons();
		dlock.Hide();
		IdeSetBar();
	}
}

Pdb::Context Pdb::ReadContext(HANDLE hThread)
{
	DR_LOG("ReadContext");
	Context r;
#ifdef CPU_64
	if(win64) {
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_FULL;
		if(!GetThreadContext(hThread, &ctx))
			Error("GetThreadContext failed");
		memcpy(&r.context64, &ctx, sizeof(CONTEXT));
	}
	else {
		static BOOL (WINAPI *Wow64GetThreadContext)(HANDLE hThread, PWOW64_CONTEXT lpContext);
		ONCELOCK
			DllFn(Wow64GetThreadContext, "Kernel32.dll", "Wow64GetThreadContext");
		
		WOW64_CONTEXT ctx;
		ctx.ContextFlags = WOW64_CONTEXT_FULL;
		if(!Wow64GetThreadContext || !Wow64GetThreadContext(hThread, &ctx))
			Error("Wow64GetThreadContext failed");
		memcpy(&r.context32, &ctx, sizeof(WOW64_CONTEXT));
	}
#else
	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_FULL;
	if(!GetThreadContext(hThread, &ctx))
			Error("GetThreadContext failed");
	memcpy(&r.context32, &ctx, sizeof(CONTEXT));
#endif
	return r;
}

void Pdb::WriteContext(HANDLE hThread, Context& context)
{
	DR_LOG("WriteContext");
#ifdef CPU_64
	if(win64) {
		CONTEXT ctx;
		memcpy(&ctx, &context.context64, sizeof(CONTEXT));
		ctx.ContextFlags = CONTEXT_CONTROL;
		if(!SetThreadContext(hThread, &ctx))
			Error("SetThreadContext failed");
	}
	else {
		static BOOL (WINAPI *Wow64SetThreadContext)(HANDLE hThread, PWOW64_CONTEXT lpContext);
		DllFn(Wow64SetThreadContext, "Kernel32.dll", "Wow64SetThreadContext");

		WOW64_CONTEXT ctx;
		memcpy(&ctx, &context.context32, sizeof(WOW64_CONTEXT));
		ctx.ContextFlags = CONTEXT_CONTROL;
		if(!Wow64SetThreadContext || !Wow64SetThreadContext(hThread, &ctx))
			Error("Wow64SetThreadContext failed");
	}
#else
	CONTEXT ctx;
	memcpy(&ctx, &context.context32, sizeof(WOW64_CONTEXT));
	ctx.ContextFlags = CONTEXT_CONTROL;
	if(!SetThreadContext(hThread, &ctx))
		Error("SetThreadContext failed");
#endif
}

void Pdb::AddThread(dword dwThreadId, HANDLE hThread)
{
	if(threads.Find(dwThreadId) >= 0)
		return;
	DR_LOG("AddThread");
	Thread& f = threads.GetAdd(dwThreadId);
	// Retrive "base-level" stack-pointer, to have limit for stackwalks:
	Context c = ReadContext(hThread);
#ifdef CPU_64
	f.sp = win64 ? c.context64.Rsp : c.context32.Esp;
#else
	f.sp = c.context32.Esp;
#endif
	f.hThread = hThread;
	LLOG("Adding thread " << dwThreadId << ", Thread SP: " << Hex(f.sp) << ", handle: " << FormatIntHex((dword)(hThread)));
}

void Pdb::RemoveThread(dword dwThreadId)
{
	int q = threads.Find(dwThreadId);
	if(q >= 0) {
		Thread& f = threads[q];
		LLOG("Closing thread " << dwThreadId << ", handle: " << FormatIntHex((dword)(f.hThread)));
		CloseHandle(f.hThread);
		threads.Remove(q);
	}
}

#define EXID(id)       { id, #id },

struct {
	adr_t code;
	const char *text;
}
ex_desc[] = {
	EXID(EXCEPTION_ACCESS_VIOLATION)
	EXID(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
	EXID(EXCEPTION_DATATYPE_MISALIGNMENT)
	EXID(EXCEPTION_FLT_DENORMAL_OPERAND)
	EXID(EXCEPTION_FLT_DIVIDE_BY_ZERO)
	EXID(EXCEPTION_FLT_INEXACT_RESULT)
	EXID(EXCEPTION_FLT_INVALID_OPERATION)
	EXID(EXCEPTION_FLT_OVERFLOW)
	EXID(EXCEPTION_FLT_STACK_CHECK)
	EXID(EXCEPTION_FLT_UNDERFLOW)
	EXID(EXCEPTION_ILLEGAL_INSTRUCTION)
	EXID(EXCEPTION_IN_PAGE_ERROR)
	EXID(EXCEPTION_INT_DIVIDE_BY_ZERO)
	EXID(EXCEPTION_INT_OVERFLOW)
	EXID(EXCEPTION_INVALID_DISPOSITION)
	EXID(EXCEPTION_NONCONTINUABLE_EXCEPTION)
	EXID(EXCEPTION_PRIV_INSTRUCTION)
	EXID(EXCEPTION_STACK_OVERFLOW )
};

void Pdb::RestoreForeground()
{
	if(hWnd) {
		SetForegroundWindow(hWnd);
		LLOG("Restored foreground window: " << FormatIntHex((dword)hWnd));
	}
	hWnd = NULL;
}

void Pdb::SaveForeground()
{
	HWND hwnd = GetForegroundWindow();
	if(hwnd) {
		DWORD pid;
		GetWindowThreadProcessId(hwnd, &pid);
		if(pid == processid) {
			hWnd = hwnd;
			LLOG("Saved foreground window: " << Hex((adr_t)hWnd));
		}
	}
}

void Pdb::ToForeground()
{
	TopWindow *w = GetTopWindow();
	if(w && !w->IsForeground()) {
		LLOG("Setting theide as foreground");
		w->SetForeground();
	}
}

bool Pdb::RunToException()
{
	DR_LOG("RunToException");
	LLOG("RUN TO EXCEPTION");
	TimeStop ts;
	bool disasfocus = disas.HasFocus();
	bool locked = false;
	bool frestored = false;
	invalidpage.Clear();
	mempage.Clear();
	int opn = 0;
	for(;;) {
		if(terminated) {
			if(locked)
				Unlock();
			return false;
		}
		opn++;
		DR_LOG("WaitForDebugEvent");
		if(WaitForDebugEvent(&event, 0)) {
			DR_LOG("WaitForDebugEvent ended");
			debug_threadid = event.dwThreadId;
			opn = 0;
			running = false;
			switch(event.dwDebugEventCode) {
			case EXCEPTION_DEBUG_EVENT: {
				DR_LOG("EXCEPTION_DEBUG_EVENT");
				LLOG("Exception: " << FormatIntHex(event.u.Exception.ExceptionRecord.ExceptionCode) <<
				     " at: " << FormatIntHex(event.u.Exception.ExceptionRecord.ExceptionAddress) <<
				     " first: " << event.u.Exception.dwFirstChance);
				SaveForeground();
				const EXCEPTION_RECORD& x = event.u.Exception.ExceptionRecord;
				if(findarg(x.ExceptionCode, EXCEPTION_BREAKPOINT, EXCEPTION_SINGLE_STEP,
				                            STATUS_WX86_BREAKPOINT, STATUS_WX86_SINGLE_STEP) < 0)
				{
					LLOG("Non-debug EXCEPTION");
					if(event.u.Exception.dwFirstChance) {
						LLOG("First chance " << FormatIntHex(x.ExceptionCode));
						break;
					}
					String desc = Format("Exception: [* %lX] at [* %16llX]&",
					                     (int64)x.ExceptionCode, (int64)x.ExceptionAddress);
					for(int i = 0; i < __countof(ex_desc); i++)
						if(ex_desc[i].code == x.ExceptionCode)
							desc << "[* " << DeQtf(ex_desc[i].text) << "]&";
					if(x.ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
						desc << (x.ExceptionInformation[0] ? "[*@3 writing]" : "[*@4 reading]");
						desc << Format(" at [* %08llX]", (int64)x.ExceptionInformation[1]);
					}
					ToForeground();
					BeepError();
					if(first_exception) {
						first_exception = false;
						ErrorOK(desc);
					}
					else
					if(!Prompt(Ctrl::GetAppName(), CtrlImg::error(), desc, t_("OK"), t_("Stop"))) {
						Stop();
						return false;
					}
				}
#ifdef CPU_64
				if(!win64 && x.ExceptionCode == EXCEPTION_BREAKPOINT && !break_running) // Ignore x64 breakpoint in wow64
					break;
#endif
				if(break_running)
					debug_threadid = mainThreadId;
				break_running = false;
				ToForeground();
				if(disasfocus)
					disas.SetFocus();
				if(locked)
					Unlock();
				if(refreshmodules)
					LoadModuleInfo();
				LLOG("event.dwThreadId = " << event.dwThreadId);
				bool isbreakpoint = findarg(x.ExceptionCode, EXCEPTION_BREAKPOINT, STATUS_WX86_BREAKPOINT) >= 0;
				for(int i = 0; i < threads.GetCount(); i++) {
					Thread& t = threads[i];
					(Context&)t = ReadContext(threads[i].hThread);
					if(event.dwThreadId == threads.GetKey(i)) {
						LLOG("Setting current context");
						if(isbreakpoint
#ifdef CPU_64
						   && bp_set.Find((win64 ? t.context64.Rip : t.context32.Eip) - 1) >= 0
#else
						   && bp_set.Find(t.context32.Eip - 1) >= 0
#endif
						) { // We have stopped at breakpoint, need to move address back
					#ifdef CPU_64
							if(win64)
								t.context64.Rip--;
							else
					#endif
								t.context32.Eip--;
						}
						context = t;
					}
				}
				RemoveBp();
				return true;
			}
			case CREATE_THREAD_DEBUG_EVENT:
				DR_LOG("CREATE_THREAD_DEBUG_EVENT");
				LLOG("Create thread: " << event.dwThreadId);
				AddThread(event.dwThreadId, event.u.CreateThread.hThread);
				break;
			case EXIT_THREAD_DEBUG_EVENT:
				DR_LOG("EXIT_THREAD_DEBUG_EVENT");
				LLOG("Exit thread: " << event.dwThreadId);
				RemoveThread(event.dwThreadId);
				break;
			case CREATE_PROCESS_DEBUG_EVENT:
				DR_LOG("CREATE_PROCESS_DEBUG_EVENT");
				LLOG("Create process: " << event.dwProcessId);
				processid = event.dwProcessId;
				AddThread(event.dwThreadId, event.u.CreateProcessInfo.hThread);
				CloseHandle(event.u.CreateProcessInfo.hFile);
				CloseHandle(event.u.CreateProcessInfo.hProcess);
				break;
			case EXIT_PROCESS_DEBUG_EVENT:
				DR_LOG("EXIT_PROCESS_DEBUG_EVENT");
				LLOG("Exit process: " << event.dwProcessId);
				if(locked)
					Unlock();
				Stop();
				return false;
			case LOAD_DLL_DEBUG_EVENT: {
				DR_LOG("LOAD_DLL_DEBUG_EVENT");
				LLOG("Load dll: " << event.u.LoadDll.lpBaseOfDll);
				CloseHandle(event.u.LoadDll.hFile);
				refreshmodules = true;
				break;
			}
			case UNLOAD_DLL_DEBUG_EVENT:
				DR_LOG("UNLOAD_DLL_DEBUG_EVENT");
				LLOG("UnLoad dll: " << event.u.UnloadDll.lpBaseOfDll);
				refreshmodules = true;
				break;
			case RIP_EVENT:
				DR_LOG("RIP_EVENT");
				LLOG("RIP!");
				Exclamation("Process being debugged died unexpectedly!");
				if(locked)
					Unlock();
				Stop();
				return false;
			}
			DR_LOG("ContinueDebugEvent");
			ContinueDebugEvent(event.dwProcessId, event.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
			running = true;
		}
		if(ts.Elapsed() > 200) {
			DR_LOG("ts.Elpsed() > 200");
			if(!lock) {
				Lock();
				locked = true;
			}
			if(!frestored) {
				RestoreForeground();
				frestored = true;
			}
		}
		if(lock) {
			DR_LOG("GuiSleep");
			GuiSleep(opn < 1000 ? 0 : 100);
			Ctrl::ProcessEvents();
		}
		else {
			DR_LOG("Sleep");
			Sleep(opn < 1000 ? 0 : 100);
		}
	}
}

/*
const CONTEXT& Pdb::CurrentContext()
{
	return threads.Get((int)~threadlist).context;
}
*/

void Pdb::WriteContext()
{
	WriteContext(threads.Get(event.dwThreadId).hThread, context);
}

bool Pdb::SingleStep()
{
	LLOG("SINGLE STEP 0");
#if CPU_64
	if(win64)
		context.context64.EFlags |= 0x100;
	else
#endif
		context.context32.EFlags |= 0x100;
	WriteContext();
	running = true;
	ContinueDebugEvent(event.dwProcessId, event.dwThreadId, DBG_CONTINUE);
	if(!RunToException())
		return false;
#if CPU_64
	if(win64)
		context.context64.EFlags &= ~0x100;
	else
#endif
		context.context32.EFlags &= ~0x100;
	WriteContext();
	return true;
}

bool Pdb::Continue()
{
	LLOG("** Continue");
	running = true;
	ContinueDebugEvent(event.dwProcessId, event.dwThreadId, DBG_CONTINUE);
	return RunToException();
}

void Pdb::SetBreakpoints()
{
	RemoveBp();
	for(int i = 0; i < breakpoint.GetCount(); i++)
		AddBp(breakpoint[i]);
}

void Pdb::BreakRunning() //TODO: Fix in wow64?
{
	stop = true;
	if(running) {
		BOOL (WINAPI *debugbreak)(HANDLE Process);
		debugbreak = (BOOL (WINAPI *)(HANDLE))
		             GetProcAddress(GetModuleHandle("kernel32.dll"), "DebugBreakProcess");
		if(debugbreak) {
			LLOG("=== DebugBreakProcess");
			break_running = true;
			(*debugbreak)(hProcess);
		}
		else
			Exclamation("Operation is not supported on this OS");
	}
}

#endif
