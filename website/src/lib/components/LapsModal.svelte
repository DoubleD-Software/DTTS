<script>
	export let laps;
	export let onClose;
	export let set;

	let modalMessage = '';
	let modalLaps = Math.floor(laps);
	let modalFractionLaps = Math.floor((laps % 1) * 10);

	function setLaps() {
		if (modalLaps < 0) {
			modalMessage = 'Die Anzahl der Runden muss 0 oder größer sein.';
		} else if (modalLaps > 100) {
			modalMessage = 'Die Anzahl der Runden darf nicht größer als 100 sein.';
		} else if (modalFractionLaps < 0) {
			modalMessage = 'Die Anzahl der Teilrunden muss 0 oder größer sein.';
		} else if (modalFractionLaps > 9) {
			modalMessage = 'Die Anzahl der Teilrunden darf nicht größer als 9 sein.';
		} else {
			let newLaps = modalLaps + modalFractionLaps / 10;
			if (newLaps <= 0) {
				modalMessage = 'Die Anzahl der Gesamtruenden muss größer als 0 sein.';
			} else {
				set(newLaps);
			}
		}
	}

	function validateFirstInput(event) {
		const value = event.target.value;
		if (!/^\d*$/.test(value)) {
			event.target.value = value.replace(/[^0-9]/g, '');
		}
		modalLaps = event.target.value;
	}

	function validateSecondInput(event) {
		const value = event.target.value;
		if (!/^\d?$/.test(value)) {
			event.target.value = value.replace(/[^0-9]/g, '').substring(0, 1);
		}
		modalFractionLaps = event.target.value;
	}
</script>

<div class="fixed inset-0 bg-black bg-opacity-50 flex justify-center items-center p-5 text-xl z-40">
	<div class="bg-bg-light p-4 rounded-lg w-full text-white">
		<p class="text-2xl mb-4 text-center font-bold">Runden</p>
		<div class="flex justify-center mx-8">
			<input class="w-full rounded bg-bg-lightest text-center" type="number" on:input={validateFirstInput}
				   on:keydown={(e) => e.key === 'Enter' && document.getElementById('second').focus()}
				   bind:value={modalLaps}
				   placeholder="1">
			<span class="text-tx-gray px-2">,</span>
			<input id="second" class="w-full rounded bg-bg-lightest text-center" type="number"
				   on:input={validateSecondInput}
				   on:keydown={(e) => e.key === 'Enter' && setLaps()} bind:value={modalFractionLaps}
				   placeholder="5">
		</div>
		<p class="text-warn-red text-base pt-4 text-center">{modalMessage}</p>
		<div class="flex justify-between gap-4 mt-4 text-base">
			<button class="bg-warn-red text-black font-bold py-2 px-4 rounded" on:click={onClose}>
				Abbrechen
			</button>
			<button class="bg-cf-green text-black font-bold py-2 px-4 rounded" on:click={setLaps}>
				Speichern
			</button>
		</div>
	</div>
</div>