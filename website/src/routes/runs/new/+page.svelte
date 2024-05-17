<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import {onMount} from "svelte";
    import ArrowRight from "$lib/icons/ArrowRightIcon.svelte";
    import Man from "$lib/icons/Man.svelte";
    import Woman from "$lib/icons/Woman.svelte";
    import {formatDate, dateToJulian} from "$lib/util.js";
    import LockedIcon from "$lib/icons/LockedIcon.svelte";

    let classes = {};
    let message = "";
    let modalMessage = "";
    let length = 100;
    let modalLength = 100;
    let laps = 1.0;
    let modalLaps = 1;
    let modalFractionLaps = 0;
    let showLengthModal = false;
    let showLapsModal = false;
    let runType = 0;
    let maleGradingKeys = {};
    let femaleGradingKeys = {};
    let students = {};
    let participatingStudents = {};

    function toggleLengthModal() {
        showLengthModal = !showLengthModal;
        modalMessage = "";
    }

    function toggleRunType() {
        fetchGradingKeys();
        runType = runType === 0 ? 1 : 0;
    }

    function toggleLapsModal() {
        if (!showLapsModal && runType === 0) {
            return;
        }
        showLapsModal = !showLapsModal;
        modalMessage = "";
    }

    async function fetchClasses() {
        const response = await fetch(PUBLIC_API_URL + '/classes?namesOnly=1', {
            method: 'GET',
            credentials: 'include'
        });
        if (response.ok) {
            classes = await response.json();
        } else if (response.status === 401) {
            window.location.href = '/';
        } else {
            console.error("Failed to fetch classes.");
        }
    }

    async function fetchGradingKeys() {
        const response = await fetch(`${PUBLIC_API_URL}/gradingkeys?type=${runType}&length=${length}`, {
            method: 'GET',
            credentials: 'include'
        });
        if (response.ok) {
            let json = await response.json();
            maleGradingKeys = json.male === undefined ? {} : json.male;
            femaleGradingKeys = json.female === undefined ? {} : json.female;
        } else if (response.status === 401) {
            window.location.href = '/';
        } else {
            console.error("Failed to fetch grading keys.");
        }
    }

    async function fetchStudents() {
        const response = await fetch(`${PUBLIC_API_URL}/classes?id=${parseInt(document.getElementById("class").value)}&studentsOnly=1`, {
            method: 'GET',
            credentials: 'include'
        });
        if (response.ok) {
            students = await response.json();
        } else if (response.status === 401) {
            window.location.href = '/';
        } else {
            console.error("Failed to fetch students.");
        }
    }

    async function save() {
        const newRun = {
            type: runType,
            length: length,
            date: dateToJulian(new Date()),
            class_id: parseInt(document.getElementById("class").value),
            grading_key_male_id: parseInt(document.getElementById("male_gradingkey").value),
            grading_key_female_id: parseInt(document.getElementById("female_gradingkey").value),
            laps: runType === 1 ? laps : -1.0,
            participants: Object.keys(participatingStudents)
        };

        if (newRun.length <= 0 || newRun.length > 100000) {
            message = "Die Länge muss zwischen 1 und 100.000 liegen.";
            return;
        }
        if ((newRun.laps <= 0 || newRun.laps > 100) && newRun.type === 1) {
            message = "Die Anzahl der Runden muss zwischen 0 und 100 liegen.";
            return;
        }
        if (newRun.class_id === undefined) {
            message = "Bitte wählen Sie eine Klasse aus.";
            return;
        }
        if (newRun.grading_key_male_id === undefined || newRun.grading_key_female_id === undefined) {
            message = "Bitte wählen Sie einen Notenschlüssel für beide Geschlechter aus.";
            return;
        }
        if (newRun.participants.length === 0) {
            message = "Bitte wählen Sie mindestens einen Teilnehmer aus.";
            return;
        }

        const response = await fetch(`${PUBLIC_API_URL}/runs`, {
            method: 'PUT',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(newRun),
            credentials: 'include'
        });

        if (response.ok) {
            window.location.href = '/active/tags';
        } else if (response.status === 401) {
            window.location.href = '/';
        } else {
            console.error('Failed to create run.');
        }
    }

    function setLength() {
        if (modalLength < 1) {
            modalMessage = "Die Länge muss größer als 0 sein.";
        } else if (modalLength > 100000) {
            modalMessage = "Die Länge darf nicht größer als 100.000 sein.";
        } else {
            toggleLengthModal();
            fetchGradingKeys();
            length = modalLength;
        }
    }

    function setLaps() {
        if (modalLaps <= 0) {
            modalMessage = "Die Anzahl der Runden muss größer als 0 sein.";
        } else if (modalLaps > 100) {
            modalMessage = "Die Anzahl der Runden darf nicht größer als 100 sein.";
        } else if (modalFractionLaps <= 0) {
            modalMessage = "Die Anzahl der Teilrunden muss größer als 0 sein.";
        } else if (modalFractionLaps > 9) {
            modalMessage = "Die Anzahl der Teilrunden darf nicht größer als 9 sein.";
        } else {
            laps = modalLaps + modalFractionLaps / 10;
            toggleLapsModal();
        }
    }

    function onSelectClass() {
        fetchStudents();
        participatingStudents = {};
    }

    let searchQuery = '';

    function addStudent(id) {
        if (!participatingStudents[id]) {
            participatingStudents = {...participatingStudents, [id]: students[id]};
        }
        searchQuery = ''; // Reset the search query
    }

    function removeStudent(id) {
        const {[id]: _, ...rest} = participatingStudents;
        participatingStudents = rest;
    }

    const filteredStudents = () => Object.entries(students).filter(([id, name]) => name.toLowerCase().includes(searchQuery.toLowerCase()) && !participatingStudents[id]);

    onMount(async () => {
        await fetchClasses();
        await fetchGradingKeys();
        await fetchStudents();
    });
</script>

{#if showLengthModal}
    <div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center p-5 text-xl z-40">
        <div class="bg-bg-light p-4 rounded-lg w-full text-white">
            <p class="text-2xl mb-4 text-center font-bold">Gesamtlänge</p>
            <div class="w-full flex justify-center">
                <input class=" w-full rounded bg-bg-lightest text-center" type="number" bind:value={modalLength}
                       placeholder="Lauflänge">
                <span class="text-tx-gray pl-2">m</span>
            </div>
            <p class="text-warn-red text-base pt-4 text-center">{modalMessage}</p>
            <div class="flex justify-between gap-4 mt-4 text-base">
                <button class="bg-gray-300 text-black font-bold py-2 px-4 rounded" on:click={toggleLengthModal}>
                    Abbrechen
                </button>
                <button class="bg-cf-green text-black font-bold py-2 px-4 rounded" on:click={setLength}>
                    Speichern
                </button>
            </div>
        </div>
    </div>
{/if}
{#if showLapsModal}
    <div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center p-5 text-xl z-40">
        <div class="bg-bg-light p-4 rounded-lg w-full text-white">
            <p class="text-2xl mb-4 text-center font-bold">Runden</p>
            <div class="flex justify-center mx-8">
                <input class="w-full rounded bg-bg-lightest text-center" type="number" bind:value={modalLaps}
                       placeholder="1">
                <span class="text-tx-gray px-2">,</span>
                <input class="w-full rounded bg-bg-lightest text-center" type="number" bind:value={modalFractionLaps}
                       placeholder="5">
            </div>
            <p class="text-warn-red text-base pt-4 text-center">{modalMessage}</p>
            <div class="flex justify-between gap-4 mt-4 text-base">
                <button class="bg-gray-300 text-black font-bold py-2 px-4 rounded" on:click={toggleLapsModal}>
                    Abbrechen
                </button>
                <button class="bg-cf-green text-black font-bold py-2 px-4 rounded" on:click={setLaps}>
                    Speichern
                </button>
            </div>
        </div>
    </div>
{/if}
<div class="p-5 text-white">
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Lauftyp</p>
        <div class="flex gap-2 h-[2.5rem]">
            <button type="button" on:click={toggleRunType} id="lap_run"
                    class="female flex-1 bg-bg-light rounded-lg focus:outline-none {runType === 1 ? 'bg-select-gray' : ''}">
                Rundenlauf
            </button>
            <button type="button" on:click={toggleRunType} id="sprint"
                    class="flex-1 bg-bg-light rounded-lg focus:outline-none {runType === 0 ? 'bg-select-gray' : ''}">
                Sprint
            </button>
        </div>
    </div>
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Länge</p>
        <button id="length" class="input-tx" on:click={toggleLengthModal}>{length} <span class="text-tx-gray">m</span>
        </button>
    </div>
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Datum</p>
        <div class="input-tx text-center flex justify-center">
            <div class="fill-white absolute left-7">
                <LockedIcon/>
            </div>
            {formatDate(new Date())}
        </div>
    </div>
    <div class="mb-4">
        <p class="block mb-1 text-xl text-tx-gray">Klasse</p>
        <select id="class" class="input-tx" on:change={onSelectClass}>
            {#each Object.entries(classes) as [i, cls]}
                <option value="{i}">{cls}</option>
            {/each}
        </select>
    </div>
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Notenschlüssel</p>
        <div class="flex gap-2 h-[2.5rem] items-center">
            <div class="scale-125 fill-white">
                <Man/>
            </div>
            <select id="male_gradingkey" class="input-tx flex-1">
                {#each Object.entries(maleGradingKeys) as [i, cls]}
                    <option value="{i}">{cls}</option>
                {/each}
            </select>
            <div class="scale-125 fill-white">
                <Woman/>
            </div>
            <select id="female_gradingkey" class="input-tx flex-1">
                {#each Object.entries(femaleGradingKeys) as [i, cls]}
                    <option value="{i}">{cls}</option>
                {/each}
            </select>
        </div>
    </div>
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Runden</p>
        <button id="laps" class="input-tx font-bold" on:click={toggleLapsModal}><span
                class="font-bold text-xl">{runType === 1 ? laps : '--'}</span> <span
                class="text-tx-gray">Runde(n)</span>
        </button>
    </div>
    <p class="text-warn-red {message !== '' ? 'pb-1' : '' }">{message}</p>
    <div class="container mb-4 text-white rounded-md">
        <p class="block text-xl mb-1 text-tx-gray">Teilnehmer</p>
        <div class="relative mb-4">
            <input
                    type="text"
                    placeholder="Bitte geben Sie die Teilnehmer an..."
                    bind:value={searchQuery}
                    class="w-full input-tx text-white"
            />
            {#if searchQuery}
                <div class="absolute bg-bg-lightest rounded-md mt-1 w-full">
                    {#each filteredStudents() as [id, name]}
                        <button
                                class="p-2 hover:bg-gray-700 cursor-pointer w-full text-left"
                                on:click={() => addStudent(id)}
                        >
                            {name}
                        </button>
                    {/each}
                </div>
            {/if}
        </div>
        {#each Object.entries(participatingStudents) as [id, name]}
            <div class="flex items-center justify-between p-2 bg-bg-lightest rounded-md mb-2">
                <span>{name}</span>
                <button
                        class="text-red-500 hover:text-red-700"
                        on:click={() => removeStudent(id)}
                >
                    &times;
                </button>
            </div>
        {/each}
    </div>
</div>
<div class="fixed bottom-0 left-0 right-0 p-5">
    <button on:click={save} class="confirm-btn"><span class="btn-label">Tags hinzufügen</span><span aria-hidden="true"
                                                                                               class="btn-icon"><ArrowRight/></span>
    </button>
</div>