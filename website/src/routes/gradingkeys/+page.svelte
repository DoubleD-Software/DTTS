<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import {onMount} from "svelte";
    import AddIcon from "$lib/icons/AddIcon.svelte";
    import {formatDate, formatTime, julianToDate} from "$lib/util.js";
    import SprintIcon from "$lib/icons/SprintIcon.svelte";
    import LapRunIcon from "$lib/icons/LapRunIcon.svelte";
    import Woman from "$lib/icons/Woman.svelte";
    import Man from "$lib/icons/Man.svelte";

    let gradingKeys = [];

    async function fetchGradingKeys() {
        try {
            const response = await fetch(PUBLIC_API_URL + '/gradingkeys', {
                method: 'GET',
                credentials: 'include'
            });

            if (response.ok) {
                const data = await response.json();
                gradingKeys = Object.entries(data);
            } else if (response.status === 401) {
                window.location.href = '/';
            } else {
                console.error("Failed to fetch data.");
            }
        } catch (error) {
            console.error("Error:", error);
        }
    }

    onMount(fetchGradingKeys);
</script>

<div class="m-3 text-white">
    {#each gradingKeys as [id, gradingKey]}
        <a href="/gradingkeys/view?id={id}"
           class="fill-white flex items-center mb-2 last:mb-0 bg-bg-lightest p-2 rounded-lg">
            <div class="fill-tx-gray mr-1">
                {#if gradingKey.gender === 1}
                    <Woman />
                {:else}
                    <Man />
                {/if}
            </div>
            {#if gradingKey.type === 1}
                <LapRunIcon />
            {:else}
                <SprintIcon />
            {/if}
            <p class="ml-3 text-lg">{gradingKey.name}</p>
            <div class="absolute left-[55%] right-8 flex items-baseline justify-between">
                <p class="text-tx-gray ml-3 text-center">{formatTime(gradingKey.min_time, gradingKey.type)}</p>
                <p class="text-right text-lg font-bold">{gradingKey.length}m</p>
            </div>
        </a>
    {/each}
    {#if gradingKeys.length === 0}
        <p class="text-center text-xl">Es gibt noch keine Notenschlüssel.</p>
    {/if}
</div>

<a href="/gradingkeys/new" class="fixed bottom-8 right-8 bg-cf-green scale-[130%] p-3 fill-black rounded-2xl shadow-lg flex items-center justify-center text-3xl">
    <div class="scale-150"><AddIcon /></div>
</a>

