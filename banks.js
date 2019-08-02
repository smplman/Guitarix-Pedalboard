const fs = require('fs');
// ~/.config/guitarix/banks
const basePath = './banks';

// fs.readdir(basePath, function (err, items) {
//     console.log(items);
// });

let rawdata = fs.readFileSync('./banks/banklist.js');
let banks = JSON.parse(rawdata);

let path = '/banks/VoxBeatleMysteryPreset/';
let pathArgs = path.split('/');

let presetArray = {};

buildPresetArray();

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

    console.log(presetArray);
}

// getBankCount();
// getBankEntry(3);
// getBankIdx('Flying');

// Count
// path
// ::count::/banks
// ::count::/jazz/
// ::count::/banks/VoxBeatleMysteryPreset/
function getBankCount(path){
    // If it's base path use banklist.js otherwise use actual bank file
    let bankCount = -1;

    if(typeof pathArgs[2] === 'undefined'){
        bankCount = banks.length;
    } else {
        getPresetArray(pathArgs[2], preset => {
            bankCount = preset.length
        });
    }

    console.log('Bank Count:', bankCount);
    return bankCount;
}

// Entry
// path / index
// ::entry::/banks::0
// ::entry::/banks/VoxBeatleMysteryPreset/::0
function getBankEntry(index){
    let bankEntry = -1;

    if (typeof pathArgs[2] === 'undefined') {
        bankEntry = banks[index][0];
    } else {
        getPresetArray(pathArgs[2], preset => {
            bankEntry = preset[index];
        });
    }

    console.log('Bank Entry:', bankEntry);
    return bankEntry;
}

// entryIdx
// path / entry
// ::entryIdx::/banks::VoxBeatleMysteryPreset
function getBankIdx(entry){
    let entryIdx = -1;

    if (typeof pathArgs[2] === 'undefined') {
        banks.forEach((bank, i) => {
            if (bank[0] === entry) {
                entryIdx = i;
            }
        });
    } else {
        getPresetArray(pathArgs[2], preset => {
            preset.forEach((bank, i) => {
                if (bank === entry) {
                    entryIdx = i;
                }
            })
        });
    }

    console.log('Bank Entry Index:', entryIdx);
    return entryIdx;
}

function getPresetArray(presetName,cb){
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