<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import {onMount} from "svelte";
    import ArrowRight from "$lib/icons/ArrowRightIcon.svelte";

    let message = "";

    async function save() {
        const newTeacher = {
            name: document.getElementById("name").value,
            username: document.getElementById("username").value,
            password: document.getElementById("password").value,
        };

        if (newTeacher.name === "") {
            message = "Bitte geben Sie einen Namen an.";
            return;
        }
        if (newTeacher.username === "") {
            message = "Bitte geben Sie einen Benutzernamen an.";
            return;
        }
        if (newTeacher.password === "") {
            message = "Bitte geben Sie ein Passwort an.";
            return;
        }

        const response = await fetch(`${PUBLIC_API_URL}/teachers`, {
            method: 'PUT',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(newTeacher),
            credentials: 'include'
        });

        if (response.ok) {
            let json = await response.json();
            window.location.href = '/teachers/view?id=' + json.id;
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 409) {
            message = "Ein Lehrer mit gleichem Namen/Benutzernamen existiert bereits.";
        } else {
            console.error('Failed to create class.');
        }
    }
</script>

<div class="p-5 text-white">
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Name</p>
        <input id="name" type="text" placeholder="Hr. Rüdiger" class="input-tx" on:keydown={(e) => e.key === 'Enter' && document.getElementById('username').focus()}>
    </div>
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Benutzername</p>
        <input id="username" type="text" placeholder="RuedigerB-MPG" class="input-tx" on:keydown={(e) => e.key === 'Enter' && document.getElementById('password').focus()}>
    </div>
    <div class="mb-4">
        <p class="block text-xl mb-1 text-tx-gray">Passwort</p>
        <input id="password" type="password" placeholder="Neues Passwort anlegen..." class="input-tx" on:keydown={(e) => e.key === 'Enter' && save()}>
    </div>
    <p class="text-warn-red pt-1">{message}</p>
</div>
<div class="absolute inset-x-0 bottom-0 p-5">
    <button on:click={save} class="confirm-btn"><span class="btn-label">Speichern</span><span aria-hidden="true"
                                                                                               class="btn-icon"><ArrowRight/></span>
    </button>
</div>
