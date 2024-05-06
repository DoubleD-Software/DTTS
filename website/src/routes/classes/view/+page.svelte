<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import {onMount} from "svelte";
    import {formatDate, formatTime, julianToDate} from "$lib/util.js";
    import LapRunIcon from "$lib/icons/LapRunIcon.svelte";
    import SprintIcon from "$lib/icons/SprintIcon.svelte";
    import Man from "$lib/icons/Man.svelte";
    import Woman from "$lib/icons/Woman.svelte";
    import EditIcon from "$lib/icons/EditIcon.svelte";
    import ClassIcon from "$lib/icons/ClassIcon.svelte";
    import Trash from "$lib/icons/Trash.svelte";

    let urlParams = new URLSearchParams(window.location.search);
    let classId = urlParams.get('id');
    let classEntry = null;
    let runs = true;
    let showModal = false;

    async function getStudent() {
        const response = await fetch(PUBLIC_API_URL + '/classes?id=' + classId, {
            method: 'GET',
            credentials: 'include'
        });

        if (response.ok) {
            classEntry = await response.json();
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 404) {
            window.location.href = '/classes';
        } else {
            console.error("Failed to fetch data.");
        }
    }

    function toggleModal() {
        showModal = !showModal;
    }

    async function deleteClass() {
        toggleModal();
        fetch(`${PUBLIC_API_URL}/classes?id=${classId}`, {
            method: "DELETE",
            credentials: "include"
        }).then(response => {
            if (response.ok) {
                window.location.href = "/classes";
            } else if (response.status === 401) {
                window.location.href = "/";
            }
        });
    }

    onMount(async () => {
        await getStudent();
        const selectors = document.querySelectorAll('.selector');
        selectors.forEach(selector => {
            selector.addEventListener('click', function () {
                selectors.forEach(sel => sel.classList.remove('bg-bg-light'));
                this.classList.add('bg-bg-light');
                runs = this.id === 'runs';
            });
        });
    });
</script>

{#if showModal}
    <div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center z-30">
        <div class="bg-bg-light p-5 rounded-lg shadow-lg text-white m-5 z-40">
            <p>Soll diese Klasse wirklich gelöscht werden? Alle Schüler und deren Läufe werden ebenfalls entfernt!</p>
            <p class="font-bold text-lg">Dies kann nicht rückgängig gemacht werden!</p>
            <div class="flex justify-end gap-4 mt-4">
                <button class="bg-gray-300 hover:bg-gray-400 text-black font-bold py-2 px-4 rounded" on:click={toggleModal}>Abbrechen</button>
                <button class="bg-warn-red hover:bg-red-700 text-white font-bold py-2 px-4 rounded" on:click={deleteClass}>Löschen</button>
            </div>
        </div>
    </div>
{/if}

<div class="text-white p-5">
    <button class="fill-warn-red pt-1 absolute right-4 scale-150 z-10" on:click={toggleModal}>
        <Trash />
    </button>

    {#if classEntry !== null}
        <div class="text-white flex-row pb-2 text-lg">
            <div class="mb-2 fill-white flex w-full justify-center">
                <div class="scale-150 pt-1 pr-2">
                    <ClassIcon/>
                </div>
                <p class="text-3xl ml-2 font-semibold text-center">{classEntry.name}</p>
                <a href="/classes/edit?id={classId}" class="scale-80 fill-white pt-1 pl-2">
                    <EditIcon/>
                </a>
            </div>
            <p class="mt-2 font-bold">Sprints</p>
            <div class="flex-row flex font-bold">
                <p><span class="text-tx-gray font-normal">Ø-Note:</span> {classEntry.sprint.avg_grade}</p>
                <p class="absolute left-[50%]"><span
                        class="text-tx-gray font-normal">Ø-Zeit:</span> {formatTime(classEntry.sprint.avg_time, 0)}</p>
            </div>
            <p class="mt-2 font-bold">Rundenlauf</p>
            <div class="flex-row flex font-bold">
                <p><span class="text-tx-gray font-normal">Ø-Note:</span> {classEntry.lap_run.avg_grade}</p>
                <p class="absolute left-[50%]"><span
                        class="text-tx-gray font-normal">Ø-Zeit:</span> {formatTime(classEntry.lap_run.avg_time, 1)}</p>
            </div>
            <p class="mt-2 font-bold">Gesamt</p>
            <p class="font-bold"><span class="text-tx-gray font-normal">Ø-Note:</span> {classEntry.global_avg_grade}</p>
        </div>
        <div class="flex items-baseline">
            <div id="runs"
                 class="selector flex-1 text-3xl text-white font-semibold items-center mt-4 mb-2 justify-center flex bg-bg-light py-2 rounded-2xl cursor-pointer">
                <p class="text-center">Läufe</p>
                <p class="text-tx-gray font-normal text-lg ml-2">({Object.keys(classEntry.runs).length})</p>
            </div>
            <div id="students"
                 class="selector flex-1 text-3xl text-white font-semibold items-center mt-4 mb-2 justify-center flex py-2 rounded-2xl cursor-pointer">
                <p class="text-center">Schüler</p>
                <p class="text-tx-gray font-normal text-lg ml-2">({Object.keys(classEntry.students).length})</p>
            </div>
        </div>

        <div class="border-t border-gray-500 pt-3">
            {#if runs}
                {#each Object.entries(classEntry.runs) as [index, run]}
                    <a href="/runs/view?id={index}{run.type === 1 ? `&student=${classId}` : ''}"
                       class="fill-white flex items-center mb-2 last:mb-0 bg-bg-lightest p-2 rounded-lg">
                        {#if run.type === 1}
                            <LapRunIcon />
                        {:else}
                            <SprintIcon />
                        {/if}
                        <p class="ml-3 text-lg">{formatDate(julianToDate(run.date))}</p>
                        <div class="absolute left-[50%] right-8 flex items-baseline">
                            <p class="flex-1 text-left">{run.length}m</p>
                            <p class="flex-1 ml-3 text-center">{formatTime(run.avg_time, run.type)}</p>
                            <p class="flex-1 text-right text-lg font-bold">{run.avg_grade}</p>
                        </div>
                    </a>
                {/each}
            {:else}
                {#each Object.entries(classEntry.students) as [index, student]}
                    <a href="/students/view?id={index}"
                       class="fill-white flex items-center mb-2 last:mb-0 bg-bg-lightest p-2 pr-3 rounded-lg text-lg">
                        {#if student.gender === 1}
                            <Woman />
                        {:else}
                            <Man />
                        {/if}
                        <div class="flex w-full justify-between">
                            <p class="ml-2">{student.name}</p>
                            <p class="font-bold">{student.avg_grade}</p>
                        </div>
                    </a>
                {/each}
            {/if}
        </div>
    {/if}
</div>

