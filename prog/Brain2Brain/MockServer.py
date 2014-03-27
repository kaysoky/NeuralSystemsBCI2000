import web
import random

# Note: This server code is not meant to take the place of the BCI2000 application
#       It IS meant for testing the HTML, Javascript, & CSS without running BCI2000

urls = (
    '/(\w*\.[^/]+)?', 'files',
    '/(Countdown/\w*\.[^/]+)?', 'files',
    '/(20Questions/\w*\.[^/]+)?', 'files',
    '/trial/start', 'trial_start',
    '/trial/stop', 'trial_stop',
    '/trial/status', 'trial_status',
    '/text/answer', 'text_answer',
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

class text_answer:
    def PUT(self):
        print web.data()

web.internalerror = web.debugerror
if __name__ == '__main__':
    app = web.application(urls, globals())
    app.run()