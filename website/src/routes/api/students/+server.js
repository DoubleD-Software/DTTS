import {json} from "@sveltejs/kit";

/** @type {import('./$types').RequestHandler} */
export async function GET({url}) {
    let studentId = url.searchParams.get('id');
    console.log('Student GET request with id: ' + studentId)
    if (studentId === "3") {
        return new Response(null, {status: 404});
    }

    return json(
        {
            "name": "Bali Schmidt",
            "gender": 0,
            "class": "8o",
            "global_avg_grade": "3,30",
            "sprint": {
                "avg_grade": "2,75",
                "avg_time": 14900
            },
            "lap_run": {
                "avg_grade": "3,90",
                "avg_time": 1187000
            },
            "runs": {
                "0": {
                    "type": 0,
                    "length": 100,
                    "grade": "2,75",
                    "time": 14900,
                    "date": 2460645
                },
                "1": {
                    "type": 1,
                    "length": 3500,
                    "grade": "3,90",
                    "time": 1187000,
                    "date": 2460589
                }
            }
        });
}