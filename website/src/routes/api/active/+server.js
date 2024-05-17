import {json} from "@sveltejs/kit";

let response = 0;

/** @type {import('./$types').RequestHandler} */
export async function GET({url}) {
    console.log('Active GET request');
    if (response === 0) {
        response = 1;
        return new Response('', {status: 200});
    } else if (response === 1) {
        response = 0;
        return json({
                "type": 0,
                "length": 100,
                "date": 2460336,
                "class": "8o",
                "grading_key_male": "8 Sprint (m)",
                "grading_key_female": "8 Sprint (w)",
                "teacher": "Hr. Schwarzenegger"
            });
    } else if (response === 2) {
        return new Response('', {status: 404});
    }
}