<script>
    import { PUBLIC_API_URL } from "$env/static/public";
    import {onMount} from "svelte";
    import StudentInfo from "$lib/components/StudentInfo.svelte";

    let urlParams = new URLSearchParams(window.location.search);
    let studentId = urlParams.get('id');
    let student = null;

    async function getStudent() {
        const response = await fetch(PUBLIC_API_URL + '/students?id=' + studentId, {
            method: 'GET',
            credentials: 'include'
        });

        if (response.ok) {
            student = await response.json();
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 404) {
            setTimeout(() => {
                window.location.href = '/runs';
            }, 5000);
        } else {
            console.error("Failed to fetch data.");
        }
    }

    onMount(() => {
        getStudent();
    });
</script>

<div class="text-white p-5">
    {#if student !== null}
        <StudentInfo {student} {studentId} />
    {:else}
        <p class="text-white font-bold text-center mt-2 text-xl">Dieser Schüler existiert nicht.</p>
        <p class="text-center text-white text-lg">Sie werden automatisch weitergeleitet.</p>
    {/if}
</div>

