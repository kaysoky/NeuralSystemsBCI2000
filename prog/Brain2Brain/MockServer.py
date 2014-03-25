import web
import random

urls = (
    '/(\w*\.[^/]+)?', 'files', 
    '/(Countdown/\w*\.[^/]+)?', 'files', 
    '/(20Questions/\w*\.[^/]+)?', 'files', 
    '/trial/start', 'trial_start', 
    '/trial/stop', 'trial_stop', 
    '/trial/status', 'trial_status',
    '/log', 'log_stuff',
)

class files:
    def GET(self, name=None):
        if name is None:
            name = 'index.html'
            
        return open('./%s' % name, 'rb').read()

class trial_start:
    def POST(self):
        return random.randint(0, 1)
        
class trial_stop:
    def POST(self):
        pass
        
class trial_status:
    def GET(self):
        pass
        
class log_stuff:
    def POST(self):
        print web.data()
        
web.internalerror = web.debugerror
if __name__ == '__main__':
    app = web.application(urls, globals())
    app.run()