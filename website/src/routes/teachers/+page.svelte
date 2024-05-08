<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import {onMount} from "svelte";
    import AddIcon from "$lib/icons/AddIcon.svelte";
    import {formatDate, formatTime, julianToDate} from "$lib/util.js";
    import SprintIcon from "$lib/icons/SprintIcon.svelte";
    import LapRunIcon from "$lib/icons/LapRunIcon.svelte";
    import Woman from "$lib/icons/Woman.svelte";
    import Man from "$lib/icons/Man.svelte";
    import RightIcon from "$lib/icons/RightIcon.svelte";

    let teachers = [];
    let admin = false;

    async function fetchTeachers() {
        try {
            const response = await fetch(PUBLIC_API_URL + '/teachers', {
                method: 'GET',
                credentials: 'include'
            });

            if (response.ok) {
                const data = await response.json();
                teachers = Object.entries(data);
            } else if (response.status === 401) {
                window.location.href = '/';
            } else {
                console.error("Failed to fetch data.");
            }
        } catch (error) {
            console.error("Error: ", error);
        }
    }

    async function isAdmin() {
        const response = await fetch(PUBLIC_API_URL + '/whoami', {
            method: 'GET',
            credentials: 'include'
        });

        if (response.ok) {
            let json = await response.json();
            admin = json.admin;
        } else if (response.status === 401) {
            window.location.href = '/';
        } else {
            console.error("Failed to fetch data.");
        }
    }

    onMount(async () => {
        await fetchTeachers();
        await isAdmin();
    });
</script>

<div class="m-3 text-white">
    {#each teachers as [id, teacher]}
        <a href="/teachers/view?id={id}"
           class="fill-white flex items-center mb-2 text-lg last:mb-0 bg-bg-lightest p-2 rounded-lg justify-between">
            <p>{teacher.name}</p>
            <div>
                <RightIcon />
            </div>
        </a>
    {/each}
    {#if teachers.length === 0}
        <p class="text-center text-xl">Es gibt noch keine Lehrer.</p>
    {/if}
</div>

{#if admin}
    <a href="/teachers/new" class="fixed bottom-8 right-8 bg-cf-green scale-[130%] p-3 fill-black rounded-2xl shadow-lg flex items-center justify-center text-3xl">
        <div class="scale-150"><AddIcon /></div>
    </a>
{/if}
