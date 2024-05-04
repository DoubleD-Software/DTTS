<script>
    import { createEventDispatcher } from 'svelte';
    import LeftIcon from "$lib/icons/LeftIcon.svelte";
    import RightIcon from "$lib/icons/RightIcon.svelte";
    import { onMount } from "svelte";
    import { formatDate } from "$lib/util.js";

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
    <button on:click={goPreviousDay}>
        <LeftIcon/>
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
    <button on:click={goNextDay}>
        <RightIcon/>
    </button>
</div>

<style>
    .date-selector {
        font-size: theme("fontSize.xl");
        display: flex;
        justify-content: space-between;
        align-items: center;
        padding: 10px;
        background-color: theme("colors.bg-lightest");
        color: white;
        fill: white;
        position: relative;
    }

    .date-display {
        cursor: pointer;
        user-select: none;
        text-align: center;
        flex-grow: 1;
    }

    .calendar {
        background-color: theme("colors.bg-lightest");
    }
</style>
