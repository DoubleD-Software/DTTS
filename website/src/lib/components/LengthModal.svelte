<script>
    import {onMount} from "svelte";

    export let length;
    export let onClose;
    export let set;
    let modalLength = 100;
    let modalMessage = "";

    function setLength() {
        if (modalLength < 1) {
            modalMessage = "Die Länge muss größer als 0 sein.";
        } else if (modalLength > 100000) {
            modalMessage = "Die Länge darf nicht größer als 100.000 sein.";
        } else {
            modalMessage = "";
            set(modalLength);
            onClose();
        }
    }

    function validateInput(event) {
        const value = event.target.value;
        if (!/^\d*$/.test(value)) {
            event.target.value = value.replace(/[^0-9]/g, '');
        }
        modalLength = event.target.value;
    }

    function toggleLengthModal() {
        onClose();
        modalMessage = "";
    }

    onMount(() => {
        modalLength = length;
    });
</script>

<div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center p-5 text-xl z-40">
    <div class="bg-bg-light p-4 rounded-lg w-full text-white">
        <p class="text-2xl mb-4 text-center font-bold">Gesamtlänge</p>
        <div class="w-full flex justify-center">
            <input class=" w-full rounded bg-bg-lightest text-center" type="number" bind:value={modalLength}
                   on:input={validateInput} placeholder="Lauflänge" on:keydown={(e) => e.key === 'Enter' && setLength()}>
            <span class="text-tx-gray pl-2">m</span>
        </div>
        <p class="text-warn-red text-base pt-4 text-center">{modalMessage}</p>
        <div class="flex justify-between gap-4 mt-4 text-base">
            <button class="bg-gray-300 text-black font-bold py-2 px-4 rounded" on:click={toggleLengthModal}>
                Abbrechen
            </button>
            <button class="bg-cf-green text-black font-bold py-2 px-4 rounded" on:click={setLength}>
                Speichern
            </button>
        </div>
    </div>
</div>