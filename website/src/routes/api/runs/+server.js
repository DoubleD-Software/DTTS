import {json} from "@sveltejs/kit";

/** @type {import('./$types').RequestHandler} */
export async function GET({url}) {
    let julianDate = url.searchParams.get('date');
    let studentId = url.searchParams.get('student');
    let runId = url.searchParams.get('id');
    console.log(runId)
    console.log(studentId)
    console.log(julianDate)

    if (julianDate !== null) {
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
    } else if (runId !== null && studentId !== null) {
        return json(
            {
                "name": "Bali Schmidt",
                "time": 770000,
                "grade": "2,00",
                "length": 750,
                "rounds": {
                    "1": {
                        "length": 150,
                        "time": 131000
                    },
                    "2": {
                        "length": 300,
                        "time": 312000
                    },
                    "3": {
                        "length": 300,
                        "time": 327000
                    }
                }
            });
    } else if (runId !== null) {
        return json(
            {
                "type": 0,
                "length": 100,
                "date": 2460336,
                "teacher": "Hr. Schwarzenegger",
                "class": "9y",
                "grading_key_male": "9 Sprint (m)",
                "grading_key_female": "9 Sprint (w)",
                "avg_grade": "2,75",
                "avg_time": 14900,
                "students": {
                    "7": {
                        "name": "Bali Schmidt",
                        "time": 14200,
                        "grade": "2,00"
                    },
                    "10": {
                        "name": "Ali Baba",
                        "time": 16200,
                        "grade": "3,90"
                    }
                }
            });
    }
    return json({ message: "Something went wrong here..." });
}