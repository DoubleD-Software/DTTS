<script>
    import ChipIcon from "$lib/icons/ChipIcon.svelte";
    import ArrowRight from "$lib/icons/ArrowRightIcon.svelte";
    import {onMount} from "svelte";
    import {PUBLIC_WS_URL, PUBLIC_API_URL} from "$env/static/public";

    let studentName = "Lädt...";
    let detected = false;
    let last = false;
    let socket;

    async function checkRunStatus() {
        const response = await fetch(PUBLIC_API_URL + '/active', {
            method: 'GET',
            credentials: 'include'
        });

        if (response.ok) {
            try {
                await response.json();
                window.location.href = '/active/dash';
            } catch(err) { /* empty */ }
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 404) {
            window.location.href = '/runs/new';
        } else {
            console.error("Failed to fetch active run data.");
        }
    }

    function next() {
        if (last) {
            socket.send('1');
            window.location.href = '/active/dash';
        } else {
            socket.send('0');
        }
    }

    function connectWebsocket() {
        socket = new WebSocket(PUBLIC_WS_URL);

        socket.addEventListener('message', (event) => {
            if (event.data.startsWith('0')) {
                studentName = event.data.substring(2);
                detected = false;
            } else if (event.data.startsWith('2')) {
                detected = true;
            } else if (event.data.startsWith('1')) {
                studentName = event.data.substring(2);
                detected = false;
                last = true;
            }
        });

        socket.addEventListener('close', () => {
            setTimeout(connectWebsocket, 500);
        });
    }

    onMount(async () => {
        await checkRunStatus();
        await connectWebsocket();
    });
</script>

<div class="w-full flex flex-col items-center justify-center text-white">
    <div class="{detected ? 'fill-cf-green' : 'fill-white'} w-64">
        <ChipIcon />
    </div>
    <div class="flex flex-col items-center justify-center">
        <h1 class="{detected ? 'text-cf-green' : ''} text-5xl text-center font-extrabold">{studentName}</h1>
        <p class="text-center text-2xl px-16 pt-3">
            {#if detected}
                Der Tag wurde erkannt!
            {:else}
                Bitte halten Sie den Tag für den Schüler vor die Antenne.
            {/if}
        </p>
    </div>
</div>

{#if detected}
    <div class="fixed bottom-0 left-0 right-0 p-5">
        <button on:click={next} class="confirm-btn"><span class="btn-label">{last ? 'Scharfstellen' : 'Nächster Tag'}</span><span aria-hidden="true"
                                                                                                        class="btn-icon"><ArrowRight/></span>
        </button>
    </div>
{/if}
