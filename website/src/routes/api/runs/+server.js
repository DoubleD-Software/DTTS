import {json} from "@sveltejs/kit";

/** @type {import('./$types').RequestHandler} */
export async function GET({url, cookies}) {
    let julianDate = url.searchParams.get('date');
    console.log(julianDate)
    if (julianDate === "2460431") {
        return json({});
    }
    return json(
        {
            "0": {
                "type": 0,
                "length": 100,
                "teacher": "Hr. Schwarzenegger",
                "class": "9y",
                "avg_grade": "2,75",
                "avg_time": 14900
            },
            "1": {
                "type": 1,
                "length": 3200,
                "teacher": "Hr. Schwarzenegger",
                "class": "9y",
                "avg_grade": "2,25",
                "avg_time": 1024000
            }
        });
}