var http = require('http');
var url = require('url');
var serveStatic = require('serve-static')
var contentDisposition = require('content-disposition')
var finalhandler = require('finalhandler')
var fs = require('fs')

var express = require('express')
var compression = require('compression')

var mongoose = require("mongoose");
mongoose.connect("mongodb://localhost:27017");

var db = mongoose.connection;
db.on('error', console.error.bind(console, 'connection error:'));
db.once('open', function() {
    console.log('connected!')
});

var nameSchema = new mongoose.Schema({
    firstName: String,
    lastNameName: String
});
var User = mongoose.model("User", nameSchema);

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

app.get('/register',function (req,res){
    var myData = new User({firstName:"first",lastNameName:"last"});//req.body);
    myData.save()
    .then(item => {
    res.send("item saved to database");
    })
    .catch(err => {
    res.status(400).send("unable to save to database");
    });
})

app.listen(port, () => console.log(`Example app listening on port ${port}!`))