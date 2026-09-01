// GLOBAL VARIABLES
let connectionFails = 0; // number of consecutive connection failures, resets on successful fetch


/**
 * Called when the index.html page is loaded, sets up the periodic data fetching
 */
function onload() {
  updateClock(); // set clock immediately on load (will be updated every second later)

  // Fetch fresh data from the backend every one second
  setInterval(() => {
    fetchData();
    updateClock();
    updateTelemetryStatus();
  }, 1000);


  // Set up event listeners for switches and buttons
  setupEventListeners();
}

// DATA MANAGEMENT

/**
 * Fetches the latest EVA  data and updates the DOM elements accordingly.
 * In the html, there is a data-path attribute on the elements that basically registers that field as needing to be updated with data from the server
 * The path specifies where in the JSON data the value can be found, e.g. "eva.telemetry.eva1.temperature"
 *
 * Note: this is a bit of a hodgepodge to maintain backwards compatibility with the old system e.g. timers, boolean switches, etc
 */
async function fetchData() {
  let evaData, ltvErrorsData;

  try {
    // Create abort controllers with 1 second timeout
    const evaController = new AbortController();
    const ltvErrorsController = new AbortController();

    const timeoutIds = [
      setTimeout(() => evaController.abort(), 2000),
      setTimeout(() => ltvErrorsController.abort(), 2000),
    ];

    // Fetch EVA, and LTV data simultaneously
    const [evaResponse, ltvErrorsResponse] = await Promise.all([
      fetch(`/data/EVA.json`, { signal: evaController.signal }),
      fetch(`/data/LTV_ERRORS.json`, { signal: ltvErrorsController.signal }),
    ]);

    // Clear timeouts on successful response
    timeoutIds.forEach((id) => clearTimeout(id));

    [evaData, ltvErrorsData] = await Promise.all([
      evaResponse.json(),
      ltvErrorsResponse.json(),
    ]);

    //check if telemetry component has been started by seeing if EVA.status.started is true
    evaStarted = evaData?.status?.started === true;

    connectionFails = 0;
  } catch (error) {
    console.error("Fatal error fetching data:", error);
    connectionFails++;
    return;
  }

  // Update the EVA fields in the DOM
  const elements = document.querySelectorAll("[data-path]");
  elements.forEach((el) => {
    const path = el.getAttribute("data-path");

    let value;

    if (path.startsWith("eva.")) {
      value = getNestedValue(evaData, path.slice(4));
    }

    // Handle checkboxes/switches (set checked property for boolean values)
    if (el.type === "checkbox") {
      el.checked = Boolean(value);
      return; // don't set textContent for checkboxes
    }

    // Handle action buttons (START/RESET) - enable/disable based on current state
    if (el.tagName === "BUTTON" && el.hasAttribute("data-action")) {
      const action = el.getAttribute("data-action");
      const isRunning = Boolean(value);

      if (action === "start") {
        // START button: enabled when NOT running
        el.disabled = isRunning;
        el.style.opacity = isRunning ? "0.5" : "1";
      } else if (action === "reset") {
        // RESET button: enabled when IS running
        el.disabled = !isRunning;
        el.style.opacity = !isRunning ? "0.5" : "1";
      }

      return; // don't set textContent for action buttons
    }

    // Handle time formatting if data-format="time" is specified
    const format = el.getAttribute("data-format");
    if (format === "time" && typeof value === "number") {
      value = formatTime(value);
    } else if (format === "status" && typeof value === "boolean") {
      value = value ? "Complete" : "Incomplete";
    } else if (typeof value === "number") {
      // Handle other number formatting for text elements
      value = value.toFixed(2);
    }

    // Append units if specified
    const units = el.getAttribute("data-units");
    if (units) {
      value = `${value} ${units}`;
    }

    el.textContent = value;
  });
}

/**
 * Updates the server with the new value for a specific field using data-path format
 *
 * @param path Data path (e.g., "eva.dcu.eva1.batt")
 * @param value New value for the field
 */
async function updateServerData(path, value) {
  try {
    const params = new URLSearchParams();
    params.append(path, value);

    const response = await fetch(`/`, {
      method: "POST",
      headers: {
        "Content-Type": "application/x-www-form-urlencoded",
      },
      body: params,
    });
  } catch (error) {
    console.error("Fatal error updating server data:", error);
  }
}

// EVENT LISTENERS

async function setupEventListeners() {
  
  const switches = document.querySelectorAll(
    'input[type="checkbox"][data-path]'
  );

  for (const switchEl of switches) {
    const path = switchEl.getAttribute("data-path");

    // Load initial value from server
    try {
      const value = await getServerData(path);
      switchEl.checked = value;
    } catch (err) {
      console.error("Failed to load value for", path, err);
    }

    // Listen for changes
    switchEl.addEventListener("change", (event) => {
      const value = event.target.checked;
      updateServerData(path, value);
    });
  }
  // When any of the buttons with data-path are clicked, send the value to the server
  const buttons = document.querySelectorAll("button[data-path]");

  buttons.forEach((button) => {
    button.addEventListener("click", (event) => {
      const path = event.target.getAttribute("data-path");
      const action = event.target.getAttribute("data-action");
      
      // For action buttons, directly update the status field
      if (action === "start") {
        // Start: set the field to true
        updateServerData(path, true);
      } else if (action === "reset") {
        // Reset: set the field to false
        updateServerData(path, false);
      } else {
        // Fallback to old data-value system for backward compatibility
        const value = event.target.getAttribute("data-value") === "true";
        updateServerData(path, value);
      }
    });
  });
}

// HELPER FUNCTIONS


/**
 * Retrieves a nested value from an object/json using a dot-separated path
 *
 * @param obj Object to retrieve the value from
 * @param path Dot-separated path to the desired value
 * @returns The value at the specified path or undefined if not found
 */
function getNestedValue(obj, path) {
  if (!path) return undefined;

  return path.split(".").reduce((current, key) => {
    return current && current[key] !== undefined ? current[key] : undefined;
  }, obj);
}

/**
 * Formats seconds into HH:MM:SS time format
 * @param {number} seconds - Total seconds to format
 * @returns {string} Formatted time string (HH:MM:SS)
 */
function formatTime(seconds) {
  const h = Math.floor(seconds / 3600);
  const m = Math.floor((seconds % 3600) / 60);
  const s = Math.floor(seconds % 60);
  return `${String(h).padStart(2, "0")}:${String(m).padStart(2, "0")}:${String(
    s
  ).padStart(2, "0")}`;
}

/**
 * Sets a cookie in the browser
 *
 * @param cname Cookie name
 * @param cvalue Cookie value
 */
function setCookie(cname, cvalue) {
  document.cookie = `${cname}=${cvalue};path=/`;
}

/**
 * Retrieves a cookie from the browser
 *
 * @param cname Cookie name
 * @returns Cookie value or null if not found
 */
function getCookie(cname) {
  const name = `${cname}=`;
  const decodedCookie = decodeURIComponent(document.cookie);
  const ca = decodedCookie.split(";");

  for (let i = 0; i < ca.length; i++) {
    let c = ca[i];
    while (c.charAt(0) === " ") {
      c = c.substring(1);
    }
    if (c.indexOf(name) === 0) {
      return c.substring(name.length, c.length);
    }
  }

  return null;
}

/**
 * Updates the clock in the navigation bar based on military time format
 */
function updateClock() {
  const now = new Date();
  const hours = String(now.getHours()).padStart(2, "0");
  const minutes = String(now.getMinutes()).padStart(2, "0");
  const seconds = String(now.getSeconds()).padStart(2, "0");
  const timeString = `${hours}:${minutes}:${seconds}`;

  const clockElement = document.getElementById("nav-clock");
  if (clockElement) {
    clockElement.textContent = timeString;
  }
}

/**
 * Updates the telemetry status indicator in the navigation bar
 */
function updateTelemetryStatus() {
  const statusElement = document.getElementById("telemetry-status");
  if (statusElement) {
    const isConnected = connectionFails <= 2;
    const statusText = isConnected
      ? "Telemetry Connected"
      : "Telemetry Disconnected";
    statusElement.innerHTML = `● ${statusText}`;
    statusElement.style.color = isConnected ? "#28ae5f" : "#d82121ff";
  }
}
