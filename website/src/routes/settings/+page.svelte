<script>
	import { PUBLIC_API_URL } from '$env/static/public';
	import { onMount } from 'svelte';
	import { admin } from '$lib/stores';

	let showModal = false;
	let message = '';
	let successful = false;
	let isStudentModal = false;
	let isAdmin = false;
	let password = '';

	function toggleStudentModal() {
		message = '';
		password = '';
		showModal = true;
		isStudentModal = true;
	}

	function toggleFactoryModal() {
		message = '';
		password = '';
		showModal = true;
		isStudentModal = false;
	}

	async function checkAdmin() {
		const response = await fetch(PUBLIC_API_URL + '/whoami', {
			method: 'GET',
			credentials: 'include'
		});

		if (response.ok) {
			let json = await response.json();
			isAdmin = json.admin;
			admin.set(isAdmin);
			if (!isAdmin) {
				window.location.href = '/runs';
			}
		} else if (response.status === 401) {
			window.location.href = '/';
		} else {
			console.error('Failed to fetch data.');
		}
	}

	async function resetStudents() {
		const confirmPassword = {
			password: password
		};

		if (confirmPassword.password === '') {
			message = 'Bitte geben Sie Ihr Passwort ein.';
			return;
		}

		const response = await fetch(`${PUBLIC_API_URL}/admin/studentsReset`, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(confirmPassword),
			credentials: 'include'
		});

		if (response.ok) {
			successful = true;
			showModal = false;
		} else if (response.status === 401 || response.status === 403) {
			window.location.href = '/';
		} else if (response.status === 400) {
			message = 'Falsches Passwort!';
		} else {
			console.error('Failed to reset students.');
		}
	}

	async function resetFactory() {
		const confirmPassword = {
			password: password
		};

		if (confirmPassword.password === '') {
			message = 'Bitte geben Sie Ihr Passwort ein.';
			return;
		}

		const response = await fetch(`${PUBLIC_API_URL}/admin/factoryReset`, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(confirmPassword),
			credentials: 'include'
		});

		if (response.ok) {
			successful = true;
			showModal = false;
		} else if (response.status === 401 || response.status === 403) {
			window.location.href = '/';
		} else if (response.status === 400) {
			message = 'Falsches Passwort!';
		} else {
			console.error('Failed to factory reset.');
		}
	}

	onMount(() => {
		checkAdmin();
	});
</script>

{#if showModal}
	<div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center z-30">
		<div class="bg-bg-light p-5 rounded-lg shadow-lg text-white m-5 z-40 text-lg">
			{#if isStudentModal}
				<p>Sollen wirklich alle Schüler und alle Läufe permanent gelöscht werden?</p>
			{:else}
				<p>Soll wirklich das ganze System auf Werkseinstellungen zurückgesetzt werden? Das Admin-Konto wird
					ebenfalls zurückgesetzt.
					Bitte informieren Sie sich über das Standardpasswort bevor Sie das System zurücksetzen.</p>
			{/if}
			<p>Das System wird danach neugestartet. Sie müssen sich anschließend erneut mit dem WLAN verbinden.</p>
			<p>Bitte geben Sie als Bestätigung Ihr Passwort ein.</p>
			<p class="font-bold text-xl">Dies kann nicht rückgängig gemacht werden!</p>
			<div class="mb-5 mt-3">
				<input id="name" type="password" placeholder="Passwort" class="input-tx !bg-bg-lightest"
					   bind:value={password}>
			</div>
			<p class="text-warn-red {message !== '' ? 'pb-1' : '' }">{message}</p>
			<div class="flex justify-between gap-4 mt-4">
				<button class="bg-cf-green hover:bg-gray-400 text-black font-bold py-2 px-4 rounded"
						on:click={() => {showModal = !showModal}}>Abbrechen
				</button>
				<button class="bg-warn-red text-white font-bold py-2 px-4 rounded"
						on:click={() => {isStudentModal ? resetStudents() : resetFactory()}}>Löschen
				</button>
			</div>
		</div>
	</div>
{/if}

<div class="text-white p-5">
	{#if !successful}
		<div class="mb-8">
			<p class="block text-3xl font-bold mb-1">Schüler zurücksetzen</p>
			<p class="block text-lg text-tx-gray">Hier können Sie alle Schüler und alle Läufe permanent löschen. Alle
				Klassen, Lehrer und Notenschlüssel bleiben unverändert.</p>
			<p class="block text-lg mb-2 font-bold">Dies kann nicht rückgängig gemacht werden!</p>
			<button on:click={toggleStudentModal} class="bg-warn-red w-full rounded-md font-bold
  			text-lg text-white text-center h-10">Zurücksetzen
			</button>
		</div>
		<div class="mb-4">
			<p class="block text-3xl font-bold mb-1">Werkseinstellungen</p>
			<p class="block text-lg text-tx-gray">Hier können Sie das gesamte DTTS System auf die Werkseinstellungen
				zurücksetzen.
				Schüler, Läufe, Klassen, Notenschlüssel und Lehrer werden permanent gelöscht.</p>
			<p class="block text-lg mb-2 font-bold">Dies kann nicht rückgängig gemacht werden!</p>
			<button on:click={toggleFactoryModal} class="bg-warn-red w-full rounded-md font-bold
  			text-lg text-white text-center h-10">Zurücksetzen
			</button>
		</div>
	{:else}
		<p class="text-center text-3xl font-bold">Erfolgreich zurückgesetzt!</p>
		<p class="text-center text-xl">Das System wird neugestartet. Sie müssen sich erneut mit dem WLAN verbinden und
			die Seite neu laden.
			Bitte haben Sie Geduld. Achten Sie auf den Bildschirm des Systems.</p>
	{/if}
</div>

