#!/usr/bin/python
"""
Run this with netcat -lp $PORTNUM -e $THIS_SCRIPT
Open $PORTNUM in the firewall if necessary.
Access from a browser with http://localhost:$PORTNUM

If the browser appears to hang, (1) check that the
server hasn't crashed. If it has crashed, you can
examine the error by relaunching it, connecting to
it with `netcat localhost $PORTNUM` from another
terminal, and typing 'GET /' followed by two
carriage-returns. If it's crashing without an error,
the connection may have timed out. If the server is
not crashing, (2) try refreshing the page, (3) check
that no other clients are trying to connect, (4) try
quitting and relaunching the browser.

A Windoze version of netcat (a.k.a. nc) is available
from http://joncraton.org/files/nc111nt.zip
"""###

import sys
import urllib # only needed for unquote_plus (unescaping url-encoded characters)

def uncrlf(t):
	return t.replace('\r\n','\n').replace('\r','\n')
def urldecode(t):
	return uncrlf(urllib.unquote_plus(t))
	
def HttpRequest(stream=sys.stdin):
	header = stream.readline()
	cmd = header.split() # first line of request contains http method, url and protocol version
	cmd += [''] * (3 - len(cmd))
	cmd = cmd[:2] + [' '.join(cmd[2:])]
	req = dict(zip(('method', 'url', 'protocol'), cmd[:3]))
	# parse any input submitted from html forms via GET
	req['url'],sep,data = req['url'].partition('?')
	# read the rest of the request header
	while True:
		line = stream.readline()
		if not len(line.rstrip()): break # blank line indicates end of request header
		header += line
		key,sep,val = line.partition(':')
		req[key.lower()] = val.strip()
	# parse any input submitted from html forms via POST
	cl = int(req.get('content-length',0))
	#if cl and req['method'] == 'POST': data = stream.read(cl)
	# NB if the form has the enctype="multipart/form-data" (e.g. for uploading file content) then the posted data will be &= delimited
	if cl and req['method'] == 'POST': data += '&' + stream.read(cl)
	if len(data): data = dict([p.split('=') for p in data.split('&') if len(p.strip())])
	else: data = {}
	for k,v in data.items(): data[k] = urldecode(v)
	return header,req,data
	
def HttpResponse(content, result='200 OK', type='text/html', stream=sys.stdout, **extraHeaders):
	content = str(content) + '\n'
	stream.write("HTTP/1.1 %s\n" % result)
	stream.write("Content-Length: %d\n" % len(content)) # without this the client may hang until the server closes stdout
	stream.write("Content-Type: %s\n" % type)
	for kv in extraHeaders.items(): stream.write("%s: %s\n" % kv)
	stream.write("\n") # blank line indicates end of response header
	stream.write(content)
	stream.flush()

def legalname(name):
	return ''.join([x for x in name if x.isalnum() or x == '_'])

class tag(object):
	def __init__(self, tagtype, children=[], parent=None, **attributes):
		self.tagtype = tagtype
		self.attributes = attributes
		if not isinstance(children,(tuple,list)): children = [children]
		self.children = list(children)
		if parent != None: parent.add(self)
		self.legalize()
	def legalize(self):
		for a in ['name', 'id']:
			if a in self.attributes:
				self.attributes[a] = legalname(self.attributes[a])
	def __str__(self):
		self.legalize()
		s = '<' + self.tagtype + ''.join([' %s="%s"'%(k.rstrip('_'),str(v)) for k,v in self.attributes.items() if v != None and v != ''])
		if len(self.children) == 0:  return s + '/>'
		s += '>'
		for c in self.children:
			if c != None: s += '\n' + str(c)
		return s + '\n</' + self.tagtype + '>'
	def __repr__(self): return str(self)
	def add(self, *pargs):
		for x in pargs:
			if isinstance(x, (tuple,list)): self.children += x
			elif x != None: self.children.append(x)
	def __iadd__(self, x):
		self.add(x)
		return self
	def tag(self, *pargs, **kwargs):
		t = tag(*pargs, **kwargs)
		self.children.append(t)
		return t

def HtmlEscape(t):
	return t.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')

def paramtags(p, name, parent, full=False):
	row   = parent.tag('tr',             class_='parameter')
	label = row.tag(   'td', [name+':'], class_='parameterlabel')
	cell  = row.tag(   'td',             class_='parameterfields') 
	input = cell.tag('input', type='text', name=name, value=p['valstr'], size=100) # TODO: move physical styling "size" to style section?
	return row

def dialog(pp):
	if pp == None or len(pp) == 0: return None
	if isinstance(pp,str): pp = pp.splitlines()
	pp = [FileReader.ParseParam(p) for p in pp if len(p.strip())]
	form = tag('form', class_='config', action="/SetParameters", method="POST")
	buttons = form.tag('fieldset', class_='buttons')
	buttons.tag('input', type='submit', name='submit', value='Done')
	buttons.tag('hr')
	buttons.tag('input', type='submit', name='submit', value='Load Parameters')
	buttons.tag('input', type='submit', name='submit', value='Save Parameters')
	buttons.tag('hr')
	buttons.tag('input', type='submit', name='submit', value='Configure Loading')
	buttons.tag('input', type='submit', name='submit', value='Configure Saving')
	d0 = {}; ok = 'order of keys'
	for p in pp:
		prm = p['name']
		tab,sec,fil = p['category']
		o0 = d0[ok]  = d0.get(ok, [])
		if not tab in o0: o0.append(tab)
		d1 = d0[tab] = d0.get(tab,{})
		o1 = d1[ok]  = d1.get(ok, [])
		if not sec in o1: o1.append(sec)
		d2 = d1[sec] = d1.get(sec,{})
		o2 = d2[ok]  = d2.get(ok, [])
		if not fil in o2: o2.append(fil)
		d3 = d2[fil] = d2.get(fil,{})
		o3 = d3[ok]  = d3.get(ok, [])
		if not prm in o3: o3.append(prm)
		d3[prm] = p
	if 'Storage' in d0[ok]: firstVisible = 'Storage'
	else: firstVisible = d0[ok][0]
	for tabname in d0[ok]:
		d1 = d0[tabname]
		if tabname == '': tabname = 'none'
		TabFieldsetClass = {True:'tab', False:'itab'}.get(tabname==firstVisible)
		tab = form.tag('fieldset', class_=TabFieldsetClass, id='TabFieldset_'+tabname)
		tabbar = tab.tag('legend', class_='tab')
		
		for tn in d0[ok]:
			if tn == tabname:
				tabbar.tag('a', tn, href="#", name='TabAnchor_'+tabname, class_="selectedtab")
			else:
				src = legalname('TabFieldset_'+tabname)
				dst = legalname('TabFieldset_'+tn)
				switchcmd = "YieldVisibility('%s','%s')" % (src,dst)
				tabbar.tag('a', tn, href='javascript:'+switchcmd, name='', class_="unselectedtab")
			
		for secname in d1[ok]:
			d2 = d1[secname]
			if secname == '': secname = 'none'; cl = 'phantom'
			else: cl = 'section'
			sec = tab.tag('fieldset', class_=cl, id='SecFieldset_'+secname)
			if secname != 'none':
				togglecmd = "ToggleVisibility('%s');" % legalname('SecTable_'+secname)
				sec.tag('legend', class_='section').tag('a', secname, class_='section', href='javascript:'+togglecmd)
			table = sec.tag('table', class_='section', id="SecTable_"+secname)
			for filname in d2[ok]:
				d3 = d2[filname]
				if filname == '': filname = 'none'
				fil = table
				for prmname in d3[ok]:
					p = d3[prmname]
					prm = paramtags(p, name=prmname, parent=fil, full=False)
					
	return form

css = """<!-- 
.buttons { position: fixed; right: 5px; top: 10px; width: 120px; text-align: center; background: #6a6; z-index: 2;}
legend.tab       { background: #ccc; }
legend.section   { background: #fff; }
fieldset.tab, fieldset.itab { background: #8a8; padding-top: 20px; position: absolute; right: 150px; left: 10px; top: 10px; }
fieldset.itab    { display: none; }
fieldset.section { background: #aea; }
fieldset.phantom { border-style: none; }
table.section tr { background: #eee; }
table, table td, table th { border-style: none; }
       table td, table th { padding: 2px; }

legend a {
	text-decoration: none;
	font-weight:     normal;
	color:           #444;
	background:      #ccc;
}
legend a.selectedtab, legend a.section  { color: #000; background: #fff; }
legend a:visited       { color: inherit; text-decoration: inherit; }
legend a:hover         { color: inherit; text-decoration: inherit;}
legend a.unselectedtab:hover  { background: #ddd; }
legend.tab a, legend.itab a { padding-left: 1em; padding-right: 1em; }
td.parameterlabel  { text-align: right; width: 300px;}
td.parameterfields { text-align: left; }

// -->"""###

js = """<!-- 
function HideContent(d) {
	if(d.length < 1) { return; }
	document.getElementById(d).style.display = "none";
}
function ShowContent(d) {
	if(d.length < 1) { return; }
	document.getElementById(d).style.display = "block";
}
function ToggleVisibility(d) {
	if(d.length < 1) { return; }
	if(document.getElementById(d).style.display == "none") { ShowContent(d); }
	else { HideContent(d); }
}
function YieldVisibility(fromTab, toTab) {
	HideContent(fromTab);
	ShowContent(toTab);
}
// -->"""###

if __name__ == '__main__':
	import os
	bcpypath = os.path.realpath(os.path.join(os.getcwd(), '..'))
	sys.path.append(os.path.join(bcpypath, 'tools', 'BCI2000Tools'))
	defaultfile = os.path.join(bcpypath, 'demo', 'parms', 'PythonDemo1_Triangle.prm')
	defaultfile = os.path.realpath(os.path.join(bcpypath, '..', '..', '..', '..', '..', '..', 'jez', 'audiostream', 'parms', 'audiostream_EEG67+EOG3+SYNC2+VMRK_Quickamp_500Hz.prm'))
	import FileReader # only needed for parsing prm format into a format python can work with (redundantly reimplements what is done in the C)
	
	instream,outstream = sys.stdin,sys.stdout
	while not instream.closed:
		header,req,data = HttpRequest(instream)
						
		contentType = "text/plain"   # default content type
		use_html = not req['url'].lower().endswith('.txt') # but let's plan on changing that unless the requested URL explicitly says don't	
		content = header  # parrot back the full text of the http request header
		#content = '\n'.join([str(x) for x in req.items() + data.items()])
		if use_html:
			contentType = "text/html"

			html = tag('html')
			head = html.tag('head')
			title = head.tag('title', 'Welcome to the HTTP Parrot')
			style = head.tag('style', css, type='text/css')
			script = head.tag('script', js, type="text/javascript", language="JavaScript1.1") # TODO: language= is non-strict
			body = html.tag('body')
			#body += HtmlEscape(content).replace('\n', '<br/>\n'),tag('hr')
			content = html

			if req['url'] == '/Test':
				req['url'] = '/Config'
				req['method'] = 'POST'
				data = {'serverside_prmpath':defaultfile}
				
			if req['url'].lower() == '/config' and req['method'] == 'POST':
				prm_content = data.get('prm_content', '').splitlines()
				serverside_prmpath = data.get('serverside_prmpath', '')
				if len(serverside_prmpath):
					a = open(serverside_prmpath)
					prm_content += a.readlines();
					a.close()
				if len(prm_content):
					body += dialog(prm_content)
			else:
				form = body.tag('form', action="/Config", method="POST")
				form += 'Path to server-side prm file:'
				form += tag('br')
				form += tag('input', name='serverside_prmpath', type='text', size=120, value=defaultfile)
				form += tag('br')
				form += tag('br')
				form += 'And/or raw prm content:'
				form += tag('br')
				form += tag('textarea', None, name='prm_content', rows=3, cols=120)
				form += tag('br')
				form += tag('input', name='submit', type='submit', value='submit')
				form += tag('input', name='reset',  type='reset', value='clear')
				form += tag('hr')

			if req['url'].lower() == '/setparameters':
				data.pop('submit', '')
				output = body.tag('pre', ['SET PARAMETER %s %s' % kv for kv in data.items()])
		
		HttpResponse(content, type=contentType, stream=outstream)


"""
LOAD PARAMETERFILE <filename>
SETCONFIG
INSERT STATE <state_name> <bit_width> <initial_value>
SET STATE <state_name> <value>
QUIT
SYSTEM <command_string>


Proposed extensions to the scripting language
=============================================
These should allow the Operator to be reasonably usable as
a command-line tool via just stdin/stdout/stderr.

SET PARAMETER <parameter_name> <value_string>
	Stores the value string in association with the named parameter.
	If it differs from the previous value, set a flag to indicate that SETCONFIG is required.
	Pending parameter values will be type- and value-checked next time SETCONFIG is performed.
	LOAD PARAMETERFILE could be refactored as a wrapper around the translation of prm-format
	lines into, and execution of, SET PARAMETER commands.
	SET PARAMETER is useful for the command-line user who wants to load a parameter file and
	then just change one or two things (perhaps in response to Preflight errors).

EXEC <script_filename>
	Read operator script commands from a file and execute them line-by-line.

LOADING INCLUDES <parameter_name>
LOADING EXCLUDES <parameter_name>
SAVING INCLUDES <parameter_name>
SAVING EXCLUDES <parameter_name>
	Four commands for defining the parameter filters for loading and saving.
	By default, loading and saving include all parameters. Passing '*' as <parameter_name>
	includes or excludes all.

GET <http_request>
POST <http_request>
	These are interpreted exactly as http requests. The server continues to read from stdin
	until a blank line is received, terminating the request header, then reads any POSTed
	data if appropriate. Posted data are translated into a series of operator script commands
	which are then run. This should allow us to investigate very easily how feasible it is
	to implement the operator's user interface as an http server serving html forms. Html
	would allow great flexibility in implementing the GUI, since an external web browser or
	any one of a number of html rendering libraries could be used.
	For proof-of-concept and early stages of development, a web browser pointed at
	http://localhost:8080 and the command `netcat -lp 8080 -e prog` can be used to turn a
	stdin/stdout program prog into a working webserver/client system. Then it's all just
	string processing.

GET /
	Prints an html form containing the main operator controls (config, set config, start, quit,
	preferences, show log, etc). Perhaps also prints an extract from the log (the first two or three
	lines of any error messages that resulted from the last operation).

GET /status
	Prints the system status in html format.
	
GET /config
	Prints an html document containing a form with fields for specifying every parameter
	(except matrix parameters, which have edit/load/save buttons instead).  CSS is used
	to divide the form into tabs to make it more readable.  Submission will cause the POSTed
	data to be translated into a lot of SET PARAMETER commands.

GET /config/tab/<tab_name>
GET /config/tab/<tab_name>/<section_name>
	As above, but prints only part of the config form (useful for browsers that don't do CSS).

GET /config/params/<parameter_name>
	As above, but prints the config form for a single parameter only, and if it is a matrix
	parameter, displays a cell-by-cell form for filling in the content.

GET /log
GET /log.txt
	Prints the operator log as html or plain text.

GET /<arbitrary_file_stem>.prm
	Prints the plain-text prm-format representation of all the parameters that SAVING INCLUDES.

"""###
