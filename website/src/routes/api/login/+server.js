/** @type {import('./$types').RequestHandler} */
export async function POST({ request, cookies }) {
    const credentials = await request.json();
    console.log('User logged in: ' + credentials)
    if (credentials.username === 'MK' && credentials.password === 'test') {
        cookies.set('session_id', 'dummy_session_id', { path: '/' });
        return new Response('', { status: 200 })
    } else {
        return new Response('', { status: 401 })
    }
}