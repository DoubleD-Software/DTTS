import {json} from "@sveltejs/kit";

/** @type {import('./$types').RequestHandler} */
export async function GET({url}) {
    console.log('GradingKey GET request id: ' + url.searchParams.get('id'))
    if (url.searchParams.has('type') && url.searchParams.has('length')) {
        return json(
            {
                "male": {
                    "0": "8 Sprint (m)",
                    "1": "9 Sprint (m)"
                },
                "female": {
                    "2": "8 Sprint (w)",
                    "3": "9 Sprint (w)"
                }
            });
    } else if (url.searchParams.has('id')) {
        if (url.searchParams.get('id') === '2') {
            return new Response(null, {status: 404});
        }
        return json(
            {
                "name": "8 Sprint",
                "type": 0,
                "gender": 1,
                "length": 75,
                "grades": {
                    "1.00": 10600,
                    "1.25": 10800,
                    "1.50": 11000,
                    "1.75": 11200,
                    "2.00": 11300,
                    "2.25": 11500,
                    "2.50": 11600,
                    "2.75": 11700,
                    "3.00": 11900,
                    "3.25": 12000,
                    "3.50": 12100,
                    "3.75": 12200,
                    "4.00": 12700,
                    "4.25": 12900,
                    "4.50": 13000,
                    "4.75": 13100,
                    "5.00": 13300,
                    "5.25": 13400,
                    "5.50": 13500,
                    "5.75": 13600,
                    "6.00": 13700
                }
            });
    }
    return json({
            "0": {
                "name": "8 Sprint",
                "type": 0,
                "gender": 0,
                "min_time": 12270,
                "length": 100
            },
            "1": {
                "name": "8 Rundenlauf",
                "type": 1,
                "gender": 1,
                "min_time": 1009000,
                "length": 3500
            }
        });
}

/** @type {import('./$types').RequestHandler} */
export async function PUT({request}) {
    let data = await request.json();
    console.log('GradingKey PUT request')
    console.log(data);
    if (data.name === "8c Sprint") {
        console.log('GradingKey PUT 409');
        return new Response(null, {status: 409});
    }
    return json(
        {
            "id": 1
        });
}

/** @type {import('./$types').RequestHandler} */
export async function PATCH({request, url}) {
    let data = await request.json();
    console.log('GradingKey PATCH request id: ' + url.searchParams.get('id'));
    console.log(data);
    if (data.name === "8c Sprint") {
        console.log('GradingKey PATCH 409');
        return new Response(null, {status: 409});
    }
    return new Response(null, {status: 200});
}

/** @type {import('./$types').RequestHandler} */
export async function DELETE({url}) {
    let runId = url.searchParams.get('id');
    console.log('Deleted grading key with id: ' + runId)
    return new Response(null, {status: 200});
}