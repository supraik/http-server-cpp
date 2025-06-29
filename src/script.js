checkServerStatus();
setInterval(checkServerStatus, 10000);

// Custom cursor
const cursor = document.querySelector(".cursor");
let mouseX = 0,
  mouseY = 0;

document.addEventListener("mousemove", (e) => {
  mouseX = e.clientX;
  mouseY = e.clientY;
  cursor.style.left = mouseX - 10 + "px";
  cursor.style.top = mouseY - 10 + "px";
});

// Cursor hover effects
const interactiveElements = document.querySelectorAll(
  "button, a, input, .file-input"
);
interactiveElements.forEach((el) => {
  el.addEventListener("mouseenter", () => cursor.classList.add("hover"));
  el.addEventListener("mouseleave", () => cursor.classList.remove("hover"));
});

// Background particles
function createParticle() {
  const particle = document.createElement("div");
  particle.className = "particle";
  particle.style.left = Math.random() * 100 + "%";
  particle.style.width = particle.style.height = Math.random() * 3 + 1 + "px";
  particle.style.animationDuration = Math.random() * 10 + 5 + "s";
  particle.style.animationDelay = Math.random() * 5 + "s";
  document.querySelector(".particles").appendChild(particle);

  setTimeout(() => {
    particle.remove();
  }, 15000);
}

function createBubble() {
  const bubble = document.createElement("div");
  bubble.className = "bubble";
  bubble.style.left = Math.random() * 100 + "%";
  const size = Math.random() * 60 + 20;
  bubble.style.width = bubble.style.height = size + "px";
  bubble.style.animationDuration = Math.random() * 15 + 10 + "s";
  bubble.style.animationDelay = Math.random() * 5 + "s";
  document.querySelector(".particles").appendChild(bubble);

  setTimeout(() => {
    bubble.remove();
  }, 25000);
}

// Create particles and bubbles
setInterval(createParticle, 200);
setInterval(createBubble, 1000);

// Navigation
const navLinks = document.querySelectorAll(".nav-link");
const sections = document.querySelectorAll(".section");

function showSection(sectionId) {
  sections.forEach((section) => {
    section.classList.remove("active");
  });

  navLinks.forEach((link) => {
    link.classList.remove("active");
  });

  document.getElementById(sectionId).classList.add("active");
  document
    .querySelector(`[data-section="${sectionId}"]`)
    .classList.add("active");
}

navLinks.forEach((link) => {
  link.addEventListener("click", (e) => {
    e.preventDefault();
    const sectionId = link.getAttribute("data-section");
    showSection(sectionId);
  });
});

// Loading overlay
function showLoading() {
  document.querySelector(".loading-overlay").classList.add("show");
}

function hideLoading() {
  document.querySelector(".loading-overlay").classList.remove("show");
}

// API Functions
const SERVER_URL = "http://localhost:4221";

async function uploadFile() {
  const fileInput = document.getElementById("fileInput");
  const responseDiv = document.getElementById("uploadResponse");

  if (!fileInput.files.length) {
    showResponse(responseDiv, "Please select a file first.", "error");
    return;
  }

  const file = fileInput.files[0];
  showLoading();

  try {
    const response = await fetch(
      `${SERVER_URL}/files/${encodeURIComponent(file.name)}`,
      {
        method: "POST",
        body: file,
        headers: {
          "Content-Type": "application/octet-stream",
        },
      }
    );

    const result = await response.text();

    if (response.ok) {
      showResponse(
        responseDiv,
        `✅ Success!\nFile uploaded: ${file.name}\nSize: ${file.size} bytes\nResponse: ${result}`,
        "success"
      );
    } else {
      showResponse(
        responseDiv,
        `❌ Error ${response.status}:\n${result}`,
        "error"
      );
    }
  } catch (error) {
    showResponse(responseDiv, `❌ Network Error:\n${error.message}`, "error");
  } finally {
    hideLoading();
  }
}

async function downloadFile() {
  const filename = document.getElementById("downloadInput").value.trim();
  const responseDiv = document.getElementById("downloadResponse");

  if (!filename) {
    showResponse(responseDiv, "Please enter a filename.", "error");
    return;
  }

  showLoading();

  try {
    const response = await fetch(
      `${SERVER_URL}/files/${encodeURIComponent(filename)}`
    );

    if (response.ok) {
      const blob = await response.blob();
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement("a");
      a.href = url;
      a.download = filename;
      a.click();
      window.URL.revokeObjectURL(url);

      showResponse(
        responseDiv,
        `✅ Success!\nFile downloaded: ${filename}\nSize: ${blob.size} bytes`,
        "success"
      );
    } else {
      const error = await response.text();
      showResponse(
        responseDiv,
        `❌ Error ${response.status}:\n${error}`,
        "error"
      );
    }
  } catch (error) {
    showResponse(responseDiv, `❌ Network Error:\n${error.message}`, "error");
  } finally {
    hideLoading();
  }
}

async function testEcho() {
  const message = document.getElementById("echoInput").value.trim();
  const responseDiv = document.getElementById("echoResponse");

  if (!message) {
    showResponse(responseDiv, "Please enter a message to echo.", "error");
    return;
  }

  showLoading();

  try {
    const response = await fetch(
      `${SERVER_URL}/echo/${encodeURIComponent(message)}`,
      {
        headers: {
          "Accept-Encoding": "gzip",
        },
      }
    );

    const result = await response.text();

    if (response.ok) {
      const encoding = response.headers.get("Content-Encoding");
      showResponse(
        responseDiv,
        `✅ Echo Response:\n"${result}"\n\nCompression: ${
          encoding || "none"
        }\nLength: ${result.length} characters`,
        "success"
      );
    } else {
      showResponse(
        responseDiv,
        `❌ Error ${response.status}:\n${result}`,
        "error"
      );
    }
  } catch (error) {
    showResponse(responseDiv, `❌ Network Error:\n${error.message}`, "error");
  } finally {
    hideLoading();
  }
}

async function getUserAgent() {
  const responseDiv = document.getElementById("userAgentResponse");
  showLoading();

  try {
    const response = await fetch(`${SERVER_URL}/user-agent`);
    const result = await response.text();

    if (response.ok) {
      showResponse(responseDiv, `✅ Your User Agent:\n${result}`, "success");
    } else {
      showResponse(
        responseDiv,
        `❌ Error ${response.status}:\n${result}`,
        "error"
      );
    }
  } catch (error) {
    showResponse(responseDiv, `❌ Network Error:\n${error.message}`, "error");
  } finally {
    hideLoading();
  }
}

function showResponse(element, message, type) {
  element.textContent = message;
  element.className = `response ${type}`;
  element.style.display = "block";
  element.scrollTop = 0;
}

// File input drag and drop
const fileInput = document.getElementById("fileInput");
const fileLabel = document.querySelector(".file-input-label");

["dragenter", "dragover", "dragleave", "drop"].forEach((eventName) => {
  fileLabel.addEventListener(eventName, preventDefaults, false);
  document.body.addEventListener(eventName, preventDefaults, false);
});

["dragenter", "dragover"].forEach((eventName) => {
  fileLabel.addEventListener(eventName, highlight, false);
});

["dragleave", "drop"].forEach((eventName) => {
  fileLabel.addEventListener(eventName, unhighlight, false);
});

fileLabel.addEventListener("drop", handleDrop, false);

function preventDefaults(e) {
  e.preventDefault();
  e.stopPropagation();
}

function highlight(e) {
  fileLabel.style.borderColor = "var(--accent-primary)";
  fileLabel.style.background = "rgba(0, 212, 255, 0.1)";
}

function unhighlight(e) {
  fileLabel.style.borderColor = "var(--border-color)";
  fileLabel.style.background = "var(--bg-secondary)";
}

function handleDrop(e) {
  const dt = e.dataTransfer;
  const files = dt.files;
  fileInput.files = files;

  if (files.length > 0) {
    fileLabel.textContent = `Selected: ${files[0].name}`;
  }
}

fileInput.addEventListener("change", function () {
  if (this.files.length > 0) {
    fileLabel.textContent = `Selected: ${this.files[0].name}`;
  }
});

// Enter key handling
document.getElementById("echoInput").addEventListener("keypress", function (e) {
  if (e.key === "Enter") {
    testEcho();
  }
});

document
  .getElementById("downloadInput")
  .addEventListener("keypress", function (e) {
    if (e.key === "Enter") {
      downloadFile();
    }
  });

async function checkServerStatus() {
  const dot = document.getElementById("status-dot");
  const text = document.getElementById("server-status-text");
  try {
    const res = await fetch(`${SERVER_URL}/echo/status`, { cache: "no-store" });
    if (res.ok) {
      dot.classList.remove("offline");
      dot.classList.add("online");
      text.textContent = "Server Status: Connected";
    } else {
      dot.classList.remove("online");
      dot.classList.add("offline");
      text.textContent = "Server Status: Disconnected";
    }
  } catch {
    dot.classList.remove("online");
    dot.classList.add("offline");
    text.textContent = "Server Status: Disconnected";
  }
}

checkServerStatus();
setInterval(checkServerStatus, 10000);

// Initialize
console.log("🚀 HTTP Server Dashboard loaded successfully!");
console.log("🔗 Server URL:", SERVER_URL);
