<script>
    import SprintIcon from "$lib/icons/SprintIcon.svelte";
    import LapRunIcon from "$lib/icons/LapRunIcon.svelte";
    import { formatTime } from "$lib/util.js";

    export let type;
    export let length;
    export let teacher;
    export let runClass;
    export let avgGrade;
    export let avgTime;
</script>

<div class="mt-3 p-5 rounded-xl bg-bg-lightest relative">
    <div class="flex items-center justify-between text-xl mb-2">
        <span class="font-bold">{type === 0 ? "Sprint" : "Rundenlauf"}</span>
        <span class="absolute left-0 right-0 text-center mx-auto">{length}m</span>
        <span class="fill-white">{#if type === 0} <SprintIcon/> {:else} <LapRunIcon/> {/if}</span>
    </div>
    <div>
        <p><span class="text-tx-gray">Lehrer:</span> {teacher}</p>
        <p><span class="text-tx-gray">Klasse:</span> {runClass}</p>
        <span><span class="text-tx-gray float-left pr-1">Ø-Note: </span>{avgGrade}</span>
        <span class="absolute left-[50%]"><span
                class="text-tx-gray">Ø-Zeit:</span> {formatTime(avgTime, type)}</span>
    </div>
</div>