<script>
	import { PUBLIC_API_URL } from '$env/static/public';
	import { onMount } from 'svelte';
	import ArrowRight from '$lib/icons/ArrowRightIcon.svelte';

	let message = '';

	let urlParams = new URLSearchParams(window.location.search);
	let classId = urlParams.get('id');

	async function getClass() {
		const response = await fetch(PUBLIC_API_URL + '/classes?namesOnly=1', {
			method: 'GET',
			credentials: 'include'
		});

		if (response.ok) {
			let student = await response.json();
			if (student[classId] === undefined) {
				window.location.href = '/classes';
			}
			document.getElementById('name').value = student[classId];
		} else if (response.status === 401) {
			window.location.href = '/';
		} else if (response.status === 404) {
			setTimeout(() => {
				window.location.href = '/classes';
			}, 5000);
		} else {
			console.error('Failed to fetch student data.');
		}
	}

	async function save() {
		const newStudent = {
			name: document.getElementById('name').value
		};

		if (newStudent.name === '') {
			message = 'Bitte geben Sie einen Namen an.';
			return;
		}

		const response = await fetch(`${PUBLIC_API_URL}/classes?id=${classId}`, {
			method: 'PATCH',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(newStudent),
			credentials: 'include'
		});

		if (response.ok) {
			window.location.href = '/classes/view?id=' + classId;
		} else if (response.status === 401) {
			window.location.href = '/';
		} else if (response.status === 409) {
			message = 'Eine Klasse mit gleichem Namen existiert bereits.';
		} else {
			console.error('Failed to create class.');
		}
	}

	onMount(async () => {
		if (classId === null) {
			window.location.href = '/classes';
			return;
		}
		await getClass();
	});
</script>

<div class="p-5 text-white">
	<div class="mb-4">
		<p class="block text-xl mb-1 text-tx-gray">Name</p>
		<input id="name" type="text" placeholder="9a" class="input-tx" on:keydown={(e) => e.key === 'Enter' && save()}>
		<p class="text-tx-gray pt-2 text-sm">Klassen können bei erstmaliger Eingabe oder Bearbeitung des Schülers
			zugewiesen werden.</p>
	</div>
	<p class="text-warn-red pt-1">{message}</p>
</div>
<div class="absolute inset-x-0 bottom-0 p-5">
	<button on:click={save} class="confirm-btn"><span class="btn-label">Speichern</span><span aria-hidden="true"
																							  class="btn-icon"><ArrowRight /></span>
	</button>
</div>
