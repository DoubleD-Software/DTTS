<script>
	import { dateToJulian } from '$lib/util.js';
	import { PUBLIC_API_URL } from '$env/static/public';
	import DateSelector from '$lib/components/DateSelector.svelte';
	import RunListEntry from '$lib/components/RunListEntry.svelte';
	import AddIcon from '$lib/icons/AddIcon.svelte';

	let selectedDate = new Date();
	let runs = undefined;

	function handleDateChange(event) {
		selectedDate = event.detail.date;
		fetchRuns();
	}

	async function fetchRuns() {
		try {
			const response = await fetch(PUBLIC_API_URL + '/runs?date=' + dateToJulian(selectedDate), {
				method: 'GET',
				credentials: 'include'
			});

			if (response.ok) {
				const data = await response.json();
				runs = Object.entries(data);
			} else if (response.status === 404) {
				runs = undefined;
			} else if (response.status === 401) {
				window.location.href = '/';
			} else {
				console.error('Failed to fetch data.');
			}
		} catch (error) {
			console.error('Error: ', error);
		}
	}
</script>

<DateSelector on:dateChange={handleDateChange} />
<div class="m-3 text-white">
	{#if runs === undefined }
		<p class="text-center text-xl">Keine Läufe an diesem Tag.</p>
	{:else}
		{#each runs as [id, run]}
			<a href="/runs/view?id={id}">
				<RunListEntry type={run.type} length={run.length} teacher={run.teacher} runClass={run.class}
							  avgTime={run.avg_time} avgGrade={run.avg_grade.replace('.', ',')} />
			</a>
		{/each}
	{/if}
</div>

<a href="/runs/new"
   class="fixed bottom-8 right-8 bg-cf-green scale-[130%] p-3 fill-black rounded-2xl shadow-lg flex items-center justify-center text-3xl">
	<div class="scale-150">
		<AddIcon />
	</div>
</a>

