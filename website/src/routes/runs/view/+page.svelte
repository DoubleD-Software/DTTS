<script>
	import { page } from '$app/stores';
	import LapInfo from '$lib/components/LapInfo.svelte';
	import RunInfo from '$lib/components/RunInfo.svelte';
</script>

<div>
	{#if $page.data.student}
		<LapInfo data={$page.data} />
	{:else}
		<RunInfo data={$page.data} />
	{/if}
</div>