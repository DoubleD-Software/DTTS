<script>
    import {dateToJulian} from "$lib/util.js";
    import {PUBLIC_API_URL} from "$env/static/public";
    import DateSelector from "$lib/components/DateSelector.svelte";
    import RunListEntry from "$lib/components/RunListEntry.svelte";

    let selectedDate = new Date();

    function handleDateChange(event) {
        selectedDate = event.detail.date;
        fetchRuns();
    }

    let runs = [];

    async function fetchRuns() {
        try {
            const response = await fetch(PUBLIC_API_URL + '/runs?date=' + dateToJulian(selectedDate), {
                method: 'GET',
                credentials: 'include'
            });

            if (response.ok) {
                const data = await response.json();
                runs = Object.entries(data);
            } else if (response.status === 401) {
                window.location.href = '/';
            } else {
                console.error("Failed to fetch data.");
            }
        } catch (error) {
            console.error("Error:", error);
        }
    }
</script>

<DateSelector on:dateChange={handleDateChange}/>
<div class="m-3 text-white">
    {#each runs as [id, run]}
        <a href="/runs/view?id={id}">
            <RunListEntry type={run.type} length={run.length} teacher={run.teacher} runClass={run.class}
                          avgTime={run.avg_time} avgGrade={run.avg_grade.replace('.', ',')}/>
        </a>
    {/each}
    {#if runs.length === 0}
        <p class="text-center text-xl">Keine Läufe an diesem Tag.</p>
    {/if}
</div>

