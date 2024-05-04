import {json} from "@sveltejs/kit";

/** @type {import('./$types').RequestHandler} */
export async function GET({url}) {
    let namesOnly = url.searchParams.get('namesOnly');
    console.log('Class GET request with namesOnly: ' + namesOnly)
    if (namesOnly === "1") {
        return json(
            {
                0: "8o",
                1: "8c"
            });
    }
}