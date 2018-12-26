var http = require('http');
var url = require('url');
var serveStatic = require('serve-static')
var contentDisposition = require('content-disposition')
var finalhandler = require('finalhandler')
var fs = require('fs')

var express = require('express')
var compression = require('compression')
var session = require('express-session');
var bodyParser = require('body-parser');
var cookieParser = require('cookie-parser');
var mongoose = require("mongoose");
var mongo = require("mongodb")
var bcrypt = require('bcrypt')
var db_url = "mongodb://localhost:27017/codeTyping"

// mongo.MongoClient.connect(db_url,function (err,db){
//     if (err) {
//         console.log("error on connection to db")
//         throw err
//     } else {
//         var db_collection = db.db("firstmongodb")
//         console.log("connected to db")
//         db_collection.collection("users").insertMany([{username:"username",password:"password"},{username:"username1",password:"password"}] , function (err,res){
//             if (err) {
//                 console.log("error on collection insertion")
//                 throw err
//             }
//             console.log("obj added to collection")
//             console.log(res)
//             // db.close()
//         })
//         var query = {username:/^username/}
//         db_collection.collection("users").find(query).toArray(function (err,res){
//             if (err) {
//                 console.log("not found")
//                 // throw err
//             } else if (res) {
//                 console.log(res)
//                 console.log("found")
//             }
//         })
//     }
// })



//setup database
mongoose.connect(db_url)
var db = mongoose.connection;

//check database connection
db.on('error', console.error.bind(console, 'connection error:'));
db.once('open', function() { console.log('connected!') });

//create User schema
var userSchema = new mongoose.Schema({
    username: String,
    password: String,
});

userSchema.methods.verifyPassword = function(candidatePassword) {
    bcrypt.hash('mypassword', 10, function(err, hash) {
        if (err) { throw (err); }
        bcrypt.compare('mypassword', hash, function(err, isMatch) {
            if (err) { 
                console.log('error while checking password')
                // throw (err); 
                return false;
            } else if (!isMatch){
                console.log('password is wrong')
                return false
            } else return true;
            // console.log(isMatch);
        });
    });
};

userSchema.pre('save', function(next){
    var user = this;
    if (!user.isModified('password')) return next(); //?
    bcrypt.hash(user.password,10, function(err, hash){
        if(err) return next(err);
        user.password = hash;
        next();
    });
});

var User = mongoose.model("User", userSchema);

//build express app
var app = express()
var port = 5000

// serves files in static folder
app.use(compression())
app.use(express.static('static'))


app.route('/' ,function (req,res){
    console.log(req)
    res.setHeader('Content-Type','text/html')
    res.sendFile('static/index.html')
})


var passport = require('passport')
var FileStore = require('session-file-store')(session)
var uuid = require('uuid/v4')
var LocalStrategy = require('passport-local').Strategy;

app.use(cookieParser());
app.use(bodyParser.urlencoded({ extended: false }))
app.use(bodyParser.json())
app.use(session({
genid: (req) => {
        console.log('Inside session middleware genid function')
        console.log(`Request object sessionID from client: ${req.sessionID}`)
        return uuid() // use UUIDs for session IDs
    },
    store: new FileStore(),
    secret: 'keyboard cat',
    resave: false,
    saveUninitialized: true
}))
app.use(passport.initialize());
app.use(passport.session());

passport.serializeUser(function(user, done) {
    done(null, user.id);
});

passport.deserializeUser(function(id, done) {
    User.findById(id, function(err, user) {
        done(err, user);
    });
});

passport.use(new LocalStrategy(
    function(username, password, done) {
      User.findOne({ username: username }, function (err, user) {
        if (err) { 
            console.log('error while finding user')
            return done(err); 
        }
        if (!user) {
            console.log('user not found')
            return done(null, false); 
        }
        if (user.verifyPassword(password)) {
            console.log('password was wrong')
            return done(null, false); 
        }
        console.log('user logged in')
        return done(null, user);
      });
    }
));


app.post('/register', (req,res)=> {
    User.findOne({ username: req.query.username }, function (err, user) {
        if (err) { 
            console.log('error while finding username for register')
            res.send('error')
        } else if (user) {
            console.log('username is used before')
            res.send('this username is not available')
        } else {
            var user = new User({username:req.query.username,password:req.query.password});
            // console.log(JSON.stringify(user))
            user.save(function(err, data){
                if (err) {
                    console.log('error while adding user');
                    res.send('error')
                } else {
                    console.log ('user added' ,data);
                    res.send(true) //user added
                }
            });
        }
    });
})


app.post('/login', (req, res, next) => {
    passport.authenticate('local', (err, user, info) => {
        req.login(user, (err) => {
            // console.log(`req.session.passport: ${JSON.stringify(req.session.passport)}`)
            return res.send(true); //logged in
        })
    })(req, res, next);
})
  
app.get('/isauth', (req, res) => {
    console.log('Inside GET /authrequired callback')
    console.log(`User authenticated? ${req.isAuthenticated()}`)
    if(req.isAuthenticated()) {
        res.send(true) //logged in
    } else {
        res.send(false) //not logged in
    }
})

app.get('/logout', function(req, res){
    req.logout();
    res.redirect('/');
});

app.listen(port, () => console.log(`Example app listening on port ${port}!`))