<script>
	import { onMount } from 'svelte';
	import { PUBLIC_API_URL } from '$env/static/public';

	let isReachable = false;

	async function checkServerReachability() {
		try {
			let url = PUBLIC_API_URL.replace('/api', '/');
			await fetch(url, {
				method: 'HEAD',
				cache: 'no-cache'
			});
			isReachable = true;
		} catch (error) {
			isReachable = false;
		}
	}

	onMount(() => {
		checkServerReachability();
		if (!window.matchMedia('(display-mode: standalone)').matches) {
			setTimeout(() => {
				if (isReachable) window.location.href = '/runs';
			}, 5000);
		}
	});
</script>

<h1 class="text-white text-center font-bold text-2xl pt-5">Da ist aber gewaltig was schiefgelaufen!</h1>
<p class="text-white text-center text-xl pt-5">Die Seite konnte nicht gefunden werden.</p>
{#if !isReachable}
	<p class="text-white text-center text-xl pt-5">Bitte prüfen Sie folgendes und laden anschließend die Seite neu:</p>
	<p class="text-white text-center text-xl pt-5">Sind Sie im WLAN des Systems und haben Sie Ihre mobilen Daten
		deaktiviert?</p>
{:else}
	<p class="text-white text-center text-xl pt-5">Sie werden automatisch auf die Hauptseite weitergeleitet.</p>
{/if}