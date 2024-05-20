<script>
	import { formatTime } from '$lib/util.js';
	import { formatDate } from '$lib/util.js';
	import { julianToDate } from '$lib/util.js';
	import Man from '$lib/icons/Man.svelte';
	import Woman from '$lib/icons/Woman.svelte';
	import Trash from '$lib/icons/Trash.svelte';
	import { PUBLIC_API_URL } from '$env/static/public';
	import { onMount } from 'svelte';

	export let data;
	let json = data.data;
	import writeXlsxFile from 'write-excel-file';
	import { DownloadIcon } from 'lucide-svelte';

	let showModal = false;

	function toggleModal() {
		showModal = !showModal;
	}

	function deleteRun() {
		toggleModal();
		fetch(`${PUBLIC_API_URL}/runs?id=${data.runId}`, {
			method: 'DELETE',
			credentials: 'include'
		}).then(response => {
			if (response.ok) {
				window.location.href = '/runs';
			} else if (response.status === 401) {
				window.location.href = '/';
			}
		});
	}

	async function generateXLSX(data) {
		const studentSchema = [
			{
				column: 'Name',
				type: String,
				value: student => student.name,
				width: 20
			},
			{
				column: 'Zeit',
				type: String,
				value: student => student.time,
				width: 15
			},
			{
				column: 'Note',
				type: String,
				value: student => student.grade,
				width: 10
			}
		];

		const runInfoSchema = [
			{
				column: 'Typ',
				type: String,
				value: info => info.type,
				width: 15
			},
			{
				column: 'Länge',
				type: String,
				value: info => info.length,
				width: 10
			},
			{
				column: 'Datum',
				type: String,
				value: info => info.date,
				width: 15
			},
			{
				column: 'Lehrer',
				type: String,
				value: info => info.teacher,
				width: 30
			},
			{
				column: 'Klasse',
				type: String,
				value: info => info.class,
				width: 15
			},
			{
				column: 'Notenschlüssel (weiblich)',
				type: String,
				value: info => info.grading_key_female,
				width: 30
			},
			{
				column: 'Notenschlüssel (männlich)',
				type: String,
				value: info => info.grading_key_male,
				width: 30
			},
			{
				column: 'Durchschnittliche Note',
				type: String,
				value: info => info.avg_grade,
				width: 30
			},
			{
				column: 'Durchschnittliche Zeit',
				type: String,
				value: info => info.avg_time,
				width: 30
			}
		];

		const runInfoObjects = [
			{
				type: data.type === 0 ? 'Sprint' : 'Rundenlauf',
				length: `${data.length}m`,
				date: formatDate(julianToDate(json.date)),
				teacher: data.teacher,
				class: data.class,
				grading_key_female: data.grading_key_female,
				grading_key_male: data.grading_key_male,
				avg_grade: data.avg_grade,
				avg_time: formatTime(data.avg_time, data.type)
			}
		];

		const students = Object.values(data.students).map(student => ({
			name: student.name,
			time: formatTime(student.time, data.type),
			grade: student.grade.replace('.', ',')
		}));

		await writeXlsxFile(
			[students, runInfoObjects],
			{
				schema: [studentSchema, runInfoSchema],
				sheets: ['Schüler', 'Informationen'],
				fileName: `${json.type === 1 ? 'Rundenlauf' : 'Sprint'} (${formatDate(julianToDate(json.date))}).xlsx`
			}
		);
	}

	function downloadXLSX(filename, xlsxContent) {
		const blob = new Blob([xlsxContent], { type: 'application/octet-stream' });
		const url = URL.createObjectURL(blob);
		const a = document.createElement('a');
		a.href = url;
		a.download = filename;
		document.body.appendChild(a);
		a.click();
		document.body.removeChild(a);
		URL.revokeObjectURL(url);
	}

	function download() {
		const xlsxContent = generateXLSX(json);
		downloadXLSX(`${json.type === 1 ? 'Rundenlauf' : 'Sprint'} (${formatDate(julianToDate(json.date))}).xlsx`, xlsxContent);
	}

	onMount(() => {
		if (json.students === undefined) {
			json.students = {};
		}
	});
</script>

<div class="text-white flex flex-col p-5 text-lg relative">
	<button class="fill-warn-red absolute right-4 pt-1 scale-150 z-10" on:click={toggleModal}>
		<Trash />
	</button>

	<button class="fill-white absolute right-4 pt-10 scale-150 z-10" on:click={download}>
		<DownloadIcon />
	</button>

	{#if showModal}
		<div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center z-30">
			<div class="bg-bg-light p-5 rounded-lg shadow-lg text-white m-5 z-40">
				<p>Soll dieser Lauf wirklich gelöscht werden?</p>
				<p class="font-bold text-lg">Dies kann nicht rückgängig gemacht werden!</p>
				<div class="flex justify-end gap-4 mt-4">
					<button class="bg-gray-300 hover:bg-gray-400 text-black font-bold py-2 px-4 rounded" on:click={toggleModal}>
						Abbrechen
					</button>
					<button class="bg-warn-red hover:bg-red-700 text-white font-bold py-2 px-4 rounded" on:click={deleteRun}>
						Löschen
					</button>
				</div>
			</div>
		</div>
	{/if}

	<div class="mb-4 font-bold">
		<div class="flex">
			<p><span class="text-tx-gray font-normal">Datum:</span> {formatDate(julianToDate(json.date))}</p>
			<p class="absolute left-[50%]"><span class="text-tx-gray font-normal">Länge:</span> {json.length}m</p>
		</div>
		<div class="flex mt-2">
			<p><span class="text-tx-gray font-normal">Ø-Note:</span> {json.avg_grade.replace('.', ',')}</p>
			<p class="absolute left-[50%]"><span
				class="text-tx-gray font-normal">Ø-Zeit:</span> {formatTime(json.avg_time, json.type)}</p>
		</div>
		<p class="mt-2"><span class="text-tx-gray font-normal">Lehrer:</span> {json.teacher}</p>
		<p class="mt-2"><span class="text-tx-gray font-normal">Klasse:</span> {json.class}</p>
		<p class="text-tx-gray mt-2 font-normal">Notenschlüssel:</p>
		<div class="fill-white flex-row flex font-bold">
			<div class="flex">
				<Man />
				<p class="pl-2">{json.grading_key_male}</p>
			</div>
			<div class="flex absolute right-0 left-[50%]">
				<Woman />
				<p class="pl-2">{json.grading_key_female}</p>
			</div>
		</div>
	</div>
	<p class="font-bold text-3xl mt-3 mb-2 text-center">Schüler</p>
	<div class="border-t border-gray-500 pt-3">
		{#each Object.entries(json.students) as [id, student]}
			<a href="{json.type === 1 ? `/runs/view?id=${data.runId}&student=${id}` : ''}"
				 class="flex justify-between items-center mb-2 bg-bg-lightest p-2 pr-3 rounded-lg">
				<div class="flex fill-white">
					{#if student.gender === 1}
						<Woman />
					{:else}
						<Man />
					{/if}
					<p class="pl-2">{student.name}</p>
				</div>
				<div class="flex items-center gap-4">
					<p class="text-tx-gray">{formatTime(student.time, json.type)}</p>
					<p class="font-bold text-xl">{student.grade.replace('.', ',')}</p>
				</div>
			</a>
		{/each}
	</div>
</div>