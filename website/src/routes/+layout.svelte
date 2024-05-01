<script>
    import "../app.css";
    import { page } from '$app/stores';
</script>

<svelte:head>
    <title>{$page.data.title}</title>
</svelte:head>

<div class="bg-bg-light w-full p-3 flex justify-center font-bold">
    <h1 class="text-white text-4xl">{$page.data.title}</h1>
</div>

<div>
    <slot />
</div>

