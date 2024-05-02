<script>
    import { page } from '$app/stores';
    import LapInfo from "$lib/components/LapInfo.svelte";
</script>

<div>
    {#if $page.data.student}
        <LapInfo data={$page.data} />
    {:else}
        <p>sd</p>
    {/if}
</div>