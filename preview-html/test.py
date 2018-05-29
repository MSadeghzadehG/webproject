import http.server
import socketserver
from requests.models import Response
import ast
import json
import os

PORT = 80

def return_words(lang):
    f = open(lang+".txt",'r')
    list1 = eval(f.read())
    f.close()
    top200 = []
    for i in range(0,200):
        top200.append(list1[i])
    return bytes(json.dumps(dict(top200)) , "utf-8")
    

def _read_body(self):
    if 'content-length' in self.headers:
        length = int(self.headers['content-length'])
        return self.rfile.read(length) if length > 0 else None
    return None

class MyHandler(http.server.CGIHTTPRequestHandler):
    def do_HEAD(self):
        self.send_response(200, "ok")
        self.send_header('Access-Control-Allow-Origin', "*")
        self.send_header('Access-Control-Allow-Methods', 'POST, OPTIONS ,GET')
        self.send_header("Access-Control-Allow-Headers", "*") 
        self.end_headers()
    
    
    def do_GET(self):
        # pass
        # self.do_HEAD()
        super().do_GET()
        
        # self.wfile.close()    
        # self.wfile.write(b"<html><head><title>Title goes here.</title></head>")
        # self.wfile.write((b"<body><p>This is a test.</p>"))
        # # If someone went to "http://something.somewhere.net/foo/bar/",
        # # then self.path equals "/foo/bar/".
        # self.wfile.write(bytes("<p>You accessed path: %s</p>" % self.path,"utf-8"))
        # self.wfile.write(b"</body></html>")
            
    def do_OPTIONS(self):
        self.do_HEAD()
        # do_POST(self)

    def do_POST(self):
        # self.send_header("Content-type", "application/json")
        self.do_HEAD()

        data = json.loads(_read_body(self).decode("utf-8"))
        print(data)
        if data['reqType'] == 'typingResultSubmit':
            f = open("records.txt",'a')
            f.write(json.dumps({'time': data['time'] , 'speed': data['speed'] , 'wrongKey': data['wrongKey'] , 'currectKey': data['currectKey'] , 'lang' : data['lang'] }))
            f.write("\n")
            f.close()
        elif data['reqType'] == 'getWords':
            self.wfile.write(return_words(data['lang']))

try:
    server = http.server.HTTPServer(('', PORT), MyHandler)
    print('Started http server')
    server.serve_forever()
    
except KeyboardInterrupt:
    print('^C received, shutting down server')
    server.socket.close()


