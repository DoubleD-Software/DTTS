<script>
	import { formatTime } from '$lib/util.js';

	export let data;
	let json = data.data;
</script>

<div class="text-white flex-row p-5 text-center text-lg">
	<div class="text-3xl font-semibold mb-2">{json.name}</div>
	<div class="mb-4 font-bold">
		<p><span class="text-tx-gray font-normal">Länge:</span> {json.length}m</p>
		<p><span class="text-tx-gray font-normal">Note:</span> {json.grade.replace('.', ',')}</p>
		<p><span class="text-tx-gray font-normal">Zeit:</span> {formatTime(json.time, 1)}</p>
	</div>
	<div class="border-t border-gray-500 pt-3">
		{#each Object.entries(json.rounds) as [index, round]}
			<div class="flex justify-between items-center mb-2 last:mb-0 bg-bg-lightest p-2 px-3 rounded-lg">
				<p class="font-bold">{index}.</p>
				<p class="text-tx-gray absolute left-0 right-0 text-center mx-auto">{round.length}m</p>
				<p>{formatTime(round.time, 1)}</p>
			</div>
		{/each}
	</div>
</div>
