<template>
    <div class="container">
        <h1 class="display-4">Register Form</h1>
        <hr>
        <form>
            <div class="form-group">
                <label for="inputUsername" class="text-left">Username</label>
                <input type="email" class="form-control" id="inputUsername" aria-describedby="usernameHelp"
                       placeholder="Enter username" v-model="inputUsername">
                <small id="usernameHelp" class="form-text text-muted">Description for your username</small>
            </div>
            <div class="form-group">
                <label for="exampleInputPassword1">Password</label>
                <input type="password" class="form-control" id="exampleInputPassword1" placeholder="Password"
                       v-model="inputPassword">
            </div>
            <button v-on:click.prevent="register" class="btn btn-primary">Register</button>
            <button v-on:click.prevent="login" class="btn btn-success" style="margin-left:10px;">Login</button>
            <button v-on:click.prevent="logout" class="btn btn-danger" style="margin-left:10px;">Logout</button>
            <button v-on:click.prevent="isauth" class="btn btn-warning" style="margin-left:10px;">Check Auth</button>
        </form>
    </div>
</template>

<script>
    export default {
        name: 'RegisterForm',
        props: {},
        data: function () {
            return {
                inputUsername: '',
                inputPassword: '',
            }
        },
        methods: {
            register: function () {
                this.$http.post("http://192.168.137.115:5000/register", {
                    params: {
                        username: this.inputUsername,
                        password: this.inputPassword,
                    },
                    headers:{
                        // "Access-Control-Allow-Origin" : "true",
                    }
                }).then(function (data) {
                    window.console.log(data);
                });
            },
            login: function(){
                this.$http.post("http://192.168.137.115:5000/login/?username=" + this.inputUsername + "&password=" + this.inputPassword, {
                    params :{},
                    // headers: {}
                }).then(function (data){
                    window.console.log(data);
                })
            },
            logout: function(){
                this.$http.get("http://192.168.137.115:5000/logout", {
                    params :{},
                    // headers: {}
                }).then(function (data){
                    window.console.log(data);
                })
            },
            isauth: function(){
                this.$http.get("http://192.168.137.115:5000/isauth", {
                    params :{},
                    // headers: {}
                }).then(function (data){
                    window.console.log(data);
                })
            },

        }
    }
</script>
<!--
    POST - > /register
    username , password as post body

!-->
<!-- Add "scoped" attribute to limit CSS to this component only -->
<style>

</style>
