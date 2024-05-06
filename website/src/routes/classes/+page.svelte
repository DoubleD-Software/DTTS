<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import ClassListEntry from "$lib/components/ClassListEntry.svelte";
    import {onMount} from "svelte";
    import AddIcon from "$lib/icons/AddIcon.svelte";

    let classes = [];

    async function fetchRuns() {
        try {
            const response = await fetch(PUBLIC_API_URL + '/classes', {
                method: 'GET',
                credentials: 'include'
            });

            if (response.ok) {
                const data = await response.json();
                classes = Object.entries(data);
            } else if (response.status === 401) {
                window.location.href = '/';
            } else {
                console.error("Failed to fetch data.");
            }
        } catch (error) {
            console.error("Error:", error);
        }
    }

    onMount(fetchRuns);
</script>

<div class="m-3 text-white">
    {#each classes as [id, classEntry]}
        <a href="/classes/view?id={id}">
            <ClassListEntry name={classEntry.name} size={classEntry.size} sprintAvgGrade={classEntry.sprint.avg_grade}
                            sprintAvgTime={classEntry.sprint.avg_time} lapRunAvgGrade={classEntry.lap_run.avg_grade}
                            lapRunAvgTime={classEntry.lap_run.avg_time}/>
        </a>
    {/each}
    {#if classes.length === 0}
        <p class="text-center text-xl">Es gibt noch keine Klassen.</p>
    {/if}
</div>

<a href="/classes/new" class="fixed bottom-8 right-8 bg-cf-green scale-[130%] p-3 fill-black rounded-2xl shadow-lg flex items-center justify-center text-3xl">
    <div class="scale-150"><AddIcon /></div>
</a>

