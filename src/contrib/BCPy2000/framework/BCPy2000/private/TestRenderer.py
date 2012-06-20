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

import sys, threading
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
		while self.__keepgoing:
			good = []
			while len(self.callbacks):
				c = self.callbacks.pop(0)
				try: c()
				except Exception,e: print e.__class__.__name__ + ": " + str(e)
				else: good.append(c)
			self.callbacks += good
			self.screen.StartFrame(self.objects)
			self.screen.FinishFrame()
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

