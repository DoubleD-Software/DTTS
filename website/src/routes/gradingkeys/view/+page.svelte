<script>
	import { PUBLIC_API_URL } from '$env/static/public';
	import { onMount } from 'svelte';
	import { formatTime } from '$lib/util.js';
	import EditIcon from '$lib/icons/EditIcon.svelte';
	import Trash from '$lib/icons/Trash.svelte';
	import SprintIcon from '$lib/icons/SprintIcon.svelte';
	import LapRunIcon from '$lib/icons/LapRunIcon.svelte';
	import Woman from '$lib/icons/Woman.svelte';
	import Man from '$lib/icons/Man.svelte';

	let id = new URLSearchParams(window.location.search).get('id');
	let grades = Array.from({ length: 21 }, (_, i) => (1 + i * 0.25).toFixed(2));
	let entries = grades.map(grade => ({ grade, time: 0 }));
	let gradingKey = null;
	let showModal = false;

	function toggleModal() {
		showModal = !showModal;
	}

	function deleteGradingKey() {
		fetch(PUBLIC_API_URL + '/gradingkeys?id=' + id, {
			method: 'DELETE',
			credentials: 'include'
		}).then(response => {
			if (response.ok) {
				window.location.href = '/gradingkeys';
			} else if (response.status === 401) {
				window.location.href = '/';
			} else {
				console.error('Failed to delete grading key.');
			}
		});
	}

	async function getGradingKey() {
		const response = await fetch(PUBLIC_API_URL + '/gradingkeys?id=' + id, {
			method: 'GET',
			credentials: 'include'
		});

		if (response.ok) {
			gradingKey = await response.json();
			entries = grades.map(grade => ({
				grade,
				time: gradingKey.grades[grade] || 0
			}));
		} else if (response.status === 401) {
			window.location.href = '/';
		} else if (response.status === 404) {
			window.location.href = '/gradingkeys';
		} else {
			console.error('Failed to fetch grading key data.');
		}
	}

	onMount(() => {
		if (id === null) {
			window.location.href = '/gradingkeys';
		}
		getGradingKey();
	});
</script>

{#if showModal}
	<div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center z-30">
		<div class="bg-bg-light p-5 rounded-lg shadow-lg text-white m-5 z-40 text-lg">
			<p>Soll dieser Notenschlüssel wirklich gelöscht werden? Die vollendeten Läufe sowie die Noten bleiben
				erhalten.</p>
			<p class="font-bold">Dies kann nicht rückgängig gemacht werden!</p>
			<div class="flex justify-between gap-4 mt-4">
				<button class="bg-gray-300 hover:bg-gray-400 text-black font-bold py-2 px-4 rounded"
						on:click={toggleModal}>Abbrechen
				</button>
				<button class="bg-warn-red hover:bg-red-700 text-white font-bold py-2 px-4 rounded"
						on:click={deleteGradingKey}>Löschen
				</button>
			</div>
		</div>
	</div>
{/if}

<div class="p-5 text-white">
	<button class="fill-warn-red pt-1 absolute right-4 scale-150 z-10" on:click={toggleModal}>
		<Trash />
	</button>

	{#if gradingKey !== null}
		<div class="text-white pb-2 text-lg">
			<div class="fill-white flex w-full justify-center items-center">
				<div class="scale-125 pr-3">
					{#if gradingKey.gender === 1}
						<Woman />
					{:else}
						<Man />
					{/if}
				</div>
				<div class="scale-150 pr-3">
					{#if gradingKey.type === 1}
						<LapRunIcon />
					{:else}
						<SprintIcon />
					{/if}
				</div>
				<p id="name" class="text-3xl font-semibold text-center">{gradingKey.name}</p>
			</div>
		</div>
		<div class="flex justify-center fill-tx-gray items-start">
			<p id="length" class="text-2xl text-center pr-3">{gradingKey.length}m</p>
			<a href="/gradingkeys/edit?id={id}" class="scale-80 fill-white">
				<EditIcon />
			</a>
		</div>
		<p class="block text-2xl mb-1 mt-4 text-white font-semibold text-center">Noten</p>
		<div class="max-w-4xl mx-auto">
			<table class="w-full text-white">
				<thead>
				<tr class="bg-bg-light">
					<th class="py-2 w-[30%]">Note</th>
					<th class="py-2">Zeit</th>
				</tr>
				</thead>
				<tbody>
				{#each Object.values(entries) as entry}
					<tr class="bg-bg-lightest">
						<td class="py-2 text-center font-bold">{entry.grade.replace('.', ',')}</td>
						<td class="py-2">
							<p class="w-full text-center bg-transparent text-white">
								{entry.time === 0 ? 'Zeit hinzufügen' : formatTime(entry.time, gradingKey.type)}
							</p>
						</td>
					</tr>
				{/each}
				</tbody>
			</table>
		</div>
	{/if}
</div>
