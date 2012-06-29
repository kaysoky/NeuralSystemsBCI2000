__doc__ = """

from TestRenderer import Toy

r = Toy()  # uses PygameRenderer by default

# or, for explicit renderer types:

import VisionEggRenderer
r = Toy(VisionEggRenderer.VisionEggRenderer)

# example:

r = Toy(coordinate_mapping='pixels from center')
d = r.Block(size=(20,60))
def spin(obj): obj.angle += 2.0

r.callbacks.append(pfunc(spin, obj=d))

"""

import sys, threading, time
class pfunc:
	def __init__(self, func, *pargs, **kwargs):
		self.__func = func
		self.__pargs = tuple(pargs)
		self.__kwargs = dict(kwargs)
	def __call__(self, *pargs, **kwargs):
		p = self.__pargs + tuple(pargs)
		kw = dict(self.__kwargs); kw.update(kwargs)
		return self.__func(*p, **kw)
		
class Toy(threading.Thread):
	def __init__(self, renderer_class = None, **kwargs):
		if renderer_class == None:
			import PygameRenderer
			renderer_class = PygameRenderer.PygameRenderer
		threading.Thread.__init__(self)
		self.objects = []
		self.callbacks = []
		self.screen = renderer_class()
		if len(kwargs): self.screen.setup(**kwargs)
		self.__keepgoing = True
		self.__started = False
		self.start()
		self.module = sys.modules.get(renderer_class.__module__)
		for name,cls in self.module.__dict__.items():
			if isinstance(cls, type) and hasattr(cls, 'draw'):
				setattr(self, name, pfunc(self.factory, cls=cls))
	
	def factory(self, cls, *pargs, **kwargs):
		while not self.__started: pass
		obj = cls(*pargs,**kwargs)
		self.objects.append(obj)
		return obj
	
	def run(self):
		self.screen.Initialize()
		self.__started = True
		self.__lasttime = 0.0
		self.frametimes = []
		frame_duration_sec = 1.0 / self.screen.GetFrameRate()
		while self.__keepgoing:
			good = []
			for i,c in enumerate(self.callbacks):
				try:
					c()
				except Exception,e:
					print e.__class__.__name__ + ": " + str(e)
					good.append(False)
				else:
					good.append(True)
			while False in good:
				self.callbacks.pop(good.index(False))
				good.pop(good.index(False))
			self.screen.StartFrame(self.objects)
			deadline = self.__lasttime + frame_duration_sec
			while time.time() < deadline - 0.002: time.sleep(0.001)
			self.screen.FinishFrame()
			#while time.time() < deadline - 0.001: time.sleep(0.001) # this halves the speed for reasons I cannot understand
			self.__lasttime = time.time()
			self.frametimes.append(self.__lasttime)
			if len(self.frametimes) > 100: self.frametimes.pop(0)
				
			for event in self.screen.GetEvents(): self.screen.DefaultEventHandler(event)
		self.screen.Cleanup()
		
	def stop(self):
		self.__keepgoing = False

if __name__ == '__main__':
	import BCPy2000.Paths
	self = Toy()
	d = self.Block(size=(20,60), position=(100,100))
	def spin(obj): obj.angle += 2
	self.callbacks.append(pfunc(spin, obj=d))
	try: __IPYTHON__
	except: import time; time.sleep(5); self.stop()

