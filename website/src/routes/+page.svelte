<script>
    import img from '$lib/images/dtts.webp'
    import ArrowRight from "$lib/icons/ArrowRightIcon.svelte";
    import { PUBLIC_API_URL } from "$env/static/public";
    import { dateToJulian } from "$lib/util.js";

    let username = "";
    let password = "";

    let message = "";

    async function login() {
        if (username === "" && password === "") {
            message = "Bitte füllen Sie alle Felder aus!";
            return;
        }
        const res = await fetch(PUBLIC_API_URL + '/login', {
            method: 'POST',
            credentials: 'include',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({username, password, date: dateToJulian(new Date())})
        })
        const code = res.status;
        if (code === 401) {
            message = "Benutzername oder Passwort falsch!";
        } else {
            message = "";
            window.location.href = "/runs";
        }
    }
</script>

<div class="flex justify-center pt-[10%]">
    <img src="{img}" alt="DTTS logo" class="w-80" />
</div>
<div class="m-7">
    <p class="text-tx-gray text-xl mt-2 mb-1">Benutzername</p>
    <input placeholder="MustermannE-MPG" bind:value={username} class="input-tx">
    <p class="text-tx-gray text-xl mt-2 mb-1">Passwort</p>
    <input placeholder="Passwort" bind:value={password} type="password" class="input-tx">
    <p class="text-warn-red pt-2">{message}</p>
    <div class="absolute inset-x-0 bottom-0 p-7">
        <button on:click={login} class="confirm-btn"><span class="btn-label">Anmelden</span><span aria-hidden="true" class="btn-icon"><ArrowRight /></span></button>
    </div>
</div>