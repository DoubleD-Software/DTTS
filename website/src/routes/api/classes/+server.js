import {json} from "@sveltejs/kit";

/** @type {import('./$types').RequestHandler} */
export async function GET({url}) {
    let namesOnly = url.searchParams.get('namesOnly');
    let id = url.searchParams.get('id');
    console.log('Class GET request with id: ' + id + ' and namesOnly: ' + namesOnly);
    if (namesOnly === "1") {
        return json(
            {
                0: "8o",
                1: "8c"
            });
    } else if (id !== null && url.searchParams.has('studentsOnly')) {
        return json(
            {
                "0": "Bali Schmidt",
                "1": "Ali Baba"
            });
    } else if (id !== null) {
        return json(
            {
                "name": "8o",
                "global_avg_grade": "3,30",
                "sprint": {
                    "avg_grade": "2.75",
                    "avg_time": 14900
                },
                "lap_run": {
                    "avg_grade": "3.90",
                    "avg_time": 1187000
                },
                "runs": {
                    "0": {
                        "type": 0,
                        "length": 100,
                        "avg_grade": "2.75",
                        "avg_time": 14900,
                        "date": 2460645
                    },
                    "1": {
                        "type": 1,
                        "length": 3500,
                        "avg_grade": "3.90",
                        "avg_time": 1187000,
                        "date": 2460620
                    }
                },
                "students": {
                    "0": {
                        "name": "Bali Schmidt",
                        "gender": 0,
                        "avg_grade": "2.00"
                    },
                    "1": {
                        "name": "Ali Baba",
                        "gender": 1,
                        "avg_grade": "3.90"
                    }
                }
            });
    } else {
        return json(
            {
                "0": {
                    "name": "8o",
                    "size": 23,
                    "sprint": {
                        "avg_grade": "2.75",
                        "avg_time": 14900
                    },
                    "lap_run": {
                        "avg_grade": "3.90",
                        "avg_time": 1187000
                    }
                }
            });
    }
}

/** @type {import('./$types').RequestHandler} */
export async function PATCH({url, request}) {
    let studentId = url.searchParams.get('id');
    console.log('Class PATCH request with id: ' + studentId);
    let json = await request.json();
    console.log(json);
    if (json.name === "8c") {
        console.log('Class PATCH 409');
        return new Response(null, {status: 409});
    }
    return new Response(null, {status: 200});
}

/** @type {import('./$types').RequestHandler} */
export async function PUT({request}) {
    let data = await request.json();
    console.log('Class PUT request')
    console.log(data);
    if (data.name === "8c") {
        console.log('Class PUT 409');
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
    console.log('Deleted class with id: ' + runId)
    return new Response(null, {status: 200});
}