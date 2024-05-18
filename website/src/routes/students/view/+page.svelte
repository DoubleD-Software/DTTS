<script>
    import { PUBLIC_API_URL } from "$env/static/public";
    import {onMount} from "svelte";
    import {formatDate, formatTime, julianToDate} from "$lib/util.js";
    import LapRunIcon from "$lib/icons/LapRunIcon.svelte";
    import SprintIcon from "$lib/icons/SprintIcon.svelte";
    import Man from "$lib/icons/Man.svelte";
    import Woman from "$lib/icons/Woman.svelte";
    import EditIcon from "$lib/icons/EditIcon.svelte";
    import Trash from "$lib/icons/Trash.svelte";

    let urlParams = new URLSearchParams(window.location.search);
    let studentId = urlParams.get('id');
    let student = null;
    let showModal = false;

    async function getStudent() {
        const response = await fetch(PUBLIC_API_URL + '/students?id=' + studentId, {
            method: 'GET',
            credentials: 'include'
        });

        if (response.ok) {
            student = await response.json();
            if (student.runs === undefined) {
                student.runs = {};
            }
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 404) {
            window.location.href = '/runs';
        } else {
            console.error("Failed to fetch data.");
        }
    }

    function toggleModal() {
        showModal = !showModal;
    }

    async function deleteStudent() {
        toggleModal();
        fetch(`${PUBLIC_API_URL}/students?id=${studentId}`, {
            method: "DELETE",
            credentials: "include"
        }).then(response => {
            if (response.ok) {
                window.location.href = "/runs";
            } else if (response.status === 401) {
                window.location.href = "/";
            }
        });
    }

    onMount(() => {
        getStudent();
    });
</script>

{#if showModal}
    <div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center z-30">
        <div class="bg-bg-light p-5 rounded-lg shadow-lg text-white m-5 z-40">
            <p>Soll dieser Schüler wirklich gelöscht werden? Alle Läufe des Schülers werden ebenfalls gelöscht.</p>
            <p class="font-bold text-lg">Dies kann nicht rückgängig gemacht werden!</p>
            <div class="flex justify-end gap-4 mt-4">
                <button class="bg-gray-300 hover:bg-gray-400 text-black font-bold py-2 px-4 rounded" on:click={toggleModal}>Abbrechen</button>
                <button class="bg-warn-red hover:bg-red-700 text-white font-bold py-2 px-4 rounded" on:click={deleteStudent}>Löschen</button>
            </div>
        </div>
    </div>
{/if}

<div class="text-white p-5">
    <button class="fill-warn-red absolute right-4 pt-1 scale-150 z-10" on:click={toggleModal}>
        <Trash />
    </button>
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
                <a href="/students/edit?id={studentId}" class="scale-80 fill-white pt-1 pl-2"><EditIcon /></a>
            </div>
            <p class="mt-2 font-bold">Sprints</p>
            <div class="flex-row flex font-bold">
                <p><span class="text-tx-gray font-normal">Ø-Note:</span> {student.sprint.avg_grade.replace('.', ',')}</p>
                <p class="absolute left-[50%]"><span class="text-tx-gray font-normal">Ø-Zeit:</span> {formatTime(student.sprint.avg_time, 0)}</p>
            </div>
            <p class="mt-2 font-bold">Rundenlauf</p>
            <div class="flex-row flex font-bold">
                <p><span class="text-tx-gray font-normal">Ø-Note:</span> {student.lap_run.avg_grade.replace('.', ',')}</p>
                <p class="absolute left-[50%]"><span class="text-tx-gray font-normal">Ø-Zeit:</span> {formatTime(student.lap_run.avg_time, 1)}</p>
            </div>
            <p class="mt-2 font-bold">Gesamt</p>
            <p class="font-bold"><span class="text-tx-gray font-normal">Ø-Note:</span> {student.global_avg_grade.replace('.', ',')}</p>
        </div>
        <div class="text-3xl text-white font-semibold mt-4 mb-2 text-center">Läufe</div>
        <div class="border-t border-gray-500 pt-3">
            {#each Object.entries(student.runs) as [index, run]}
                <a href="/runs/view?id={index}" class="fill-white flex items-center mb-2 last:mb-0 bg-bg-lightest p-2 rounded-lg">
                    {#if run.type === 1}
                        <LapRunIcon />
                    {:else}
                        <SprintIcon />
                    {/if}
                    <p class="ml-3 text-lg">{formatDate(julianToDate(run.date))}</p>
                    <div class="absolute left-[50%] right-8 flex">
                        <p class="flex-1 text-left">{run.length}m</p>
                        <p class="flex-1 ml-3 text-center">{formatTime(run.time, run.type)}</p>
                        <p class="flex-1 text-right font-bold">{run.grade.replace('.', ',')}</p>
                    </div>
                </a>
            {/each}
        </div>
    {/if}
</div>

