<script>
    import SprintIcon from "$lib/icons/SprintIcon.svelte";
    import LapRunIcon from "$lib/icons/LapRunIcon.svelte";
    import { formatTime } from "$lib/util.js";
    import AddIcon from "$lib/icons/AddIcon.svelte";

    export let name;
    export let size
    export let sprintAvgGrade;
    export let sprintAvgTime;
    export let lapRunAvgGrade;
    export let lapRunAvgTime;
</script>

<div class="mt-3 p-5 rounded-xl bg-bg-lightest relative fill-white">
    <div class="flex mb-2 items-baseline">
        <p class="font-bold text-2xl">{name}</p>
        <p class="text-tx-gray ml-3 text-xl">{size} Schüler</p>
    </div>
    <div class="font-bold">
        <div class="flex">
            <SprintIcon />
            <span class="ml-2"><span class="text-tx-gray float-left pr-1 font-normal">Ø-Note: </span>{sprintAvgGrade}</span>
            <span class="absolute left-[50%]"><span
                    class="text-tx-gray font-normal">Ø-Zeit:</span> {formatTime(sprintAvgTime, 0)}</span>
        </div>
        <div class="flex">
            <LapRunIcon />
            <span class="ml-2"><span class="text-tx-gray float-left pr-1 font-normal">Ø-Note: </span>{lapRunAvgGrade}</span>
            <span class="absolute left-[50%]"><span
                    class="text-tx-gray font-normal">Ø-Zeit:</span> {formatTime(lapRunAvgTime, 1)}</span>
        </div>
    </div>
</div>
