<script>
	import { PUBLIC_API_URL } from '$env/static/public';
	import { onMount } from 'svelte';
	import AddIcon from '$lib/icons/AddIcon.svelte';
	import RightIcon from '$lib/icons/RightIcon.svelte';
	import { admin } from '\$lib/stores';

	let teachers = [];
	let isAdmin = false;

	async function fetchTeachers() {
		try {
			const response = await fetch(PUBLIC_API_URL + '/teachers', {
				method: 'GET',
				credentials: 'include'
			});

			if (response.ok) {
				const data = await response.json();
				teachers = Object.entries(data);
			} else if (response.status === 401) {
				window.location.href = '/';
			} else {
				console.error('Failed to fetch data.');
			}
		} catch (error) {
			console.error('Error: ', error);
		}
	}

	async function checkAdmin() {
		const response = await fetch(PUBLIC_API_URL + '/whoami', {
			method: 'GET',
			credentials: 'include'
		});

		if (response.ok) {
			let json = await response.json();
			isAdmin = json.admin;
			admin.set(isAdmin);
		} else if (response.status === 401) {
			window.location.href = '/';
		} else {
			console.error('Failed to fetch data.');
		}
	}

	onMount(async () => {
		await fetchTeachers();
		await checkAdmin();
	});
</script>

<div class="m-3 text-white">
	{#each teachers as [id, teacher]}
		<a href="/teachers/view?id={id}"
		   class="fill-white flex items-center mb-2 text-lg last:mb-0 bg-bg-lightest p-2 rounded-lg justify-between">
			<p>{teacher.name}</p>
			<div>
				<RightIcon />
			</div>
		</a>
	{/each}
	{#if teachers.length === 0}
		<p class="text-center text-xl">Es gibt noch keine Lehrer.</p>
	{/if}
</div>

{#if isAdmin}
	<a href="/teachers/new"
	   class="fixed bottom-8 right-8 bg-cf-green scale-[130%] p-3 fill-black rounded-2xl shadow-lg flex items-center justify-center text-3xl">
		<div class="scale-150">
			<AddIcon />
		</div>
	</a>
{/if}
