const fs = require('fs');
// ~/.config/guitarix/banks
const basePath = './banks';

// fs.readdir(basePath, function (err, items) {
//     console.log(items);
// });

let rawdata = fs.readFileSync('./banks/banklist.js');
let banks = JSON.parse(rawdata);
// console.log(banks);

banks.forEach(bank => {
    let bankName = bank[0];
    let bankFile = bank[1];
    console.log(bankName, bankFile);
});

let singleBank = fs.readFileSync('./banks/Pedal.gx');
singleBank = JSON.parse(singleBank);
console.log(singleBank[2]);
// singleBank.forEach(bank => {
//     // let bankName = bank[0];
//     // let bankFile = bank[1];
//     // console.log(bankName, bankFile);
//     console.log(bank[0]);
//     // console.log(Object.keys(bank));
// });
