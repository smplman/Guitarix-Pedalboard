const config = require('./config.json');
const DEBUG = true;

const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const PORTNAME = config.serialPort;

const fs = require('fs');
const Banks = require('./banks');

const player = require('play-sound')(opts = {});
let audio;

const readline = require('readline');
const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

const port = new SerialPort(PORTNAME, {
    autoOpen: false,
    baudRate: 115200,
});

const parser = port.pipe(new Readline({ delimiter: '\r\n' }));

let isBank = false;
let pathArgs = [];

function openSerialPort(){
    port.open((error) => {
        if(!error) {
            console.log('Connected to pedalboard');
            return;
        }

        console.log('Serial port is not open: ' + error.message);
        setTimeout(openSerialPort, 5000);
    });
}

port.on('close', () => {
    console.log('Serial port closed.');
    openSerialPort();
});

port.on('error', (error) => {
    console.log('Serial port error: ' + error.message);
});

// Read the port data
parser.on('data', data => {
    // Clear console
    // console.log('\033[2J');
    //process.stdout.write('\033c');

    // Command ":ls:{path}"
    let args = data.split('::');
    let command = args[1];
    let path = args[2];
    let file = args[3];

    // Explode path and check for banks
    if(path) {
        pathArgs = path.split('/');
        isBank = pathArgs[1] === 'banks' ? true : false;
    }

    // Only show menu data
    if(DEBUG){
        console.log(data);
    } else if (args[0]) {
        console.log(args[0]);
    }

    if (command === 'play') {
        if (DEBUG)console.log('Play ' + config.backingTracksPath + path);
        // If already playing stop
        if (audio) audio.kill();
        audio = player.play(config.backingTracksPath + path, { mplayer: ['-loop', 0 , '-ao', 'jack'] }, function (err) {
            if (err) console.log('Error playing backing track: ' + err)//throw err
        })
    }

    if (command === 'stop') {
        if (DEBUG)console.log('Stop');
        if (audio) {
            audio.kill();
        }
    }

    if (command === 'ls') {
        if (DEBUG) console.log('Listing ' + path);
        fs.readdir(config.backingTracksPath + path, function (err, items) {
            let res = '<listing::' + items.join() + '>\r\n';
            if (DEBUG) console.log("Sending: ", res);
            port.write(res);
        });
    }

    if (command === 'count') {
        if (DEBUG) console.log('Count ' + path);
        let count = -1;

        if (isBank) {
            count = Banks.getBankCount(path, pathArgs);
        } else {
            let items = fs.readdirSync(config.backingTracksPath + path);
            count = items.length;
        }

        let res = '<count::' + count + '>\r\n';
        if (DEBUG) console.log("Sending: ", res);
        port.write(res);
    }

    if (command === 'entry') {
        if (DEBUG)console.log('Entry ' + file + " " + path);

        let entry = "";

        if(isBank) {
            entry = Banks.getBankEntry(file, pathArgs);
        } else {
            let items = fs.readdirSync(config.backingTracksPath + path, { withFileTypes: true });
            let item = items[file];
            entry = item.name + (item.isDirectory() ? '/' : '')
        }

        let res = '<entry::' + entry + '>\r\n';
        if (DEBUG) console.log("Sending: ", res);
        port.write(res);
    }

    if (command === 'entryIdx') {
        if(DEBUG)console.log('Entry idx ' + file + " " + path);

        let entryIdx = -1;

        if(isBank) {
            entryIdx = Banks.getBankIdx(file, pathArgs);
        } else {
            let items = fs.readdirSync(config.backingTracksPath + path);
            entryIdx = items.indexOf(file);
        }

        let res = '<entryIdx::' + entryIdx + '>\r\n';
        if (DEBUG) console.log("Sending: ", res);
        port.write(res);
    }
});

rl.on('line', (input) => {
    port.write(input);
});

// Open serial port
openSerialPort();