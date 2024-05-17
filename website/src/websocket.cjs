const http = require('http');
const Websocket = require('ws');

const port = 8080;

let response = 0;
let time = 0;
let finished = false;

const server = http.createServer((req, res) => {
    res.writeHead(404);
    res.end();
});

const wss = new Websocket.Server({ noServer: true });

wss.on('connection', (ws) => {
    console.log('Client connected');

    ws.on('message', (message) => {
        console.log(`Received message: ${message}`);
        if (response === 0) {
            if (message.toString() === '0') {
                console.log('Sending tag name');
                ws.send('1 Bali Schmidt');
                setTimeout(() => ws.send('2'), 2000);
            } else if (message.toString() === '1') {
                console.log('Arming the system');
                response = 1;
            }
        } else {
            if (message.toString() === '3') {
                console.log('Cancelling run')
                ws.send('3');
                response = 0;
            }
        }
    });

    ws.on('close', () => {
        console.log('Client disconnected');
    });

    if (response === 0) {
        ws.send('0 Ali Baba');
        setTimeout(() => ws.send('2'), 2000);
    } else {
        time = 0;
        finished = true;
        setTimeout(() => {
            finished = false;
            ws.send('1 0');
            setTimeout(sendTime, 1000);
        }, 5000);
    }
});

function sendTime() {
    if (response === 1 && !finished) {
        time += 1000;
        wss.clients.forEach((client) => {
            client.send(`1 ${time}`);
            if (time === 6000) {
                client.send('2 {\n' +
                    '  "finished": {\n' +
                    '    "0": {\n' +
                    '      "name": "Bali Schmidt",\n' +
                    '      "time": 14200\n' +
                    '    },\n' +
                    '    "1": {\n' +
                    '      "name": "Ali Baba",\n' +
                    '      "time": 16200\n' +
                    '    } \n' +
                    '  }\n' +
                    '}')
            } else if (time === 15000) {
                client.send('4 5');
                response = 0;
            }
        });
        setTimeout(sendTime, 1000);
    }
}

server.on('upgrade', (request, socket, head) => {
    if (request.url === '/api/ws') {
        wss.handleUpgrade(request, socket, head, (ws) => {
            wss.emit('connection', ws, request);
        });
    } else {
        socket.destroy();
    }
});

server.listen(port, () => {
    console.log(`Server is listening on port ${port}`);
});
