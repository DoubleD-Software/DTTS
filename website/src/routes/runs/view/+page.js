import { PUBLIC_API_URL } from '$env/static/public';
import { redirect } from '@sveltejs/kit';

export const ssr = false;

/** @type {import('./$types').PageLoad} */
export async function load({ url }) {
	let runId = url.searchParams.get('id');
	let studentId = url.searchParams.get('student');
	if (runId === null) {
		throw redirect(302, '/runs');
	}

	const studentQueryParam = studentId ? `&student=${studentId}` : '';
	const response = await fetch(`${PUBLIC_API_URL}/runs?id=${runId}${studentQueryParam}`, {
		method: 'GET',
		credentials: 'include'
	});
	if (response.status === 401) {
		throw redirect(302, '/');
	} else if (response.status === 404 || response.status === 406) {
		throw redirect(302, '/runs' + (studentId === null ? '' : `/view?id=${runId}`));
	}

	const data = await response.json();
	return {
		title: studentId ? 'Runden' : (data.type ? 'Rundenlauf' : 'Sprint'),
		data: data,
		student: !!studentId,
		runId: runId
	};
}
