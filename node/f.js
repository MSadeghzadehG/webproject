var http = require('http');
var url = require('url');
var serveStatic = require('serve-static')
var contentDisposition = require('content-disposition')
var finalhandler = require('finalhandler')
var fs = require('fs')

var express = require('express')
var compression = require('compression')

var app = express()
var port = 5000

app.use(compression())

// serves files in static folder

app.use(express.static('static'))
app.get('/' ,function (req,res){
    console.log(req)
    res.setHeader('Content-Type','text/html')
    res.sendFile('static/index.html')
})

app.listen(port, () => console.log(`Example app listening on port ${port}!`))