const config = require('./config.json');

const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
// const PORTNAME = 'COM16';
// const PORTNAME = '/dev/cu.usbmodem141101';
const PORTNAME = config.serialPort;

const DEBUG = false;

const fs = require('fs');
const basePath = '.';

let rawdata = fs.readFileSync(config.banksPath + 'banklist.js');
let banks = JSON.parse(rawdata);
let presetArray = {};
buildPresetArray();

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

let isBank = false;
let pathArgs = [];

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
            audio = player.play(config.backingTracksPath + path, { mplayer: ['-loop', 0] }, function (err) {
                if (err) console.log(err)//throw err
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
                count = getBankCount(path);
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
                entry = getBankEntry(file);
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
                entryIdx = getBankIdx(file);
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
});

// Count
// path
// ::count::/banks
// ::count::/jazz/
// ::count::/banks/VoxBeatleMysteryPreset/
function getBankCount(path) {
    // If it's base path use banklist.js otherwise use actual bank file
    let bankCount = -1;

    if (typeof pathArgs[2] === 'undefined' || pathArgs[2] === '') {
        bankCount = banks.length;
    } else {
        // getPresetArray(pathArgs[2], preset => {
        //     bankCount = preset.length
        // });
        bankCount = presetArray[pathArgs[2]].length;
    }

    if (DEBUG)console.log('Bank Count:', bankCount);
    return bankCount;
}

// Entry
// path / index
// ::entry::/banks::0
// ::entry::/banks/VoxBeatleMysteryPreset/::0
function getBankEntry(index) {
    let bankEntry = -1;

    if (typeof pathArgs[2] === 'undefined' || pathArgs[2] === '') {
        bankEntry = banks[index][0] + '/';
    } else {
        // getPresetArray(pathArgs[2], preset => {
        //     bankEntry = preset[index];
        // });
        bankEntry = presetArray[pathArgs[2]][index];
    }

    if(DEBUG)console.log('Bank Entry:', bankEntry);
    return bankEntry;
}

// entryIdx
// path / entry
// ::entryIdx::/banks::VoxBeatleMysteryPreset
function getBankIdx(entry) {
    let entryIdx = -1;

    if (typeof pathArgs[2] === 'undefined' || pathArgs[2] === '') {
        banks.forEach((bank, i) => {
            if (bank[0] === entry) {
                entryIdx = i;
            }
        });
    } else {
        // getPresetArray(pathArgs[2], preset => {
        //     preset.forEach((bank, i) => {
        //         if (bank === entry) {
        //             entryIdx = i;
        //         }
        //     });
        // });
        presetArray[pathArgs[2]].forEach((bank, i) => {
            if (bank === entry) {
                entryIdx = i;
            }
        });
    }

    if (DEBUG)console.log('Bank Entry Index:', entryIdx);
    return entryIdx;
}

function getPresetArray(presetName, cb) {
    banks.forEach(bank => {
        let bankName = bank[0];
        let bankFile = bank[1];
        if (presetName === bankName) {
            let singleBank = fs.readFileSync('./banks/' + bankFile);
            singleBank = JSON.parse(singleBank);
            // remove items 0 and 1
            singleBank.splice(0, 2);
            // Filter out every other item because of how the bank is structured
            singleBank = singleBank.filter((element, index) => {
                return index % 2 === 0;
            });

            cb(singleBank);
        }
    });
}

function buildPresetArray(){
    banks.forEach(bank => {
        let bankName = bank[0];
        let bankFile = bank[1];

        let singleBank = fs.readFileSync('./banks/' + bankFile);
        singleBank = JSON.parse(singleBank);
        // remove items 0 and 1
        singleBank.splice(0, 2);
        // Filter out every other item because of how the bank is structured
        singleBank = singleBank.filter((element, index) => {
            return index % 2 === 0;
        });

        presetArray[bankName] = singleBank;
    });
}