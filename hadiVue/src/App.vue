<template>
  <div id="app">
    <RegisterForm></RegisterForm>
  </div>
</template>

<script>
import RegisterForm from './components/RegisterForm.vue'

export default {
  name: 'app',
  components: {
      RegisterForm
  }
}
</script>

<style>
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}
</style>
