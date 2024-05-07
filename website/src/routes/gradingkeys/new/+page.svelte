<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import {onMount} from "svelte";
    import {formatTime} from "$lib/util.js";
    import ArrowRight from "$lib/icons/ArrowRightIcon.svelte";
    import TimeModal from "$lib/components/TimeModal.svelte";

    let message = "";

    let type = 0;
    let grades = Array.from({ length: 21 }, (_, i) => (1 + i * 0.25).toFixed(2));
    let entries = grades.map(grade => ({ grade, time: 0 }));
    let showModal = false;
    let currentTimeEntry = null;

    function openModal(entry) {
        currentTimeEntry = entry;
        showModal = true;
    }

    function closeModal() {
        showModal = false;
        entries = entries.map(entry => ({ ...entry }));
    }

    function toggleType() {
        type = type === 0 ? 1 : 0;
        entries = grades.map(grade => ({ grade, time: 0 }));
    }

    async function save() {
        if (!entries.every(entry => entry.time > 0)) {
            message = "Bitte geben Sie gültige Zeiten für alle Noten an.";
            return;
        }
        let gradesObject = entries.reduce((obj, entry) => {
            obj[entry.grade.replace('.', ',')] = entry.time;
            return obj;
        }, {});
        const newGradingKey = {
            name: document.getElementById("name").value,
            type: document.querySelector('.bg-select-gray').id === "lap_run" ? 1 : 0,
            length: parseInt(document.getElementById("length").value),
            grades: gradesObject
        };

        if (newGradingKey.name === "") {
            message = "Bitte geben Sie einen Namen an.";
            return;
        }
        if (isNaN(newGradingKey.length)) {
            message = "Bitte geben Sie eine Länge an.";
            return;
        }

        const response = await fetch(`${PUBLIC_API_URL}/gradingkeys`, {
            method: 'PUT',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(newGradingKey),
            credentials: 'include'
        });

        if (response.ok) {
            const data = await response.json();
            window.location.href = '/gradingkeys/view?id=' + data.id;
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 409) {
            message = "Ein Notenschlüssel mit diesem Namen existiert bereits.";
        } else {
            console.error('Failed to create student.');
        }
    }

    onMount(async () => {
        document.getElementById("sprint").classList.add('bg-select-gray');
        type = 0;
    });
</script>

{#if showModal}
    <TimeModal type={type} entry={currentTimeEntry} onClose={closeModal}/>
{/if}
<div class="p-5 text-white mb-[3rem]">
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Name</p>
        <input id="name" type="text" placeholder="Max Mustermann" class="input-tx">
    </div>
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Typ</p>
        <div class="flex gap-2 h-[2.5rem] text-xl">
            <button type="button" id="lap_run"
                    class="flex-1 selector bg-bg-light rounded-lg focus:outline-none {type === 1 ? 'bg-select-gray' : ''}"
                    on:click={() => toggleType()}>
                Rundenlauf
            </button>
            <button type="button" id="sprint"
                    class="flex-1 selector bg-bg-light rounded-lg focus:outline-none {type === 0 ? 'bg-select-gray' : ''}"
                    on:click={() => toggleType()}>
                Sprint
            </button>
        </div>
    </div>
    <p class="block text-xl mb-1 text-tx-gray">Länge</p>
    <div class="relative flex items-center justify-center w-full">
        <input id="length" type="number" min="1" placeholder="100" class="w-full h-[2.5rem] text-lg bg-bg-light px-4 rounded-md text-center text-white focus:outline-none" style="padding-right: 2rem;">
        <span class="absolute text-tx-gray right-3">m</span>
    </div>
    <p class="text-warn-red pt-2">{message}</p>
    <p class="block text-xl mb-1 text-tx-gray">Noten</p>
    <div class="max-w-4xl mx-auto">
        <table class="w-full text-white">
            <thead>
            <tr class="bg-bg-light">
                <th class="py-2 w-[30%]">Note</th>
                <th class="py-2">Zeit</th>
            </tr>
            </thead>
            <tbody>
            {#each Object.values(entries) as entry}
                <tr class="bg-bg-lightest">
                    <td class="py-2 text-center font-bold">{entry.grade.replace('.', ',')}</td>
                    <td class="py-2">
                        <button type="button" class="w-full text-center bg-transparent text-white"
                                on:click={() => openModal(entry)}>
                            {entry.time === 0 ? 'Zeit hinzufügen' : formatTime(entry.time, type)}
                        </button>
                    </td>
                </tr>
            {/each}
            </tbody>
        </table>
    </div>
</div>
<div class="fixed bottom-0 left-0 right-0 p-5">
    <button on:click={save} class="confirm-btn"><span class="btn-label">Hinzufügen</span><span aria-hidden="true"
                                                                                               class="btn-icon"><ArrowRight/></span>
    </button>
</div>
