<template>
  <div class="h-100">
    <TopMenu/>
    <router-view/>
  </div>
</template>

<script>
    import TopMenu from './components/TopMenu';
    export default {
        name: 'App',
        components: {TopMenu},
    }
</script>

<style>

</style>
