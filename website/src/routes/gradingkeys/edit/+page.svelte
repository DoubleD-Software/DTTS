<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import {onMount} from "svelte";
    import {formatTime} from "$lib/util.js";
    import ArrowRight from "$lib/icons/ArrowRightIcon.svelte";
    import TimeModal from "$lib/components/TimeModal.svelte";
    import LockedIcon from "$lib/icons/LockedIcon.svelte";
    import LengthModal from "$lib/components/LengthModal.svelte";

    let message = "";

    let id = new URLSearchParams(window.location.search).get('id');
    let type = 0;
    let grades = Array.from({ length: 21 }, (_, i) => (1 + i * 0.25).toFixed(2));
    let entries = grades.map(grade => ({ grade, time: 0 }));
    let showModal = false;
    let currentTimeEntry = null;
    let gradingKey;
    let showLengthModal = false;
    let length = 100;

    function openModal(entry) {
        currentTimeEntry = entry;
        showModal = true;
    }

    function closeModal() {
        showModal = false;
        entries = entries.map(entry => ({ ...entry }));
    }

    function toggleLengthModal() {
        showLengthModal = !showLengthModal;
    }

    function setLength(newLength) {
        length = newLength;
    }

    async function getGradingKey() {
        const response = await fetch(PUBLIC_API_URL + '/gradingkeys?id=' + id, {
            method: 'GET',
            credentials: 'include'
        });

        if (response.ok) {
            gradingKey = await response.json();
            document.getElementById("name").value = gradingKey.name;
            length = gradingKey.length;
            type = gradingKey.type;
            entries = grades.map(grade => ({
                grade,
                time: gradingKey.grades[grade] || 0 // Replace '.' with ',' to match server format and ensure fallback to 0 if undefined
            }));
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 404) {
            window.location.href = '/gradingkeys';
        } else {
            console.error("Failed to fetch grading key data.");
        }
    }

    async function save() {
        if (!entries.every(entry => entry.time > 0)) {
            message = "Bitte geben Sie gültige Zeiten für alle Noten an.";
            return;
        }
        let gradesObject = entries.reduce((obj, entry) => {
            obj[entry.grade] = entry.time;
            return obj;
        }, {});
        const newGradingKey = {
            name: document.getElementById("name").value,
            length: length,
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

        const response = await fetch(`${PUBLIC_API_URL}/gradingkeys?id=${id}`, {
            method: 'PATCH',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(newGradingKey),
            credentials: 'include'
        });

        if (response.ok) {
            window.location.href = '/gradingkeys/view?id=' + id;
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 409) {
            message = "Ein Notenschlüssel mit diesem Namen existiert bereits.";
        } else {
            console.error('Failed to create student.');
        }
    }

    onMount(async () => {
        if (id === null) {
            window.location.href = '/gradingkeys';
        }
        await getGradingKey();
    });
</script>

{#if showModal}
    <TimeModal type={type} entry={currentTimeEntry} onClose={closeModal}/>
{/if}
{#if showLengthModal}
    <LengthModal length={length} set={setLength} onClose={toggleLengthModal} />
{/if}
<div class="p-5 text-white mb-[3rem]">
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Name</p>
        <input id="name" type="text" placeholder="9 Sprint (m)" class="input-tx">
    </div>
    <div class="mb-2">
        <p class="block text-xl mb-1 text-tx-gray">Länge</p>
        <button id="length" class="input-tx" on:click={toggleLengthModal}>{length} <span class="text-tx-gray">m</span>
        </button>
    </div>
    <p class="text-warn-red pt-2">{message}</p>
    <p class="block text-xl mb-1 mt-2 text-tx-gray">Noten</p>
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
    <button on:click={save} class="confirm-btn"><span class="btn-label">Speichern</span><span aria-hidden="true"
                                                                                               class="btn-icon"><ArrowRight/></span>
    </button>
</div>
