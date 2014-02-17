import web

urls = (
    '(/[^/]+)', 'files', 
    '/trial/start', 'trial_start', 
    '/trial/stop', 'trial_stop', 
    '/trial/hit', 'trial_hit', 
)

class files:
    def GET(self, name):
        if name is None:
            name = 'index.html'
            
        return open('.%s' % name, 'rb').read()

class trial_start:
    def POST(self):
        pass
        
class trial_stop:
    def POST(self):
        pass
        
class trial_hit:
    def GET(self):
        pass
        
web.internalerror = web.debugerror
if __name__ == '__main__':
    app = web.application(urls, globals())
    app.run()