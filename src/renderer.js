const path = require('path');
// Correctly reference the addon from the src/native directory
const addon = require(path.join(__dirname, 'native', 'build', 'Release', 'addon.node'));

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
        addon.loadDyLib();
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
    const myData = Buffer.from('Some data from JS');
    try {
        const resultBuffer = addon.exchangeData(myData);
        // The result is a buffer, we need to convert it to a string to display.
        exchangeDisplay.textContent = resultBuffer.toString();
    } catch (e) {
        exchangeDisplay.textContent = `Error: ${e.message}`;
    }
});