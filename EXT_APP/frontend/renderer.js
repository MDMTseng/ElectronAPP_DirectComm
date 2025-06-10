const path = require('path');
// Correctly reference the addon from the src/native directory

// console.log(path.join(__dirname, 'native', 'build', 'Release', 'addon.node'));
const addon = require("/Users/mdm/workspace/LittleJourney/ElectronWebDirect/src/native/build/Release/addon.node");

const myButton = document.getElementById('my-button');
const dataDisplay = document.getElementById('data-display');
const exchangeButton = document.getElementById('exchange-button');
const exchangeDisplay = document.getElementById('exchange-display');
const loadButton = document.getElementById('load-button');
const unloadButton = document.getElementById('unload-button');
const libStatus = document.getElementById('lib-status');

myButton.addEventListener('click', () => {
    const data = addon.hello();
    dataDisplay.textContent = data;
});

loadButton.addEventListener('click', () => {
    try {
        addon.loadDyLib("/Users/mdm/workspace/LittleJourney/ElectronWebDirect/EXT_APP/backend/build/libdlib.dylib");
        libStatus.textContent = 'Loaded';
        libStatus.style.color = 'green';
        exchangeDisplay.textContent = '';
    } catch (e) {
        libStatus.textContent = `Error: ${e.message}`;
        libStatus.style.color = 'red';
    }
});

unloadButton.addEventListener('click', () => {
    try {
        addon.unloadDyLib();
        libStatus.textContent = 'Not Loaded';
        libStatus.style.color = 'black';
        exchangeDisplay.textContent = '';
    } catch (e) {
        libStatus.textContent = `Error: ${e.message}`;
        libStatus.style.color = 'red';
    }
});

exchangeButton.addEventListener('click', () => {
    // Create a buffer that is large enough to hold the result.
    const buffer = Buffer.alloc(128); 
    try {
        const bytesWritten = addon.exchangeDataInPlace(buffer);

        if (bytesWritten > 0) {
            // Display the part of the buffer that was written to.
            exchangeDisplay.textContent = buffer.slice(0, bytesWritten).toString();
        } else {
            exchangeDisplay.textContent = 'Error: dlib function returned 0, maybe buffer too small?';
        }
    } catch (e) {
        exchangeDisplay.textContent = `Error: ${e.message}`;
    }
});