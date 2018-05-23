import http.server
import socketserver
from requests.models import Response
import ast
import json

PORT = 80

class MyHandler(http.server.BaseHTTPRequestHandler):
    def do_HEAD(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
    def do_GET(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header("Content-type", "text/html")
        self.end_headers()
        
        f = open("data.txt",'r')
        # print(f.read())
        # print(ast.literal_eval(f.read)[1])
        list1 = eval(f.read())
        # print(list1[0])
        top200 = []
        for i in range(0,200):
            top200.append(list1[i])
        top200 = dict(top200)
        # top200 = top200.keys()
        # for key in top200.keys():
        #     self.wfile.write(bytes( " \"" + str(key) + "\", ",'utf-8'))

        self.wfile.write(bytes(json.dumps(top200) , "utf-8"))
        f.close()


        # print(self.wfile)
        # self.wfile.write(b"<html><head><title>Title goes here.</title></head>")
        # self.wfile.write((b"<body><p>This is a test.</p>"))
        # # If someone went to "http://something.somewhere.net/foo/bar/",
        # # then s.path equals "/foo/bar/".
        # self.wfile.write(bytes("<p>You accessed path: %s</p>" % self.path,"utf-8"))
        # self.wfile.write(b"</body></html>")
        # self.wfile.close()

try:
    server = http.server.HTTPServer(('', PORT), MyHandler)
    print('Started http server')
    server.serve_forever()
except KeyboardInterrupt:
    print('^C received, shutting down server')
    server.socket.close()
