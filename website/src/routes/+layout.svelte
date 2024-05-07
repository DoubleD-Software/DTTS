<script>
    import "../app.css";
    import { page } from '$app/stores';
</script>

<svelte:head>
    <title>{$page.data.title}</title>
</svelte:head>

<div class="bg-bg-light w-full h-16 p-3 flex justify-center font-bold">
    <h1 class="text-white {$page.data.title.length < 17 ? 'text-4xl' : 'text-2xl pt-1'}">{$page.data.title}</h1>
</div>

<div>
    <slot />
</div>

