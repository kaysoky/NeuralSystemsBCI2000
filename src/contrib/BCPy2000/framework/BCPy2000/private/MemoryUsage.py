__all__ = ['memuse', 'monitor_memuse']

memuse = None

import platform
if platform.system().lower() == 'windows': 
	import ctypes

	GetProcessMemoryInfo = ctypes.windll.psapi.GetProcessMemoryInfo
	GetCurrentProcess = ctypes.windll.kernel32.GetCurrentProcess
	GetLastError = ctypes.windll.kernel32.GetLastError
	
	DWORD = ctypes.c_uint32
	SIZE_T = ctypes.c_ulong
	HANDLE = ctypes.c_voidp
	
	class PROCESS_MEMORY_COUNTERS(ctypes.Structure):
		_pack_ = 1
		_fields_ = [
			('cb',                         DWORD),
			('PageFaultCount',             DWORD),
			('PeakWorkingSetSize',         SIZE_T),
			('WorkingSetSize',             SIZE_T),
			('QuotaPeakPagedPoolUsage',    SIZE_T),
			('QuotaPagedPoolUsage',        SIZE_T),
			('QuotaPeakNonPagedPoolUsage', SIZE_T),
			('QuotaNonPagedPoolUsage',     SIZE_T),
			('PagefileUsage',              SIZE_T),
			('PeakPagefileUsage',          SIZE_T),
		]
	
	def memuse(field='WorkingSetSize', units=1024):
		s = PROCESS_MEMORY_COUNTERS(0,0,0,0,0,0,0,0,0,0)
		ret = GetProcessMemoryInfo(
			HANDLE(GetCurrentProcess()),
			ctypes.byref(s),
			DWORD(ctypes.sizeof(PROCESS_MEMORY_COUNTERS)),
		)
		if ret == 0: raise WindowsError,"failed to get process memory info: error %d" % GetLastError()
		if field == None:
			m = {}
			for f in s._fields_: m[f[0]] = getattr(s, f[0])
			return m
		return float(getattr(s, field)) / units

if memuse == None:
	print __name__,'module could find an implementation for memuse---only supported for Win32 at the moment'




def monitor_memuse(bci, fac=0.2, wait=2.0, statemonitors=False, bar=True):
	
	bci._memuse = d = {}

	if statemonitors:
		import AppTools.StateMonitors
		m = AppTools.StateMonitors.addstatemonitor(bci, 'memory leak')
		m.func = lambda x: '% 6d bytes/sec' % x._memuse['persec']; m.pargs = (bci,)
		m = AppTools.StateMonitors.addstatemonitor(bci, ' memory leak')
		m.func = lambda x: '% 6d bytes/frame' % x._memuse['perframe']; m.pargs = (bci,)
		m = AppTools.StateMonitors.addstatemonitor(bci, '  memory leak')
		m.func = lambda x: '% 6d bytes/packet' % x._memuse['perpacket']; m.pargs = (bci,)

	if bar:
		import AppTools.Meters
		font_name = getattr(bci.screen, 'monofont', None)
		d['bar'] = AppTools.Meters.addbar(bci,pos=(40,10), fac=fac, fliplr=1, fmt='%+ 6dK', font_name=font_name, font_size=14)
		
	d['persec'] = d['perframe'] = d['perpacket'] = 0
	d['wait'] = int(round(wait * bci.nominal['PacketsPerSecond']))
	d['subtract_frames'] = d['subtract_msec'] = 0
	bci.add_callback('Process', memuse_update, (bci,))

def memuse_update(bci):
	d = bci._memuse
	elapsed = bci.since('run')
	pc = elapsed['packets'] - d['wait']
	m = memuse(units=1)
	if pc <= 0:
		d['start'] = m
		d['subtract_msec'] = elapsed['msec']
		d['subtract_frames'] = elapsed['frames']
		d['persec'] = d['perframe'] = d['perpacket'] = 0
	else:
		md = float(m - d['start'])
		d['persec'] =  md / max(1.0, elapsed['msec'] - d['subtract_msec']) * 1000.0
		d['perframe'] = md / max(1.0, elapsed['frames'] - d['subtract_frames'])
		d['perpacket'] = md / max(1.0, float(pc))	
		
	if d.has_key('bar'):
		d['bar'].set((m-d['start']) / 1024.0)
