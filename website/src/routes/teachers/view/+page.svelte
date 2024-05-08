<script>
    import {PUBLIC_API_URL} from "$env/static/public";
    import {onMount} from "svelte";
    import {formatDate, formatTime, julianToDate} from "$lib/util.js";
    import LapRunIcon from "$lib/icons/LapRunIcon.svelte";
    import SprintIcon from "$lib/icons/SprintIcon.svelte";
    import Man from "$lib/icons/Man.svelte";
    import Woman from "$lib/icons/Woman.svelte";
    import EditIcon from "$lib/icons/EditIcon.svelte";
    import Trash from "$lib/icons/Trash.svelte";

    let urlParams = new URLSearchParams(window.location.search);
    let teacherId = urlParams.get('id');
    let teacher = null;
    let showModal = false;
    let admin = false;

    async function getTeacher() {
        const response = await fetch(PUBLIC_API_URL + '/teachers?id=' + teacherId, {
            method: 'GET',
            credentials: 'include'
        });

        if (response.ok) {
            teacher = await response.json();
        } else if (response.status === 401) {
            window.location.href = '/';
        } else if (response.status === 404) {
            window.location.href = '/runs';
        } else {
            console.error("Failed to fetch data.");
        }
    }

    function toggleModal() {
        showModal = !showModal;
    }

    async function deleteTeacher() {
        toggleModal();
        fetch(`${PUBLIC_API_URL}/teachers?id=${teacherId}`, {
            method: "DELETE",
            credentials: "include"
        }).then(response => {
            if (response.ok) {
                window.location.href = "/teachers";
            } else if (response.status === 401) {
                window.location.href = "/";
            }
        });
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

    onMount(() => {
        getTeacher();
        isAdmin();
    });
</script>

{#if showModal}
    <div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center z-30">
        <div class="bg-bg-light p-5 rounded-lg shadow-lg text-white m-5 z-40">
            <p>Soll dieser Lehrer wirklich gelöscht werden? Alle Läufe des Lehrers bleiben erhalten.</p>
            <p class="font-bold text-lg">Dies kann nicht rückgängig gemacht werden!</p>
            <div class="flex justify-end gap-4 mt-4">
                <button class="bg-gray-300 hover:bg-gray-400 text-black font-bold py-2 px-4 rounded"
                        on:click={toggleModal}>Abbrechen
                </button>
                <button class="bg-warn-red hover:bg-red-700 text-white font-bold py-2 px-4 rounded"
                        on:click={deleteTeacher}>Löschen
                </button>
            </div>
        </div>
    </div>
{/if}

<div class="text-white p-5">
    {#if admin}
        <button class="fill-warn-red absolute right-4 pt-1 scale-150 z-10" on:click={toggleModal}>
            <Trash/>
        </button>
    {/if}
    {#if teacher !== null}
        <div class="text-white flex-row pb-2 text-lg">
            <div class="mb-2 fill-white flex w-full justify-center">
                <p class="text-3xl font-semibold text-center">{teacher.name}</p>
                {#if admin}
                    <a href="/teachers/edit?id={teacherId}" class="scale-80 fill-white pt-1 pl-2">
                        <EditIcon/>
                    </a>
                {/if}
            </div>
            <p class="text-xl font-regular text-center">{teacher.username}</p>
            <p class="text-base mt-4 font-regular text-tx-gray text-center">Administrative Funktionen wie "Lehrer
                hinzufügen"
                oder "Passwort zurücksetzen" können nur durch den Administrator des Systems durchgeführt werden.</p>
        </div>
    {/if}
</div>

