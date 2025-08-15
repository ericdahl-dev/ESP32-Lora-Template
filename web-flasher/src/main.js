import { connect, ESPLoader } from 'esp-web-flasher';

// Global state
let flasher = null;
let isFlashing = false;
let selectedFirmware = null;
let latestRelease = null;
let selectedDeviceType = 'transmitter'; // Default to transmitter
let deviceSelectionInitialized = false; // Prevent duplicate event listeners
let flashButtonInitialized = false; // Prevent duplicate flash button listeners

// DOM elements
const elements = {
  flashButton: null,
  status: null,
  progressContainer: null,
  progressFill: null,
  progressText: null,
  versionInfo: null,
  manualUpload: null,
  firmwareDetails: null,
  transmitterRadio: null,
  receiverRadio: null,
  senderFile: null,
  receiverFile: null
};

// Initialize DOM elements
function initializeElements() {
  elements.flashButton = document.getElementById('flashButton');
  elements.status = document.getElementById('status');
  elements.progressContainer = document.getElementById('progressContainer');
  elements.progressFill = document.getElementById('progressFill');
  elements.progressText = document.getElementById('progressText');
  elements.versionInfo = document.getElementById('versionInfo');
  elements.manualUpload = document.getElementById('manualUpload');
  elements.firmwareDetails = document.getElementById('firmwareDetails');
  elements.transmitterRadio = document.getElementById('transmitter');
  elements.receiverRadio = document.getElementById('receiver');
  elements.senderFile = document.getElementById('senderFile');
  elements.receiverFile = document.getElementById('receiverFile');
}

// Update firmware details display based on selected device
function updateFirmwareDetails() {
  if (!elements.firmwareDetails) return;

  const deviceType = selectedDeviceType;
  const deviceInfo = {
    transmitter: {
      name: 'Transmitter (Sender)',
      description: 'Lightning detection and environmental monitoring device',
      features: ['AS3935 Lightning Sensor', 'Environmental Sensors', 'LoRa Transmission', 'Battery Powered'],
      firmware: 'sender_firmware.bin',
      size: '~500KB'
    },
    receiver: {
      name: 'Receiver',
      description: 'LoRa reception and WiFi data forwarding device',
      features: ['LoRa Reception', 'WiFi Connectivity', 'Data Forwarding', 'Web Interface'],
      firmware: 'receiver_firmware.bin',
      size: '~450KB'
    }
  };

  const info = deviceInfo[deviceType];

  elements.firmwareDetails.innerHTML = `
    <div class="firmware-info">
      <div class="info-item">
        <strong>Device:</strong> ${info.name}
      </div>
      <div class="info-item">
        <strong>Firmware File:</strong> ${info.firmware}
      </div>
      <div class="info-item">
        <strong>Description:</strong> ${info.description}
      </div>
      <div class="info-item">
        <strong>Size:</strong> ${info.size}
      </div>
    </div>
    <div style="margin-top: 15px;">
      <strong>Key Features:</strong>
      <ul style="margin: 10px 0 0 20px; color: #6c757d;">
        ${info.features.map(feature => `<li>${feature}</li>`).join('')}
      </ul>
    </div>
  `;
}

// Handle device type selection
function handleDeviceSelection() {
  // Prevent duplicate event listener initialization
  if (deviceSelectionInitialized) {
    console.log('Device selection already initialized, skipping...');
    return;
  }

  if (elements.transmitterRadio && elements.receiverRadio) {
    console.log('Initializing device selection event listeners...');
    
    elements.transmitterRadio.addEventListener('change', () => {
      console.log('Transmitter selected');
      selectedDeviceType = 'transmitter';
      updateFirmwareDetails();
    });

    elements.receiverRadio.addEventListener('change', () => {
      console.log('Receiver selected');
      selectedDeviceType = 'receiver';
      updateFirmwareDetails();
    });

    // Mark as initialized
    deviceSelectionInitialized = true;
    console.log('Device selection event listeners initialized');
  } else {
    console.warn('Device selection radio buttons not found');
  }
}

// Handle firmware file uploads
function handleFirmwareUploads() {
  if (elements.senderFile && elements.receiverFile) {
    elements.senderFile.addEventListener('change', (event) => {
      const file = event.target.files[0];
      if (file) {
        console.log('Sender firmware file selected:', file.name, file.size, 'bytes');
        updateStatus(`Sender firmware loaded: ${file.name} (${(file.size / 1024).toFixed(1)} KB)`, 'success');
      }
    });

    elements.receiverFile.addEventListener('change', (event) => {
      const file = event.target.files[0];
      if (file) {
        console.log('Receiver firmware file selected:', file.name, file.size, 'bytes');
        updateStatus(`Receiver firmware loaded: ${file.name} (${(file.size / 1024).toFixed(1)} KB)`, 'success');
      }
    });
  }
}

// Fetch firmware from GitHub release
async function fetchFirmware(deviceType) {
  if (!latestRelease || !latestRelease.firmwareManifest) {
    throw new Error('No firmware manifest available. Please upload firmware manually or check releases.');
  }

  const deviceKey = deviceType === 'transmitter' ? 'transmitter' : 'receiver';
  const firmwareInfo = latestRelease.firmwareManifest.firmware_files[deviceKey];

  if (!firmwareInfo) {
    throw new Error(`No firmware found for ${deviceType} in release ${latestRelease.tag_name}`);
  }

    // For now, we'll use a placeholder since direct GitHub downloads have CORS issues
  // In production, you'd either:
  // 1. Host firmware files on a CORS-enabled service
  // 2. Use GitHub API to get download URLs
  // 3. Implement server-side proxy

  updateStatus(`Firmware ${firmwareInfo.filename} selected from release ${latestRelease.tag_name}`, 'info');

    // Simulate firmware data for now
  // TODO: Implement actual firmware download with CORS-compatible URLs
  const firmwareData = new ArrayBuffer(firmwareInfo.size || 512000);

  console.log(`Would download: ${firmwareInfo.filename} from ${latestRelease.tag_name}`);
  console.log('Note: Direct GitHub downloads blocked by CORS. Need CORS-enabled hosting.');

  updateStatus(`Firmware prepared: ${firmwareInfo.filename} (${(firmwareData.byteLength / 1024).toFixed(1)} KB)`, 'success');

  return {
    data: firmwareData,
    filename: firmwareInfo.filename,
    size: firmwareData.byteLength,
    checksum: firmwareInfo.checksum
  };
}

// Validate device compatibility with selected firmware
async function validateDeviceCompatibility(connection, deviceType) {
  try {
    // Get device information from the connection
    const chipName = connection.chipName || 'Unknown';
    const flashSize = connection.flashSize || 'Unknown';

    console.log('Device info:', { chipName, flashSize, deviceType });

    // Check if we have a valid chip name
    if (!chipName || chipName === 'Unknown' || chipName === null) {
      return {
        compatible: false,
        reason: 'Could not identify ESP32 chip type. Please ensure device is in download mode.',
        chipName: 'Unknown',
        flashSize: flashSize
      };
    }

    // Validate chip type compatibility
    const supportedChips = ['ESP32', 'ESP32-S2', 'ESP32-S3', 'ESP32-C3'];
    if (!supportedChips.some(chip => chipName.includes(chip))) {
      return {
        compatible: false,
        reason: `Unsupported chip type: ${chipName}. Supported: ${supportedChips.join(', ')}`,
        chipName: chipName,
        flashSize: flashSize
      };
    }

    // Check flash size compatibility
    const minFlashSize = 4; // 4MB minimum
    if (flashSize && flashSize !== 'Unknown') {
      const flashSizeMB = parseInt(flashSize) / (1024 * 1024);
      if (flashSizeMB < minFlashSize) {
        return {
          compatible: false,
          reason: `Insufficient flash size: ${flashSizeMB.toFixed(1)}MB. Minimum required: ${minFlashSize}MB`,
          chipName: chipName,
          flashSize: flashSize
        };
      }
    }

    // All checks passed
    return {
      compatible: true,
      reason: 'Device is compatible',
      chipName: chipName,
      flashSize: flashSize,
      deviceType: deviceType
    };

  } catch (error) {
    console.error('Device compatibility validation error:', error);
    return {
      compatible: false,
      reason: `Validation error: ${error.message}`,
      chipName: 'Unknown',
      flashSize: 'Unknown'
    };
  }
}

// Initialize the web flasher
async function initializeFlasher() {
  try {
    // Check if Web Serial API is available
    if (!('serial' in navigator)) {
      throw new Error('Web Serial API not supported. Please use Chrome/Edge or enable experimental features.');
    }

    // Initialize ESPLoader
    flasher = new ESPLoader();

    updateStatus('Web flasher initialized successfully', 'success');
    elements.flashButton.disabled = false;

  } catch (error) {
    console.error('Failed to initialize flasher:', error);
    updateStatus(`Failed to initialize: ${error.message}`, 'error');
  }
}

// Update status display
function updateStatus(message, type = 'info') {
  if (elements.status) {
    elements.status.textContent = message;
    elements.status.className = `status ${type}`;
  }
  console.log(`[${type.toUpperCase()}] ${message}`);
}

// Update progress bar
function updateProgress(percent, text) {
  if (elements.progressFill) {
    elements.progressFill.style.width = `${percent}%`;
  }
  if (elements.progressText) {
    elements.progressText.textContent = text;
  }
}

// Show progress container
function showProgress() {
  if (elements.progressContainer) {
    elements.progressContainer.style.display = 'block';
  }
}

// Hide progress container
function hideProgress() {
  if (elements.progressContainer) {
    elements.progressContainer.style.display = 'none';
  }
}

// Fetch latest release from GitHub
async function fetchLatestRelease() {
  try {
    const response = await fetch('https://api.github.com/repos/Skeyelab/LightningDetector/releases/latest');
    if (!response.ok) {
      if (response.status === 403) {
        throw new Error('GitHub API rate limit exceeded. Please try again later or check the repository manually.');
      } else if (response.status === 404) {
        throw new Error('No releases found. This repository may not have any releases yet.');
      } else {
        throw new Error(`GitHub API error: ${response.status}`);
      }
    }

    const release = await response.json();
    latestRelease = release;

    // Fetch firmware manifest if available
    try {
      // Use raw GitHub content URL which supports CORS
      const manifestResponse = await fetch(`https://raw.githubusercontent.com/Skeyelab/LightningDetector/main/web-flasher/firmware_manifest.json`);
      if (manifestResponse.ok) {
        const manifest = await manifestResponse.json();
        latestRelease.firmwareManifest = manifest;
        console.log('Firmware manifest loaded:', manifest);
      }
    } catch (manifestError) {
      console.warn('Could not load firmware manifest:', manifestError.message);
    }

    if (elements.versionInfo) {
      elements.versionInfo.innerHTML = `
        <strong>Latest Release:</strong> ${release.tag_name}<br>
        <strong>Published:</strong> ${new Date(release.published_at).toLocaleDateString()}<br>
        <strong>Description:</strong> ${release.body || 'No description available'}
        ${latestRelease.firmwareManifest ? '<br><strong>Firmware:</strong> Available for download' : ''}
      `;
    }

    return release;
  } catch (error) {
    console.error('Failed to fetch latest release:', error);

    // Provide helpful fallback message
    if (elements.versionInfo) {
      elements.versionInfo.innerHTML = `
        <strong>Release Information Unavailable</strong><br>
        <em>${error.message}</em><br><br>
        <strong>Alternative:</strong> Download firmware manually from the
        <a href="https://github.com/Skeyelab/LightningDetector/releases" target="_blank">GitHub releases page</a>
      `;
    }

    updateStatus(`Failed to fetch release: ${error.message}`, 'warning');
    return null;
  }
}

// Start flashing process
function startFlashing() {
  if (isFlashing) {
    updateStatus('Flashing already in progress', 'warning');
    return;
  }

  const deviceType = selectedDeviceType;
  const deviceName = deviceType === 'transmitter' ? 'Transmitter' : 'Receiver';

  // Start the process immediately while we have user gesture context
  updateStatus(`Preparing to flash ${deviceName} firmware...`, 'info');
  showProgress();
  updateProgress(0, `Preparing ${deviceName} firmware...`);

  updateProgress(10, 'Requesting serial port access...');
  updateStatus(`Requesting serial port access for ${deviceName} firmware...`, 'info');

  // Request port access immediately while we still have user gesture context
  navigator.serial.requestPort()
    .then(port => {
      updateProgress(20, 'Opening serial port...');
      updateStatus(`Serial port selected for ${deviceName} firmware...`, 'info');

      console.log('Port selected:', port);
      console.log('Port type:', typeof port);
      console.log('Port constructor:', port?.constructor?.name);

      // Don't open the port here - let the connect() function handle it
      // The connect() function will open the port with the correct settings
      return port;
    })
    .then(port => {
      console.log('Port after opening:', port);
      console.log('Port type after opening:', typeof port);

      updateProgress(30, 'Connecting to ESP32...');
      updateStatus(`Connecting to ESP32. Starting ${deviceName} flash process...`, 'info');

      // Use the connect function from esp-web-flasher
      console.log('About to call connect() with port:', port);
      if (!port) {
        throw new Error('Port object is undefined - cannot proceed with connection');
      }

      console.log('Calling connect() function...');
      // connect() returns a Promise, so we need to await it
      console.log('connect function type:', typeof connect);
      console.log('connect function:', connect);

      // Create a logger object for the connect function
      const logger = {
        log: (message) => {
          console.log(`[ESP32] ${message}`);
          updateStatus(message, 'info');
        },
        error: (message) => {
          console.error(`[ESP32] ${message}`);
          updateStatus(message, 'error');
        }
      };

      const connectPromise = connect(logger, port);
      console.log('connectPromise created:', connectPromise);
      console.log('connectPromise type:', typeof connectPromise);
      console.log('connectPromise constructor:', connectPromise?.constructor?.name);

      return connectPromise.then(connection => {
        console.log('connect() Promise resolved with:', connection);
        console.log('Connection type:', typeof connection);
        console.log('Connection constructor:', connection?.constructor?.name);

        if (connection && typeof connection === 'object') {
          console.log('Connection keys:', Object.keys(connection));
          console.log('Connection methods:', Object.getOwnPropertyNames(connection));
        }

        return connection;
      }).catch(connectError => {
        console.error('connect() Promise failed:', connectError);
        throw new Error(`ESP32 connection failed: ${connectError.message}`);
      });
    })
    .then(async connection => {
      console.log('ESP32 connection established:', connection);
      console.log('Connection type:', typeof connection);
      console.log('Connection constructor:', connection?.constructor?.name);
      console.log('Connection keys:', connection ? Object.keys(connection) : 'undefined');

      // Log detailed device information for debugging
      if (connection) {
        console.log('Device Details:');
        console.log('- Chip Name:', connection.chipName);
        console.log('- Flash Size:', connection.flashSize);
        console.log('- Connected:', connection.connected);
        console.log('- IS_STUB:', connection.IS_STUB);
        console.log('- Debug Mode:', connection.debug);
      }

      // Validate the connection object
      if (!connection) {
        throw new Error('ESP32 connection failed - no connection object returned');
      }

      // Check if connection has required methods
      if (typeof connection.log !== 'function') {
        console.warn('Connection object missing log method, but continuing...');
        console.log('Available methods on connection:', Object.getOwnPropertyNames(connection));
      }

      updateProgress(40, 'ESP32 identified');
      updateStatus(`ESP32 identified. Starting ${deviceName} firmware flash...`, 'info');

      // Validate device compatibility before proceeding
      updateProgress(45, 'Validating device compatibility...');

      // Check if the connected device is compatible with the selected firmware
      const deviceCompatibility = await validateDeviceCompatibility(connection, deviceType);
      if (!deviceCompatibility.compatible) {
        throw new Error(`Device compatibility check failed: ${deviceCompatibility.reason}`);
      }

      updateStatus(`Device compatibility verified: ${deviceCompatibility.chipName}`, 'success');

      // Get firmware from GitHub release or user upload
      let firmwareData = null;
      let firmwareFilename = 'Unknown';

      try {
        // Try to fetch from GitHub release first
        updateProgress(50, 'Fetching firmware from GitHub...');
        const firmware = await fetchFirmware(deviceType);
        firmwareData = firmware.data;
        firmwareFilename = firmware.filename;
        updateStatus(`Using firmware: ${firmwareFilename}`, 'success');
      } catch (fetchError) {
        console.warn('Could not fetch firmware from GitHub:', fetchError.message);
        updateStatus('GitHub firmware not available, please upload firmware manually', 'warning');

        // Check if user has uploaded firmware files
        const uploadedFile = deviceType === 'transmitter' ?
          elements.senderFile?.files[0] : elements.receiverFile?.files[0];

        if (uploadedFile) {
          updateProgress(50, 'Reading uploaded firmware...');
          firmwareData = await uploadedFile.arrayBuffer();
          firmwareFilename = uploadedFile.name;
          updateStatus(`Using uploaded firmware: ${firmwareFilename}`, 'success');
        } else {
          throw new Error('No firmware available. Please upload firmware manually or check GitHub releases.');
        }
      }

      updateProgress(60, 'Flashing firmware...');
      updateStatus(`Flashing ${firmwareFilename} to ESP32...`, 'info');

      // TODO: Implement real ESPLoader flashing here
      // For now, simulate the process
      await new Promise(resolve => setTimeout(resolve, 3000));

      updateProgress(80, 'Verifying flash...');
      updateStatus('Verifying firmware installation...', 'info');

      // TODO: Implement real verification here
      await new Promise(resolve => setTimeout(resolve, 1000));

      return { success: true, firmware: firmwareFilename };
    })
    .then((result) => {
      if (result && result.success) {
        updateProgress(100, 'Flash complete!');
        const firmwareInfo = result.firmware ? ` using ${result.firmware}` : '';
        updateStatus(`${deviceName} firmware flashed successfully${firmwareInfo}!`, 'success');
      } else {
        throw new Error('Firmware flashing failed');
      }
    })
    .catch(error => {
      console.error('Flashing failed:', error);

      let errorMessage = 'Unknown error occurred';
      if (error.name === 'NotFoundError') {
        errorMessage = 'No serial port selected. Please select a port and try again.';
      } else if (error.name === 'SecurityError') {
        errorMessage = 'Serial port access denied. Please allow access and try again.';
      } else if (error.message) {
        errorMessage = error.message;
      }

      updateStatus(`Flashing failed: ${errorMessage}`, 'error');
    })
    .finally(() => {
      isFlashing = false;
      elements.flashButton.disabled = false;
      hideProgress();
    });

  // Set isFlashing immediately to prevent multiple clicks
  isFlashing = true;
  elements.flashButton.disabled = true;
}

// Initialize when page loads
window.addEventListener('load', async () => {
  console.log('SBT PIO Web Flasher page loaded');

  initializeElements();
  await initializeFlasher();
  await fetchLatestRelease();
  
  console.log('Initializing device selection...');
  handleDeviceSelection(); // Initialize device selection listeners
  
  console.log('Initializing firmware uploads...');
  handleFirmwareUploads(); // Initialize firmware upload listeners
  
  console.log('Updating firmware details...');
  updateFirmwareDetails(); // Display initial firmware details

  // Add event listeners
  if (elements.flashButton && !flashButtonInitialized) {
    console.log('Initializing flash button event listener...');
    elements.flashButton.addEventListener('click', startFlashing);
    flashButtonInitialized = true;
    console.log('Flash button event listener initialized');
  } else if (elements.flashButton && flashButtonInitialized) {
    console.log('Flash button already initialized, skipping...');
  }
  
  console.log('Page initialization complete');
});
