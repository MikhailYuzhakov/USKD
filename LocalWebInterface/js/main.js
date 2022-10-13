const URI = window.location.origin;
let probes = [];
let isLoggerShow = false;

Array.prototype.findLast = function(fn) {
    for(let i = this.length - 1; i >= 0; i--) 
      if(fn( this[i], i, this )) return this[i];
    return null;
};


function setupProbesConnect() {
    const source = new EventSource('/probes');
    source.onmessage = (event) => {
        if (event.data && !probes.length) {
            removeSpinner();
        }

        if (event.data) {
            let probe = event.data
                .replaceAll(/\\|\//g, '')
                .replaceAll('"{', '{')
                .replaceAll('}"', '}')
                .replaceAll(',', ', ');
            addLog(probe);
            let newProbe = JSON.parse(probe);
            let logsList = localStorage.getItem('logsList');
            if (logsList) {
                logsList = JSON.parse(logsList);
                logsList.forEach(log => {
                    if (typeof log === 'string') {
                        log = JSON.parse(log);
                    }
                });
                probes = logsList;
                if (logsList.length > 100) {
                    logsList.shift();
                }
                logsList.push(probe);
                const logsString = JSON.stringify(logsList);
                localStorage.removeItem('logsList');
                localStorage.setItem('logsList', logsString);
            } else {
                probes.push(newProbe);
                localStorage.setItem('logsList', JSON.stringify(probes));
            }
            
            let foundProbe = probes.find(x => x.id == newProbe.id);
            if (foundProbe) {
                foundProbe = newProbe;
                updateCard(newProbe);
            } else {
                probes.push(newProbe);
                createCard(newProbe);
            }
        }
    }
}

async function getHotspotConfig() {
    let response = await fetch(URI + '/hotspot-config');
    let hotspotConfig = await response.json();
    document.getElementById('title').innerText = `Базовая станция №${hotspotConfig.id}`;
}

function removeSpinner() {
    const spinner = document.getElementById('loader-container');
    if (spinner) {
        spinner.remove();
    }
}

function parseDate(paramDate) {
    if (!paramDate) return 'Нет даты';
    let mappedString = paramDate.substring(0, paramDate.length - 2).substring(2);
    const [date, time] = mappedString.split(' ');
    let [year, month, day] = date.split('-');
    month = month < 10 ? `0${month}` : month;
    day = day < 10 ? `0${day}` : day;
    const parsedDate = `${day}.${month}.${year} ${time}`;
    return parsedDate;
}

function createCard(probe) {
    const date = parseDate(probe?.datetime);
    const container = document.getElementById('probes-cards');
    container.innerHTML += `
    <div class="card" id="card_${probe?.id || 'Нет данных'}">
    <div class="card__header">
        <p class="card__title">${'Зонд №' + probe?.id || 'Нет данных'}</p>
        <p class="card__charge">Заряд ${probe?.charge || 'Нет данных'}%</p>
    </div>
    <div class="card__content">
        <div class="card__date">
            <span class="card__last-active">Выходил на связь</span>
            <span class="card__date-value">${date || 'Нет данных'}</span>
        </div>                     
        <div class="card__attributes">
            <div class="card__row">
                <div class="card__column">
                    <div class="data">
                        <span class="data__attribute">
                            Температура воздуха (DS3231)
                        </span>
                        <span class="data__value" id="temp_rtc">
                           ${probe?.temp_rtc || 'Нет данных'} °C
                        </span>
                    </div>
                </div>
                <div class="card__column">
                    <div class="data">
                        <span class="data__attribute">
                            Температура воздуха (BMP280)
                        </span>
                        <span class="data__value" id="atemp">
                            ${probe?.atemp || 'Нет данных'} °C
                        </span>
                    </div>
                </div>
            </div>
            <div class="card__row">
                <div class="card__column">
                    <div class="data">
                        <span class="data__attribute">
                            Температура воздуха (HTU21)
                        </span>
                        <span class="data__value" id="temp_htu">
                            ${probe?.temp_htu || 'Нет данных'} °C
                        </span>
                    </div>
                </div>
                <div class="card__column">
                    <div class="data">
                        <span class="data__attribute">
                            Атмосферное давление
                        </span>
                        <span class="data__value" id="press_bmp">
                            ${probe?.press_bmp || 'Нет данных'} мм рт. ст.
                        </span>
                    </div>
                </div>
            </div>
            <div class="card__row">
                <div class="card__column">
                    <div class="data">
                        <span class="data__attribute">
                            Относительная влажность
                        </span>
                        <span class="data__value" id="hum_htu">
                            ${probe?.hum_htu || 'Нет данных'}%
                        </span>
                    </div>
                </div>
                <div class="card__column">
                    <div class="data">
                        <span class="data__attribute">
                            Температура почвы
                        </span>
                        <span class="data__value" id="stemp">
                            ${probe?.stemp || 'Нет данных'} °C
                        </span>
                    </div>
                </div>
            </div>
            <div class="card__row">
                <div class="card__column">
                    <div class="data">
                        <span class="data__attribute">
                            Напряжение АКБ
                        </span>
                        <span class="data__value" id="vacc">
                            ${probe?.vacc || 'Нет данных'}В
                        </span>
                    </div>
                </div>
                <div class="card__column">
                    <div class="data">
                        <span class="data__attribute">
                            RSSI
                        </span>
                        <span class="data__value" id="rssi">
                            ${probe?.rssi || 'Нет данных'} дБ
                        </span>
                    </div>
                </div>
            </div>
        </div>
        </div>
    </div>`;
}

function updateCard(probe) {
    const cardId = `card_${probe.id}`;
    const card = document.getElementById(cardId);
    if (card) {
        card.getElementsByClassName('card__charge')[0].innerText = `Заряд ${probe.charge}%`;
        card.getElementsByClassName('card__date-value')[0].innerText = parseDate(probe.datetime);
        document.querySelector(`#${cardId} #temp_rtc`).innerHTML = `${probe.temp_rtc}°C`;
        document.querySelector(`#${cardId} #atemp`).innerHTML = `${probe.atemp}°C`;
        document.querySelector(`#${cardId} #temp_htu`).innerHTML = `${probe.temp_htu}°C`;
        document.querySelector(`#${cardId} #press_bmp`).innerHTML = `${probe.press_bmp} мм рт.ст.`;
        document.querySelector(`#${cardId} #hum_htu`).innerHTML = `${probe.hum_htu}%`;
        document.querySelector(`#${cardId} #stemp`).innerHTML = `${probe.stemp}°C`;
        document.querySelector(`#${cardId} #vacc`).innerHTML = `${probe.vacc}В`;
        document.querySelector(`#${cardId} #rssi`).innerHTML = `${probe.rssi} дБ`;
    } else {
        createCard(probe);
    }
}

function toggleLoggerVisibility() {
    const logger = document.getElementsByClassName('logger')[0];
    const loggerIcon = document.getElementsByClassName('logger-icon')[0];
    isLoggerShow = !isLoggerShow;
    if (isLoggerShow) {
        logger.classList.add('logger-opened');
        loggerIcon.classList.add('logger-icon_active');
        return;
    }
    logger.classList.remove('logger-opened');
    loggerIcon.classList.remove('logger-icon_active');
}

function addLog(probeJSON) {
    const content = document.getElementsByClassName('logger__content')[0];
    let logDiv = document.createElement('div');
    logDiv.classList.add('log');
    let logNumber = document.createElement('span');
    logNumber.classList.add('log__number');
    let logObject = document.createElement('span');
    logObject.classList.add('log__object');
    logObject.innerText = probeJSON;
    logDiv.appendChild(logNumber);
    logDiv.appendChild(logObject);
    content.appendChild(logDiv);
    let logs = document.getElementsByClassName('log');
    if (logs.length > 100) {
        logs[0].remove();
    }
    for (let i = 0; i < logs.length; i++) {
        logs[i].querySelector('.log__number').innerText = `${i+1}:`;
    }
}

function startApp() {
    let logsList = localStorage.getItem('logsList');
    if (logsList) {
        removeSpinner();
        logsList = logsList
            .replaceAll(',', ', ')
            .replaceAll(/\\|\//g, '')
            .replaceAll('"{', '{')
            .replaceAll('}"', '}');
        logsList = JSON.parse(logsList);
        logsList.forEach(log => {
            if (typeof log === 'string') {
                log = JSON.parse(log);
            }
        });
        const allLogsIds = logsList.map(log => log.id);
        const ids = [... new Set(allLogsIds)];
        ids.sort((a, b) => a - b);
        ids.forEach(id => {
            const card = logsList.findLast(log => log.id === id);
            if (card) {
                createCard(card);
            }
        });
        logsList.forEach(log => {
            addLog(JSON.stringify(log).replaceAll(',', ', '));
        });
    }
}

getHotspotConfig();
startApp();
setupProbesConnect();
