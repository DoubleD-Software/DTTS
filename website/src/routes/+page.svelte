<script>
	import ArrowRight from '$lib/icons/ArrowRightIcon.svelte';
	import { PUBLIC_API_URL } from '$env/static/public';
	import { dateToJulian } from '$lib/util.js';
	import { admin } from '$lib/stores.js';

	let username = '';
	let password = '';

	let message = '';

	async function isAdmin() {
		const response = await fetch(PUBLIC_API_URL + '/whoami', {
			method: 'GET',
			credentials: 'include'
		});

		if (response.ok) {
			let json = await response.json();
			return json.admin;
		} else if (response.status === 401) {
			window.location.href = '/';
		} else {
			console.error('Failed to fetch data.');
		}
	}

	async function login() {
		if (username === '' && password === '') {
			message = 'Bitte füllen Sie alle Felder aus!';
			return;
		}
		const res = await fetch(PUBLIC_API_URL + '/login', {
			method: 'POST',
			credentials: 'include',
			headers: {
				'Content-Type': 'application/json'
			},
			body: JSON.stringify({ username, password, date: dateToJulian(new Date()) })
		});
		const code = res.status;
		if (code === 401) {
			message = 'Benutzername oder Passwort falsch!';
		} else {
			message = '';
			let whoami = await isAdmin();
			admin.set(whoami);
			window.location.href = '/runs';
		}
	}
</script>

<div class="flex justify-center pt-[10%]">
	<img src="/dtts.webp" alt="DTTS logo" class="w-80" />
</div>
<div class="m-5">
	<p class="text-tx-gray text-xl mt-2 mb-1">Benutzername</p>
	<input placeholder="MustermannE-MPG" bind:value={username} class="input-tx"
		   on:keydown={(e) => e.key === 'Enter' && document.getElementById('password').focus()}>
	<p class="text-tx-gray text-xl mt-2 mb-1">Passwort</p>
	<input placeholder="Passwort" id="password" bind:value={password} type="password" class="input-tx"
		   on:keydown={(e) => e.key === 'Enter' && login()}>
	<p class="text-warn-red pt-2">{message}</p>
	<div class="absolute inset-x-0 bottom-0 p-5">
		<button on:click={login} class="confirm-btn"><span class="btn-label">Anmelden</span><span aria-hidden="true"
																								  class="btn-icon"><ArrowRight /></span>
		</button>
	</div>
</div>