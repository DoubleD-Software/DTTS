<script>
    import {onMount} from "svelte";

    export let type;
    export let entry;
    export let onClose;

    let first = undefined;
    let second = undefined;
    let message = '';

    function saveTime() {
        if (first === undefined || second === undefined) {
            message = 'Bitte füllen Sie alle Felder aus!';
            return;
        }
        if (first === 0 && second === 0) {
            message = 'Die Zeit darf nicht 0 sein!';
            return;
        }
        if (first < 0 || second < 0) {
            message = 'Die Zeit muss größer als 0 sein!';
            return;
        }
        if (type === 0) {
            if (second > 99) {
                message = 'Die Millisekunden dürfen nicht größer als 99 sein! (Nur die zwei ersten Ziffern)';
                return;
            }
            entry.time = (second < 10 ? second * 100 : second * 10) + first * 1000;
        } else {
            if (second > 59) {
                message = 'Die Sekunden dürfen nicht größer als 59 sein!';
                return;
            }
            entry.time = first * 60000 + second * 1000;
        }
        onClose();
    }

    function validateFirstInput(event) {
        const value = event.target.value;
        if (!/^\d*$/.test(value)) {
            event.target.value = value.replace(/[^0-9]/g, '');
        }
        first = event.target.value;
    }

    function validateSecondInput(event) {
        const value = event.target.value;
        if (!/^\d{0,2}$/.test(value)) {
            event.target.value = value.replace(/[^0-9]/g, '').substring(0, 2);
        }
        second = event.target.value;
    }

    onMount(() => {
        if (entry && entry.time > 0) {
            if (type === 0) {
                first = Math.floor(entry.time / 1000);
                second = (entry.time % 1000) / 10;
            } else {
                first = Math.floor(entry.time / 60000);
                second = Math.floor((entry.time % 60000) / 1000);
            }
        }
        document.getElementById('first').focus()
    });
</script>

<div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center p-5 text-xl z-40">
    <div class="bg-bg-light p-4 rounded-lg w-full text-white">
        <div class="text-xl font-bold text-center flex justify-center items-baseline">
            <p class="text-tx-gray font-normal pr-3">Note </p>
            <p class="text-2xl">{entry.grade.replace('.', ',')}</p>
            <p class="text-tx-gray font-normal pl-3">ab</p>
        </div>
        <div class="flex justify-center">
            {#if type === 0}
                <input id="first" class="w-14 rounded bg-bg-lightest text-center" type="number" on:input={validateFirstInput} bind:value={first}
                       placeholder="sek" on:keydown={(e) => e.key === 'Enter' && document.getElementById('second').focus()}>
                <span>,</span>
                <input id="second" class="w-14 rounded bg-bg-lightest text-center mr-2" type="number" on:input={validateSecondInput} bind:value={second}
                       placeholder="ms" max="99" on:keydown={(e) => e.key === 'Enter' && saveTime()}>
                <span>sek</span>
            {:else}
                <input id="first" class="w-14 rounded bg-bg-lightest text-center" type="number" on:input={validateFirstInput} bind:value={first}
                       placeholder="min" on:keydown={(e) => e.key === 'Enter' && document.getElementById('second').focus()}>
                <span>:</span>
                <input id="second" class="w-14 rounded bg-bg-lightest text-center mr-2" type="number" on:input={validateSecondInput} bind:value={second}
                       placeholder="sek" max="59" on:keydown={(e) => e.key === 'Enter' && saveTime()}>
                <span>min</span>
            {/if}
        </div>
        <p class="text-warn-red text-base pt-4 text-center">{message}</p>
        <div class="flex justify-between gap-4 mt-4 text-base">
            <button class="bg-gray-300 text-black font-bold py-2 px-4 rounded" on:click={onClose}>
                Abbrechen
            </button>
            <button class="bg-cf-green text-black font-bold py-2 px-4 rounded" on:click={saveTime}>
                Speichern
            </button>
        </div>
    </div>
</div>
