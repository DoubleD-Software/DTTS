<script>
    import "../app.css";
    import {page} from '$app/stores';
    import MenuIcon from "$lib/icons/MenuIcon.svelte";
    import CloseIcon from "$lib/icons/CloseIcon.svelte";
    import {onMount} from "svelte";

    let menuOpen = false;

    function toggleMenu() {
        menuOpen = !menuOpen;
    }

    $: {
        document.body.style.overflow = menuOpen ? 'hidden' : 'auto';
    }

    onMount(() => {
        menuOpen = false;
    });
</script>

<svelte:head>
    <title>{$page.data.title}</title>
</svelte:head>

<div class="bg-bg-light w-full h-16 p-3 flex justify-center items-center font-bold relative z-20">
    <div
            class="absolute left-3 fill-white w-14 cursor-pointer"
            role="button"
            tabindex="0"
            aria-expanded={menuOpen}
            aria-label="Toggle menu"
            on:click={toggleMenu}
            on:keypress={(e) => { if (e.key === 'Enter') toggleMenu() }}
    >
        {#if menuOpen}
            <CloseIcon/>
        {:else}
            <MenuIcon/>
        {/if}
    </div>
    {#if menuOpen}
        <h1 class="text-white text-4xl">DTTS</h1>
    {:else}
        <h1 class="text-white {$page.data.title.length < 17 ? 'text-4xl' : 'text-2xl pt-1 ml-10'}">{$page.data.title}</h1>
    {/if}
</div>

{#if menuOpen}
    <button type="button" class="fixed inset-0 bg-black bg-opacity-50 z-10" on:click={toggleMenu}></button>
    <div class="menu fixed top-16 w-full bg-bg-light text-center z-20">
        <a href="/runs" on:click={toggleMenu}>Läufe</a>
        <a href="/classes" on:click={toggleMenu}>Klassen</a>
        <a href="/students/new" on:click={toggleMenu}>Neuer Schüler</a>
        <a href="/gradingkeys" on:click={toggleMenu}>Notenschlüssel</a>
        <a href="/teachers" on:click={toggleMenu}>Lehrer</a>
        <a href="/active/tags" on:click={toggleMenu}>Aktiver Lauf</a>
    </div>
{/if}

<div>
    <slot />
</div>

<style lang="postcss">
    .menu a {
        @apply text-2xl text-white block py-3 border-t-2 border-bg-lightest;
    }

    .menu a:hover {
        @apply bg-bg-lightest;
    }
</style>
