<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import {onMount} from "svelte";
    import ArrowRight from "$lib/icons/ArrowRightIcon.svelte";

    let urlParams = new URLSearchParams(window.location.search);
    let studentId = urlParams.get('id');
    let student = null;
    let classes = {};
    let classesReversed = {};
    let message = "";

    async function fetchClasses() {
        const response = await fetch(PUBLIC_API_URL + '/classes?namesOnly=1', {
            method: 'GET',
            credentials: 'include'
        });
        if (response.ok) {
            classes = await response.json();
            Object.keys(classes).forEach((key) => {
                classesReversed[classes[key]] = classesReversed[classes[key]] || [];
                classesReversed[classes[key]].push(key);
            });
        } else if (response.status === 401) {
            window.location.href = '/';
        } else {
            console.error("Failed to fetch classes.");
        }
    }

    async function getStudent() {
        const response = await fetch(PUBLIC_API_URL + '/students?id=' + studentId, {
            method: 'GET',
            credentials: 'include'
        });

        if (response.ok) {
            student = await response.json();
            document.getElementById("class").value = classesReversed[student.class];
            document.getElementById("name").value = student.name;
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 404) {
            window.location.href = '/runs';
        } else {
            console.error("Failed to fetch student data.");
        }
    }

    async function save() {
        const updatedStudent = {
            name: document.getElementById("name").value,
            class_id: parseInt(document.getElementById("class").value)
        };

        if (updatedStudent.name === "") {
            message = "Bitte geben Sie einen Namen an.";
            return;
        }

        const response = await fetch(`${PUBLIC_API_URL}/students?id=${studentId}`, {
            method: 'PATCH',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(updatedStudent),
            credentials: 'include'
        });

        if (response.ok) {
            window.location.href = '/students/view?id=' + studentId;
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 409) {
            message = "Ein Schüler mit gleichem Namen existiert bereits in dieser Klasse.";
        } else {
            console.error('Failed to update student.');
        }
    }

    onMount(async () => {
        if (studentId === null) {
            window.location.href = '/runs';
        }
        await fetchClasses();
        await getStudent();
    });
</script>

<div class="p-5 text-white">
    <div class="mb-4">
        <p class="block mb-1 text-xl text-tx-gray">Klasse</p>
        <select id="class" class="input-tx">
            {#each Object.entries(classes) as [i, cls]}
                <option value="{i}">{cls}</option>
            {/each}
        </select>
    </div>
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Name</p>
        <input id="name" type="text" placeholder="Max Mustermann" class="input-tx" on:keydown={(e) => e.key === 'Enter' && save()}>
    </div>
    <p class="text-warn-red pt-2">{message}</p>
</div>
<div class="absolute inset-x-0 bottom-0 p-5">
    <button on:click={save} class="confirm-btn"><span class="btn-label">Speichern</span><span aria-hidden="true"
                                                                                              class="btn-icon"><ArrowRight/></span>
    </button>
</div>
