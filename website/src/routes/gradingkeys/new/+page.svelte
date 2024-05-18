<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import {formatTime} from "$lib/util.js";
    import ArrowRight from "$lib/icons/ArrowRightIcon.svelte";
    import TimeModal from "$lib/components/TimeModal.svelte";
    import LengthModal from "$lib/components/LengthModal.svelte";

    let message = "";

    let type = 0;
    let gender = 0;
    let grades = Array.from({ length: 21 }, (_, i) => (1 + i * 0.25).toFixed(2));
    let entries = grades.map(grade => ({ grade, time: 0 }));
    let showModal = false;
    let currentTimeEntry = null;
    let length = 100;
    let showLengthModal = false;

    function openModal(entry) {
        currentTimeEntry = entry;
        showModal = true;
    }

    function closeModal() {
        showModal = false;
        entries = entries.map(entry => ({ ...entry }));
    }

    function toggleType(newType) {
        if (type !== newType) {
            type = newType;
            entries = grades.map(grade => ({ grade, time: 0 }));
        }
    }

    function toggleGender(newGender) {
        gender = newGender;
    }

    function toggleLengthModal() {
        showLengthModal = !showLengthModal;
    }

    function setLength(newLength) {
        length = newLength;
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
            type: document.querySelector('.bg-select-gray').id === "lap_run" ? 1 : 0,
            length: length,
            gender: gender,
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
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Typ</p>
        <div class="flex gap-2 h-[2.5rem] text-lg">
            <button type="button" id="lap_run"
                    class="flex-1 selector bg-bg-light rounded-lg focus:outline-none {type === 1 ? 'bg-select-gray' : ''}"
                    on:click={() => toggleType(1)}>
                Rundenlauf
            </button>
            <button type="button" id="sprint"
                    class="flex-1 selector bg-bg-light rounded-lg focus:outline-none {type === 0 ? 'bg-select-gray' : ''}"
                    on:click={() => toggleType(0)}>
                Sprint
            </button>
        </div>
    </div>
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Geschlecht</p>
        <div class="flex gap-2 h-[2.5rem] text-lg">
            <button type="button" id="female"
                    class="flex-1 selector bg-bg-light rounded-lg focus:outline-none {gender === 1 ? 'bg-select-gray' : ''}"
                    on:click={() => toggleGender(1)}>
                weiblich
            </button>
            <button type="button" id="male"
                    class="flex-1 selector bg-bg-light rounded-lg focus:outline-none {gender === 0 ? 'bg-select-gray' : ''}"
                    on:click={() => toggleGender(0)}>
                männlich
            </button>
        </div>
    </div>
    <div class="mb-2">
        <p class="block text-xl mb-1 text-tx-gray">Länge</p>
        <button id="length" class="input-tx" on:click={toggleLengthModal}>{length} <span class="text-tx-gray">m</span>
        </button>
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
