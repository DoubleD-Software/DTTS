import {json} from "@sveltejs/kit";

/** @type {import('./$types').RequestHandler} */
export async function GET({url}) {
    console.log('Whoami GET request');
    return json(
        {
            "admin": true
        });
}