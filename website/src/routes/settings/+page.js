export const ssr = false;

/** @type {import('./$types').PageLoad} */
export async function load({ data }) {
	return {
		title: 'Einstellungen'
	};
}