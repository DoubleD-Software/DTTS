/** @type {import('@sveltejs/kit').Handle} */
export async function handle({ event, resolve }) {
    if (event.url.pathname.startsWith('/api/') && !event.url.pathname.startsWith('/api/login') && event.cookies.get('session_id') !== 'dummy_session_id') {
        return new Response('', { status: 401 })
    }

    const response = await resolve(event);
    return response;
}