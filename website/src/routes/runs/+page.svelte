<script>
    import { dateToJulian} from "$lib/julian.js";
    import { PUBLIC_API_URL } from "$env/static/public";
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
                runs = Object.values(data);
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

<DateSelector on:dateChange={handleDateChange} />
<div class="m-3 text-white">
    {#each runs as run}
        <RunListEntry type={run.type} length={run.length} teacher={run.teacher} run_class={run.class} avg_time={run.avg_time} avg_grade={run.avg_grade} />
    {/each}
    {#if runs.length === 0}
        <p class="text-center text-xl">Keine Läufe an diesem Tag.</p>
    {/if}
</div>

