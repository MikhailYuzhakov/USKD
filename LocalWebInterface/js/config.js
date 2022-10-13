const URI = window.location.origin;
let probesConfigs = [];
let hotspotConfig = null;

async function getProbesConfigs() {
    let response = await fetch(URI + '/probes-configs');
    let configs = await response.json();
    probesConfigs = configs;
    if (probesConfigs?.length) {
        const select = document.getElementById('probe-id');
        probesConfigs.forEach(config => {
            opt = document.createElement('option');
            opt.value = config.id;
            opt.textContent = `Настройка зонда №${config.id}`;
            select.appendChild(opt);
        });
        setProbeConfigValues(probesConfigs[0]);
    }
}

async function getHotspotConfig() {
    let response = await fetch(URI + '/hotspot-config');
    let config = await response.json();
    hotspotConfig = config;
    if (hotspotConfig) {
        setHotspotConfigValues();
    }
}

function setProbeConfigValues(probe) {
    document.getElementById('probeTransTime').value = probe.time_trans;
    document.getElementById('probeReadTime').value = probe.time_read;
    document.getElementById('probeFreq').value = probe.freq;
    document.getElementById('probeBw').value = probe.bw;
    document.getElementById('probeSf').value = probe.sf;
}

function setHotspotConfigValues() {
    document.getElementById('hotspotSsid').value = hotspotConfig.ssid;
    document.getElementById('hotspotPass').value = hotspotConfig.pass;
    document.getElementById('hotspotApssid').value = hotspotConfig.apssid;
    document.getElementById('hotspotAppass').value = hotspotConfig.appass;
    document.getElementById('hotspotFreq').value = hotspotConfig.freq;
    document.getElementById('hotspotBandWidth').value = hotspotConfig.bw;
    document.getElementById('hotspotSf').value = hotspotConfig.sf;
    document.getElementById('hotspotPower').value = hotspotConfig.pwr;
}

function onProbeChange(id) {
    const foundProbe = probesConfigs.find(config => config.id == id);
    if (foundProbe) {
        setProbeConfigValues(foundProbe);
    }
}

async function saveHotspotConfig() {
    let url = URI + '/hotspot?';
    const { elements } = document.forms[0];
    url += `ssid=${elements.hotspotSsid.value}&`;
    url += `pass=${elements.hotspotPass.value}&`;
    url += `apssid=${elements.hotspotApssid.value}&`;
    url += `appass=${elements.hotspotAppass.value}&`;
    url += `bw=${elements.hotspotBandWidth.value}&`;
    url += `sf=${elements.hotspotSf.value}&`;
    url += `freq=${elements.hotspotFreq.value}&`;
    url += `power=${elements.hotspotPower.value}`;
    const save = document.getElementById('save-one');
    const spinner = document.getElementById('spinner-one');
    save.style.display = 'none';
    spinner.style.display = 'block';
    try {
        await fetch(url);
    } catch (error) {
        console.error('Ошибка: ', error);
    } finally {
        save.style.display = 'block';
        spinner.style.display = 'none';
    }
}

async function saveProbeConfig() {
    const id = document.getElementById('probe-id').value;
    let url = URI + '/probe?';
    const { elements } = document.forms[1];
    url += `id=${id}&`;
    url += `read-time=${elements.probeReadTime.value}&`;
    url += `bw=${elements.probeBw.value}&`;
    url += `sf=${elements.probeSf.value}&`;
    url += `freq=${elements.probeFreq.value}&`;
    url += `trans-time=${elements.probeTransTime.value}`;
    const save = document.getElementById('save-two');
    const spinner = document.getElementById('spinner-two');
    save.style.display = 'none';
    spinner.style.display = 'block';
    try {
        await fetch(url);
    } catch (error) {
        console.error('Ошибка: ', error);
    } finally {
        save.style.display = 'block';
        spinner.style.display = 'none';
    }
}

getProbesConfigs();
getHotspotConfig();
