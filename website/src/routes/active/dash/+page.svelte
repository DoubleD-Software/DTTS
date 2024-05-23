<script>
	import { onMount } from 'svelte';
	import { PUBLIC_API_URL, PUBLIC_WS_URL } from '$env/static/public';
	import { formatDate, formatTime, julianToDate } from '$lib/util.js';
	import Man from '$lib/icons/Man.svelte';
	import Woman from '$lib/icons/Woman.svelte';
	import Trash from '$lib/icons/Trash.svelte';

	let time = 0;
	let finishers = {};
	let started = false;
	let info = {};
	let timeText = '00:00:00';

	let showModal = false;

	let socket;

	function toggleModal() {
		showModal = !showModal;
	}

	async function cancelRun() {
		toggleModal();
		socket.send('0');
	}

	async function checkRunStatus() {
		const response = await fetch(PUBLIC_API_URL + '/active', {
			method: 'GET',
			credentials: 'include'
		});

		if (response.ok) {
			try {
				info = await response.json();
			} catch (err) {
				window.location.href = '/active/tags';
			}
		} else if (response.status === 401) {
			window.location.href = '/';
		} else if (response.status === 404) {
			window.location.href = '/runs/new';
		} else {
			console.error('Failed to fetch active run data.');
		}
	}

	function showTime() {
		time += 5;
		if (info.type === 0) {
			timeText = String(Math.floor(time / 60000)).padStart(2, '0') + ':' +
				String(Math.floor((time % 60000) / 1000)).padStart(2, '0') + ':' +
				String(time % 1000).padStart(3, '0').substring(0, 2);
		} else {
			timeText = String(Math.floor(time / 3600000)).padStart(2, '0') + ':' +
				String(Math.floor((time % 3600000) / 60000)).padStart(2, '0') + ':' +
				String(Math.floor((time % 60000) / 1000)).padStart(2, '0');
		}
		setTimeout(showTime, 5);
	}

	function connectWebsocket() {
		socket = new WebSocket(PUBLIC_WS_URL);

		socket.addEventListener('message', (event) => {
			if (event.data.startsWith('1')) {
				if (!started) {
					started = true;
					showTime();
				}
				time = parseInt(event.data.substring(2));
			} else if (event.data.startsWith('2')) {
				let json = JSON.parse(event.data.substring(2));
				if (json.finished === undefined) {
					finishers = {};
				} else {
					finishers = json.finished;
				}
			} else if (event.data.startsWith('3')) {
				window.location.href = '/runs';
			} else if (event.data.startsWith('4')) {
				window.location.href = '/runs/view?id=' + event.data.substring(2);
			}
		});

		socket.addEventListener('close', () => {
			setTimeout(connectWebsocket, 500);
		});
	}

	onMount(async () => {
		await checkRunStatus();
		connectWebsocket();
	});
</script>

{#if showModal}
	<div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center z-30">
		<div class="bg-bg-light p-5 rounded-lg shadow-lg text-white m-5 z-40 text-lg">
			<p>Soll dieser Lauf wirklich abgebrochen werden? Es bleibt nichts erhalten.</p>
			<p class="font-bold">Dies kann nicht rückgängig gemacht werden!</p>
			<div class="flex justify-between gap-4 mt-4">
				<button class="bg-gray-300 hover:bg-gray-400 text-black font-bold py-2 px-4 rounded"
						on:click={toggleModal}>Zurück
				</button>
				<button class="bg-warn-red hover:bg-red-700 text-white font-bold py-2 px-4 rounded"
						on:click={cancelRun}>Abbrechen
				</button>
			</div>
		</div>
	</div>
{/if}

<div class="w-full text-white p-4 px-9 text-lg">
	<button class="fill-warn-red absolute right-4 pt-1 scale-150 z-10" on:click={toggleModal}>
		<Trash />
	</button>
	{#if !started}
		<div class="text-2xl text-center pb-4 border-b border-tx-gray">
			<p class="font-bold">Das System ist scharfgestellt.</p>
			<p>Bei Startkommando bitte den</p>
			<p class="text-cf-green font-bold text-4xl">Start-Knopf</p>
			<p>drücken!</p>
		</div>
	{/if}
	<div class="py-4 font-bold border-b border-tx-gray">
		<div class="flex">
			<p><span class="text-tx-gray font-normal">Datum:</span> {formatDate(julianToDate(info.date))}</p>
			<p class="absolute left-[50%]"><span class="text-tx-gray font-normal">Länge:</span> {info.length}m</p>
		</div>
		<p class="mt-2"><span class="text-tx-gray font-normal">Lehrer:</span> {info.teacher}</p>
		<p class="mt-2"><span class="text-tx-gray font-normal">Klasse:</span> {info.class}</p>
		<p class="text-tx-gray mt-2 font-normal">Notenschlüssel:</p>
		<div class="fill-white flex-row flex font-bold">
			<div class="flex">
				<Man />
				<p class="pl-2">{info.grading_key_male}</p>
			</div>
			<div class="flex absolute right-0 left-[50%]">
				<Woman />
				<p class="pl-2">{info.grading_key_female}</p>
			</div>
		</div>
	</div>
	<div class="py-9 font-normal border-b border-tx-gray {started ? 'text-cf-green' : 'text-warn-red'}">
		<p class="font-dg text-center text-5xl">{timeText}</p>
	</div>
	<div class="py-4 text-center">
		{#each Object.values(finishers) as finisher}
			<div
				class="fill-white flex items-center mb-2 text-lg last:mb-0 bg-bg-lightest p-2 rounded-lg justify-between">
				<p>{finisher.name}</p>
				<p>{formatTime(finisher.time, info.type)}</p>
			</div>
		{/each}
		{#if Object.keys(finishers).length === 0}
			<p>Noch niemand ist fertig.</p>
		{/if}
	</div>
</div>