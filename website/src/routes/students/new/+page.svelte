<script>
	import { PUBLIC_API_URL } from '$env/static/public';
	import { onMount } from 'svelte';
	import ArrowRight from '$lib/icons/ArrowRightIcon.svelte';

	let classes = {};
	let message = '';

	async function fetchClasses() {
		const response = await fetch(PUBLIC_API_URL + '/classes?namesOnly=1', {
			method: 'GET',
			credentials: 'include'
		});
		if (response.ok) {
			classes = await response.json();
		} else if (response.status === 401) {
			window.location.href = '/';
		} else {
			console.error('Failed to fetch classes.');
		}
	}

	async function save() {
		const newStudent = {
			name: document.getElementById('name').value,
			gender: document.querySelector('.bg-select-gray').id === 'female' ? 1 : 0,
			class_id: parseInt(document.getElementById('class').value)
		};

		if (newStudent.name === '') {
			message = 'Bitte geben Sie einen Namen an.';
			return;
		}

		const response = await fetch(`${PUBLIC_API_URL}/students`, {
			method: 'PUT',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(newStudent),
			credentials: 'include'
		});

		if (response.ok) {
			const data = await response.json();
			window.location.href = '/students/view?id=' + data.id;
		} else if (response.status === 401) {
			window.location.href = '/';
		} else if (response.status === 409) {
			message = 'Ein Schüler mit gleichem Namen existiert bereits in dieser Klasse.';
		} else {
			console.error('Failed to create student.');
		}
	}

	onMount(async () => {
		await fetchClasses();
		document.getElementById('male').classList.add('bg-select-gray');
		const buttons = document.querySelectorAll('button');
		buttons.forEach(button => {
			button.addEventListener('click', function() {
				buttons.forEach(btn => btn.classList.remove('bg-select-gray'));
				this.classList.add('bg-select-gray');
			});
		});
	});
</script>

<div class="p-5 text-white">
	<div class="mb-4">
		<p class="block text-xl mb-1 text-tx-gray">Geschlecht</p>
		<div class="flex gap-2 h-[2.5rem] text-lg">
			<button type="button" id="female" class="female flex-1 bg-bg-light rounded-lg focus:outline-none">
				weiblich
			</button>
			<button type="button" id="male" class="flex-1 bg-bg-light rounded-lg focus:outline-none">
				männlich
			</button>
		</div>
	</div>
	<div class="mb-4">
		<p class="block mb-1 text-xl text-tx-gray">Klasse</p>
		<select id="class" class="input-tx">
			{#each Object.entries(classes) as [i, cls]}
				<option value="{i}">{cls}</option>
			{/each}
		</select>
	</div>
	<div class="mb-4">
		<p class="block text-xl mb-1 text-tx-gray">Name</p>
		<input id="name" type="text" placeholder="Max Mustermann" class="input-tx"
			   on:keydown={(e) => e.key === 'Enter' && save()}>
	</div>
	<p class="text-warn-red pt-2">{message}</p>
</div>
<div class="absolute inset-x-0 bottom-0 p-5">
	<button on:click={save} class="confirm-btn"><span class="btn-label">Hinzufügen</span><span aria-hidden="true"
																							   class="btn-icon"><ArrowRight /></span>
	</button>
</div>
