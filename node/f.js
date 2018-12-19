var http = require('http');
var url = require('url');
var serveStatic = require('serve-static')
var contentDisposition = require('content-disposition')
var finalhandler = require('finalhandler')
var fs = require('fs')

// Serve up static folder
var serve = serveStatic('static', {
    'index': false,
    'setHeaders': setHeaders
})
function setHeaders (res, path) {
    res.setHeader('Content-Disposition', contentDisposition(path))
}

//create a server object:
http.createServer(function (req, res) {
    // set headers
    res.writeHead(200, {'Access-Control-Allow-Origin': '*' , 'Access-Control-Allow-Methods' : 'POST, OPTIONS ,GET' , 'Access-Control-Allow-Headers' : '*' });
    
    // processing url
    var q = url.parse(req.url, true).query;
    console.log(req.url);

    fs.readFile("static" + req.url, function(err, data) {
        if (err) {
          res.writeHead(404, {'Content-Type': 'text/html'});
          return res.end("404 Not Found");
        }  
        res.writeHead(200, {'Content-Type': 'text/html'});
        res.write(data);
        return res.end();
      });
    // serve(req, res, finalhandler(req, res));

    // write a response to the client
    // res.write('Hello World!'); 
    
    // res.end(); //end the response
  }).listen(5000); //the server object listens on port 5000
console.log("server is up on port 5000");

