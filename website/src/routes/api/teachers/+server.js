import {json} from "@sveltejs/kit";

/** @type {import('./$types').RequestHandler} */
export async function GET({url}) {
    let id = url.searchParams.get('id');
    console.log('Teachers GET request with id: ' + id);
    if (id === null) {
        return json(
            {
                "0": {
                    "name": "Hr. Schwarzenegger"
                },
                "1": {
                    "name": "Hr. Cruise"
                }
            });
    }
    return json(
        {
            "name": "Hr. Schwarzenegger",
            "username": "SchwarzeneggerA-MPG"
        });
}

/** @type {import('./$types').RequestHandler} */
export async function PATCH({url, request}) {
    let studentId = url.searchParams.get('id');
    console.log('Teacher PATCH request with id: ' + studentId);
    let json = await request.json();
    console.log(json);
    if (json.name === "Hr. Jürgen") {
        console.log('Teacher PATCH 409');
        return new Response(null, {status: 409});
    }
    return new Response(null, {status: 200});
}

/** @type {import('./$types').RequestHandler} */
export async function PUT({request}) {
    let data = await request.json();
    console.log('Teacher PUT request')
    console.log(data);
    if (data.name === "Hr. Jürgen") {
        console.log('Teacher PUT 409');
        return new Response(null, {status: 409});
    }
    return json(
        {
            "id": 1
        });
}

/** @type {import('./$types').RequestHandler} */
export async function DELETE({url}) {
    let runId = url.searchParams.get('id');
    console.log('Deleted teacher with id: ' + runId)
    return new Response(null, {status: 200});
}