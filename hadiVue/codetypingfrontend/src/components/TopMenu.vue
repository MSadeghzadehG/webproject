<template>
    <nav class="navbar navbar-default w-100" style="margin-bottom:0 !important;" v-bind:class="[navbarColorClass]" role="navigation">
        <div class="container-fliud">
            <div class="navbar-header">
                <button type="button" class="navbar-toggle" data-toggle="collapse"
                        data-target="#bs-example-navbar-collapse-1">
                    <span class="sr-only">Toggle navigation</span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                    <span class="icon-bar"></span>
                </button>
                <a class="navbar-brand" href="#">CodeTyping.Net</a>
            </div>

            <div class="collapse navbar-collapse" id="bs-example-navbar-collapse-1">
                <ul class="nav navbar-nav">
                    <li v-bind:class="{active : isActiveHome}"><router-link to="/">Home</router-link></li>
                    <li v-bind:class="{active : isActiveLogin}"><router-link to="/login">Login / Register</router-link></li>
                </ul>
                <div class="pull-right">
                    <button class="btn btn-rose btn-fab btn-fab-mini btn-round" v-on:click="changeColorTo('rose')">
                        <!--<i class="material-icons">favorite</i>-->
                    </button>

                    <button class="btn btn-primary btn-fab btn-fab-mini btn-round"
                            v-on:click="changeColorTo('primary')">
                        <!--<i class="material-icons">favorite</i>-->
                    </button>

                    <button class="btn btn-info btn-fab btn-fab-mini btn-round" v-on:click="changeColorTo('info')">
                        <!--<i class="material-icons">favorite</i>-->
                    </button>

                    <button class="btn btn-warning btn-fab btn-fab-mini btn-round"
                            v-on:click="changeColorTo('warning')">
                        <!--<i class="material-icons">favorite</i>-->
                    </button>

                    <button class="btn btn-success btn-fab btn-fab-mini btn-round"
                            v-on:click="changeColorTo('success')">
                        <!--<i class="material-icons">favorite</i>-->
                    </button>

                    <button class="btn btn-danger btn-fab btn-fab-mini btn-round" v-on:click="changeColorTo('danger')">
                        <!--<i class="material-icons">favorite</i>-->
                    </button>

                    <button class="btn btn-default btn-fab btn-fab-mini btn-round"
                            v-on:click="changeColorTo('default')">
                        <!--<i class="material-icons">favorite</i>-->
                    </button>
                </div>
            </div>
        </div>
    </nav>
</template>

<script>

    export default{
        name: "TopMenu",
        data: function () {
            return {
                navbarColorClass: '',
                navItems: [
                    {name: "Home", target: "/"},
                    {name: "Register", target: "/register"},
                    {name: "Login", target: "/login"}
                ],
                colorNames: [
                    "rose", "primary", "info", "warning", "success", "danger"
                ],
                navColor: '',
            }
        },
        computed: {
            isActiveHome : function(){
                return this.$route.path === '/';
            },
            isActiveLogin : function(){
                return this.$route.path === '/login';
            },
        },
        methods: {
            changeColorTo: function (colorName) {
                window.console.log("Color button clicked. to " + colorName);
                window.console.log(this.$route.path)

                // var loginMustChange;
                // loginMustChange = this.$route.path === '/login';

                switch (colorName) {
                    case "rose":
                        this.navbarColorClass = "navbar-rose";
                        break;
                    case "primary":
                        this.navbarColorClass = "navbar-primary";
                        break;
                    case "info":
                        this.navbarColorClass = "navbar-info";
                        break;
                    case "warning":
                        this.navbarColorClass = "navbar-warning";
                        break;
                    case "success":
                        this.navbarColorClass = "navbar-success";
                        break;
                    case "danger":
                        this.navbarColorClass = "navbar-danger";
                        break;
                    case "default":
                        this.navbarColorClass = "navbar-default";
                        break;
                }
            }
        }
    }
</script>

<style scoped>

</style>
