<script>
    import SprintIcon from "$lib/icons/SprintIcon.svelte";
    import LapRunIcon from "$lib/icons/LapRunIcon.svelte";

    function formatTime(time, runType) {
        if (runType === 0) {
            return (time / 1000).toFixed(2).replace('.', ',') + "sek";
        } else {
            const minutes = Math.floor(time / 60000);
            const seconds = ((time % 60000) / 1000);
            return minutes + ":" + (seconds < 10 ? '0' : '') + seconds.toFixed(0) + "min";
        }
    }

    export let type;
    export let length;
    export let teacher;
    export let run_class;
    export let avg_grade;
    export let avg_time;
</script>

<div class="mt-3 p-5 rounded-xl bg-bg-lightest relative">
    <div class="flex items-center justify-between text-xl mb-2">
        <span class="font-bold">{type === 0 ? "Sprint" : "Rundenlauf"}</span>
        <span class="absolute left-0 right-0 text-center mx-auto">{length}m</span>
        <span class="fill-white">{#if type === 0} <SprintIcon/> {:else} <LapRunIcon/> {/if}</span>
    </div>
    <div class="">
        <p><span class="text-tx-gray">Lehrer:</span> {teacher}</p>
        <p><span class="text-tx-gray">Klasse:</span> {run_class}</p>
        <div class="flex items-center justify-between text-base">
            <span><span class="text-tx-gray float-left pr-1">Ø-Note: </span>{avg_grade}</span>
            <span class="absolute left-0 right-0 text-center mx-auto"><span
                    class="text-tx-gray">Ø-Zeit:</span> {formatTime(avg_time, type)}</span>
        </div>
    </div>
</div>