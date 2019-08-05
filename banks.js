const fs = require('fs');
const config = require('./config.json');
const DEBUG = true;

let rawdata = fs.readFileSync(config.banksPath + '/banklist.js');
let banks = JSON.parse(rawdata);
let presetArray = {};
buildPresetArray();

// Count
// ::count::{path}
function getBankCount(path, pathArgs) {
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

    if (DEBUG) console.log('Bank Count:', bankCount);
    return bankCount;
}

// entry
// ::entry::{path}::{index}
function getBankEntry(index, pathArgs) {
    let bankEntry = -1;

    if (typeof pathArgs[2] === 'undefined' || pathArgs[2] === '') {
        bankEntry = banks[index][0] + '/';
    } else {
        // getPresetArray(pathArgs[2], preset => {
        //     bankEntry = preset[index];
        // });
        bankEntry = presetArray[pathArgs[2]][index];
    }

    if (DEBUG) console.log('Bank Entry:', bankEntry);
    return bankEntry;
}

// entryIdx
// ::entryIdx::{path}::{entryName}
function getBankIdx(entry, pathArgs) {
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

    if (DEBUG) console.log('Bank Entry Index:', entryIdx);
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

function buildPresetArray() {
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

module.exports = {
    getBankCount: getBankCount,
    getBankEntry: getBankEntry,
    getBankIdx: getBankIdx
}