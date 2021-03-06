import Vue from 'vue'
import Router from 'vue-router'
import Home from '../components/Home'
import Login from '../components/Login'
import TypingArea from '../components/TypingArea'
Vue.use(Router)

export default new Router({
    routes: [
        {
            path: '/',
            name: 'Home',
            component: Home,
        },
        {
            path: '/login',
            name: 'Login',
            component: Login,
        },
        {
            path : '/type',
            name: 'TypeArea',
            component : TypingArea,
        }
    ], mode: 'history'
})
