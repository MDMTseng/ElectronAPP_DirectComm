// main.js
import { app, BrowserWindow, ipcMain, powerSaveBlocker, powerMonitor } from 'electron';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

let mainWindow = null;
let biosWindow = null;

// --- Keep-awake helpers ------------------------------------------------------
const blockers = { app: null, display: null };

function enableKeepAwake() {
  if (blockers.app == null) {
    blockers.app = powerSaveBlocker.start('prevent-app-suspension');
    console.log('[PSB] prevent-app-suspension started', blockers.app);
  }
  if (blockers.display == null) {
    // Prevents the display from sleeping (screensaver/off)
    blockers.display = powerSaveBlocker.start('prevent-display-sleep');
    console.log('[PSB] prevent-display-sleep started', blockers.display);
  }
}

function disableKeepAwake() {
  for (const key of Object.keys(blockers)) {
    const id = blockers[key];
    if (id != null && powerSaveBlocker.isStarted(id)) {
      powerSaveBlocker.stop(id);
      console.log(`[PSB] stopped ${key}`, id);
    }
    blockers[key] = null;
  }
}

// --- Chromium background throttling switches (optional but useful) -----------
app.commandLine.appendSwitch('disable-renderer-backgrounding');
app.commandLine.appendSwitch('disable-background-timer-throttling');
app.commandLine.appendSwitch('disable-backgrounding-occluded-windows');

function createBiosWindow() {
  biosWindow = new BrowserWindow({
    width: 600,
    height: 550,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false,
      webSecurity: false,
    },
    autoHideMenuBar: true,
  });

  biosWindow.loadFile('src/bios.html');
  biosWindow.on('closed', () => { biosWindow = null; if (!mainWindow) app.quit(); });
}

let current_config = null;
let appMode = null;           // 'dev' | 'prod'
let devServerPort = NaN;
let currentArtifactPath = null;

function createMainWindow(config) {
  current_config = config;
  appMode = config?.mode;
  currentArtifactPath = config?.artifactPath ?? null;
  if (config?.devServerPort) devServerPort = config.devServerPort;

  const isDevMode = appMode === 'dev';

  if (biosWindow) biosWindow.close();

  mainWindow = new BrowserWindow({
    width: 1000,
    height: 800,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false,
      devTools: true,
      webSecurity: false,
      backgroundThrottling: false, // <-- critical
    }
  });

  // Belt & suspenders: also available per-webContents
  mainWindow.webContents.setBackgroundThrottling(false);

  // Start keep-awake once the work window exists
  enableKeepAwake();

  let loadUrl;
  if (devServerPort && !isNaN(devServerPort)) {
    loadUrl = `http://localhost:${devServerPort}`;
  } else {
    if (!currentArtifactPath) {
      console.error('ERROR: Artifact path is required in production mode!');
      app.quit();
      return;
    }
    const indexPath = path.join(currentArtifactPath, 'frontend', 'index.html');
    loadUrl = `file://${indexPath}`;
  }

  mainWindow.loadURL(loadUrl).catch(err => {
    console.error(`Failed to load ${loadUrl}:`, err);
  });

  mainWindow.webContents.on('did-finish-load', () => {
    const rendererConfig = { mode: appMode, artifactPath: currentArtifactPath };
    mainWindow?.webContents.send('set-app-config', rendererConfig);
  });

  if (isDevMode) mainWindow.webContents.openDevTools();

  mainWindow.on('closed', () => {
    mainWindow = null;
    disableKeepAwake(); // stop blockers when main window is gone
    if (!biosWindow) app.quit();
  });
}

// IPC you already have...
ipcMain.on('launch-main-app', (event, config) => {
  if (!mainWindow) createMainWindow(config);
});
ipcMain.on('get-current-config', (event) => {
  event.reply('current-config', current_config);
  event.returnValue = current_config;
});
ipcMain.on('set_backgroundThrottling', (event, enable) => {
  mainWindow?.webContents.setBackgroundThrottling(Boolean(enable));
});

// --- Observe OS power events (for diagnostics & optional behavior) -----------
app.whenReady().then(() => {
  createBiosWindow();

  powerMonitor.on('suspend', () => {
    console.log('[Power] System suspending… (you should NOT see this if keep-awake is active)');
  });
  powerMonitor.on('resume', () => {
    console.log('[Power] System resumed');
    // Optional: re-initialize GPU/RAF heavy loops if you had paused them.
    // Avoid calling loadURL() here; let the page keep running.
  });
  powerMonitor.on('lock-screen', () => console.log('[Power] Screen locked'));
  powerMonitor.on('unlock-screen', () => console.log('[Power] Screen unlocked'));

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) createBiosWindow();
  });
});

app.on('window-all-closed', () => {
  disableKeepAwake();
  if (process.platform !== 'darwin') app.quit();
});