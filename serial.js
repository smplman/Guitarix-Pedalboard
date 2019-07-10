const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
// const PORTNAME = 'COM10';
// const PORTNAME = '/dev/cu.usbmodem141101';
const PORTNAME = '/dev/cu.usbserial-14110';

const fs = require('fs');
const basePath = './backing-tracks';

const player = require('play-sound')(opts = {});
let audio;

const readline = require('readline');
const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

const port = new SerialPort(PORTNAME, {
    baudRate: 115200,
});

const parser = port.pipe(new Readline({ delimiter: '\r\n' }));

// Read the port data
port.on('open', () => {
    console.log('Connected to pedalboard');

    parser.on('data', data => {
        // Clear console
        // console.log('\033[2J');
        //process.stdout.write('\033c');


        // Command ":ls:{path}"
        let args = data.split('::');
        let command = args[1];
        let path = args[2];
        let file = args[3];

        // Only show menu data
        //console.log(args);
        console.log(data);
        if (args[0]) {
            //console.log(args[0]);
        }

        if (command === 'play') {
            console.log('Play ' + basePath + path);
            // If already playing stop
            if (audio) audio.kill();
            audio = player.play(basePath + path, { mplayer: ['-loop', 0] }, function (err) {
                if (err) console.log(err)//throw err
            })
        }

        if (command === 'stop') {
            if (audio) {
                audio.kill();
            }
        }

        if (command === 'ls') {
            //console.log('Listing ' + basePath + path);
            fs.readdir(basePath + path, function (err, items) {
                let res = '<listing::' + items.join() + '>\r\n';
                // console.log("Sending: ", res);
                port.write(res);
            });
        }

        if (command === 'count') {
            //console.log('Count ' + basePath + path);
            fs.readdir(basePath + path, function (err, items) {
                let res = '<count::' + items.length + '>\r\n';
                console.log("Sending: ", res);
                port.write(res);
            });
        }

        if (command === 'entry') {
            //console.log('Entry ' + file + " " + basePath + path);
            fs.readdir(basePath + path, { withFileTypes: true }, function (err, items) {
                // console.log('ITEMS!!', items);
                let item = items[file];
                // let res = '<entry:' + items[file] + '>\r\n';
                let res = '<entry::' + item.name + (item.isDirectory() ? '/' : '') + '>\r\n';
                console.log("Sending: ", res);
                port.write(res);
            });
        }

        if (command === 'entryIdx') {
            //console.log('Entry idx ' + file + " " + basePath + path);
            fs.readdir(basePath + path, function (err, items) {
                // console.log('ITEMS!!',items);
                let res = '<entryIdx::' + items.indexOf(file) + '>\r\n';
                console.log("Sending: ", res);
                port.write(res);
            });
        }
    });

    rl.on('line', (input) => {
        port.write(input);
    });
});