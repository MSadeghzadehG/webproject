<template>
        <div class="page-header header-filter h-100" style="background-image: url('../../static/assets/img/bg7.jpg'); background-size: cover; background-position: top center;">
            <div class="container">
                <div class="row">
                    <div class="col-md-4 col-md-offset-4 col-sm-6 col-sm-offset-3">
                        <div class="card card-signup">
                            <form class="form" method="" action="">
                                <div class="header text-center" v-bind:class="[loginFormHeaderColor]">
                                    <h4 class="card-title">Log in</h4>
                                    <div class="social-line">
                                        <a href="#" class="btn btn-just-icon btn-simple">
                                            <i class="fa fa-google-plus"></i>
                                        </a>
                                    </div>
                                </div>
                                <div class="card-content">

                                    <div class="input-group">
									<span class="input-group-addon">
										<i class="material-icons">email</i>
									</span>
                                        <div class="form-group" v-bind:class="[formGroupUnderlineColorClass]"><input type="text" class="form-control" placeholder="Email..."><span class="material-input"></span></div>
                                    </div>

                                    <div class="input-group">
									<span class="input-group-addon">
										<i class="material-icons">lock_outline</i>
									</span>
                                        <div class="form-group" v-bind:class="[formGroupUnderlineColorClass]"><input type="password" placeholder="Password..." class="form-control"><span class="material-input"></span></div>
                                    </div>

                                    <!-- If you want to add a checkbox to this form, uncomment this code

                                    <div class="checkbox">
                                        <label>
                                            <input type="checkbox" name="optionsCheckboxes" checked>
                                            Subscribe to newsletter
                                        </label>
                                    </div> -->
                                </div>
                                <div class="footer text-center">
                                    <a href="#pablo" class="btn btn-simple btn-wd btn-lg" v-bind:class="[buttonColorTypeClass]">Login</a>
                                    <p class="d-block">

                                    </p>
                                </div>
                            </form>
                        </div>
                    </div>
                </div>
            </div>
        </div>

</template>

<script>
    export default {
        name: "Login",
        data: function(){
            return {
                loginFormHeaderColor:  "header-info",
                buttonColorTypeClass: "btn-info",
                formGroupUnderlineColorClass: "form-info"
            }
        },
        components:{

        },
        computed: {

        },
        methods:{

        }
    }
</script>

<style>

</style>
