import {redirect} from "@sveltejs/kit";

export const prerender = true;

/** @type {import('./$types').PageLoad} */
export async function load({ data }) {
    return {
        title: 'Fehler'
    };
}