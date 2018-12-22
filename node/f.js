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
    username: String,
    password: String
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


function check_username_exist(req,to_check){
    var output = false
    if (to_check=="register")
        User.findOne({ username:req.query.username},function(err,user){
            if (err){ 
                console.log('error')
            } else if (user) {
                // console.log('true')
                output = true
            }
            // console.log('false')
        });
    else if (to_check=="login")
        User.findOne({ username:req.query.username,password:req.query.password},function(err,user){
            if (err){ 
                console.log('error')
            } else if (user) {
                // console.log('true')
                output = true
            }
            // console.log('false')
        });
    return output
}

app.get('/register',function (req,res){ 
    console.log(req.query)
    try {
        if (check_username_exist(req,"register")){
            var myData = new User({username:req.query.username,password:req.query.password});//req.body);
            myData.save()
            console.log('done')
            res.send(true);
        } else 
            res.send(false)
    }
    catch(err){
        console.log('error')
        res.send(false);
    }
    res.end()
})

app.get('/login', function(req,res){
    console.log(req.query)
    res.send(check_username_exist(req,"login"))
    res.end()
})

app.listen(port, () => console.log(`Example app listening on port ${port}!`))