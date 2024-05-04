<script>
    import { PUBLIC_API_URL } from "$env/static/public";
    import {onMount} from "svelte";
    import {formatTime} from "$lib/util.js";
    import {formatDate} from "$lib/util.js";
    import {julianToDate} from "$lib/util.js";
    import Man from "$lib/icons/Man.svelte";
    import Woman from "$lib/icons/Woman.svelte";
    import LapRunIcon from "$lib/icons/LapRunIcon.svelte";
    import SprintIcon from "$lib/icons/SprintIcon.svelte";
    import EditIcon from "$lib/icons/EditIcon.svelte";

    let urlParams = new URLSearchParams(window.location.search);
    let studentId = urlParams.get('id');
    let student = null;

    async function getStudent() {
        const response = await fetch(PUBLIC_API_URL + '/students?id=' + studentId, {
            method: 'GET',
            credentials: 'include'
        });

        if (response.ok) {
            student = await response.json();
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 404) {
            setTimeout(() => {
                window.location.href = '/runs';
            }, 5000);
        } else {
            console.error("Failed to fetch data.");
        }
    }

    onMount(() => {
        getStudent();
    });
</script>

<div class="text-white p-5">
    {#if student !== null}
        <div class="text-white flex-row pb-2 text-lg">
            <p class="text-3xl font-semibold text-center">{student.name}</p>
            <div class="mb-2 fill-white flex w-full justify-center">
                <div class="scale-150 pt-1 pr-2">
                    {#if student.gender === 0}
                        <Man />
                    {:else}
                        <Woman />
                    {/if}
                </div>
                <p class="text-3xl font-regular text-tx-gray text-center">{student.class}</p>
                <a href="/students/edit?id={studentId}" class="scale-80 pt-1 pl-2"><EditIcon /></a>
            </div>
            <p class="mt-2 font-bold">Sprints</p>
            <div class="flex-row flex font-bold">
                <p><span class="text-tx-gray font-normal">Ø-Note:</span> {student.sprint.avg_grade}</p>
                <p class="absolute left-[50%]"><span class="text-tx-gray font-normal">Ø-Zeit:</span> {formatTime(student.sprint.avg_time, 0)}</p>
            </div>
            <p class="mt-2 font-bold">Rundenlauf</p>
            <div class="flex-row flex font-bold">
                <p><span class="text-tx-gray font-normal">Ø-Note:</span> {student.lap_run.avg_grade}</p>
                <p class="absolute left-[50%]"><span class="text-tx-gray font-normal">Ø-Zeit:</span> {formatTime(student.lap_run.avg_time, 1)}</p>
            </div>
            <p class="mt-2 font-bold">Gesamt</p>
            <p class="font-bold"><span class="text-tx-gray font-normal">Ø-Note:</span> {student.global_avg_grade}</p>
        </div>
        <div class="text-3xl text-white font-semibold mt-4 mb-2 text-center">Läufe</div>
        <div class="border-t border-gray-500 pt-3">
            {#each Object.entries(student.runs) as [index, run]}
                <a href="/runs/view?id={index}{run.type === 1 ? `&student=${studentId}` : ''}" class="fill-white flex items-center mb-2 last:mb-0 bg-bg-lightest p-2 rounded-lg">
                    {#if run.type === 1}
                        <LapRunIcon />
                    {:else}
                        <SprintIcon />
                    {/if}
                    <p class="ml-3 text-lg">{formatDate(julianToDate(run.date))}</p>
                    <div class="absolute left-[50%] right-8 flex">
                        <p class="flex-1 text-left">{run.length}m</p>
                        <p class="flex-1 ml-3 text-center">{formatTime(run.time, run.type)}</p>
                        <p class="flex-1 text-right font-bold">{run.grade}</p>
                    </div>
                </a>
            {/each}
        </div>

    {:else}
        <p class="text-white font-bold text-center mt-2 text-xl">Dieser Schüler existiert nicht.</p>
        <p class="text-center text-white text-lg">Du wirst automatisch weitergeleitet.</p>
    {/if}
</div>

