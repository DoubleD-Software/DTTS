<script>
	import { createEventDispatcher, onMount } from 'svelte';
	import LeftIcon from '$lib/icons/LeftIcon.svelte';
	import RightIcon from '$lib/icons/RightIcon.svelte';
	import { formatDate } from '$lib/util.js';

	const dispatch = createEventDispatcher();
	let currentDate = new Date();
	let formattedDate = formatDate(currentDate);
	let calendarElement;

	function goPreviousDay() {
		currentDate.setDate(currentDate.getDate() - 1);
		updateCalendar();
	}

	function goNextDay() {
		currentDate.setDate(currentDate.getDate() + 1);
		updateCalendar();
	}

	function updateCalendar() {
		calendarElement.value = currentDate.toISOString().split('T')[0];
		formattedDate = formatDate(currentDate);
		dispatch('dateChange', { date: currentDate });
	}

	function selectDate(event) {
		currentDate = event.target.valueAsDate;
		formattedDate = formatDate(currentDate);
		dispatch('dateChange', { date: currentDate });
	}

	function openCalendar() {
		calendarElement.showPicker();
	}

	onMount(() => {
		updateCalendar();
	});
</script>

<div class="date-selector">
	<button class="pr-6 pl-3 py-3" on:click={goPreviousDay}>
		<LeftIcon />
	</button>
	<button class="date-display" on:click={openCalendar}>
		{formattedDate}
	</button>
	<input
		bind:this={calendarElement}
		type="date"
		class="calendar"
		on:input={selectDate}
		style="display: none;"
	/>
	<button class="pl-6 pr-3 py-3" on:click={goNextDay}>
		<RightIcon />
	</button>
</div>

<style lang="postcss">
    .date-selector {
        @apply text-xl flex justify-between items-center bg-bg-lightest text-white fill-white relative;
    }

    .date-display {
        @apply cursor-pointer select-none text-center px-5 py-3;
    }

    .calendar {
        @apply bg-bg-lightest
    }
</style>
