<script>
    import {onMount} from "svelte";

    onMount(() => {
        setTimeout(() => {
            window.location.href = '/runs';
        }, 5000);
    });
</script>

<h1 class="text-white text-center font-bold text-2xl pt-5">Da ist aber gewaltig was schiefgelaufen!</h1>
<p class="text-white text-center text-xl pt-5">Die Seite konnte nicht gefunden werden.</p>
<p class="text-white text-center text-xl pt-5">Sie werden automatisch auf die Hauptseite weitergeleitet.</p>