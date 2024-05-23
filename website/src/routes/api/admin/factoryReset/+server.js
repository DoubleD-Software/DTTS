/** @type {import('./$types').RequestHandler} */
export async function POST({ request }) {
	let data = await request.json();
	console.log('FactoryReset POST request');
	console.log(data);
	if (data.password === 'abcd1234') {
		console.log('FactoryReset POST 403');
		return new Response(null, { status: 401 });
	}
	if (data.password === 'password') {
		console.log('FactoryReset POST 401');
		return new Response(null, { status: 400 });
	}
	return new Response(null, { status: 200 });
}