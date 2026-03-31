const appRoot = document.querySelector(".app");
const fileInput = document.getElementById("fileInput");
const filePath = document.getElementById("filePath");
const uploadForm = document.getElementById("uploadForm");
const dropzone = document.getElementById("dropzone");
const progress = document.getElementById("progress");
const bar = document.getElementById("bar");
const statusMsg = document.getElementById("statusMsg");
const result = document.getElementById("result");
const downloadBtn = document.getElementById("downloadBtn");
const resultMeta = document.getElementById("resultMeta");
const submitBtn = document.getElementById("submitBtn");

const encodeUrl = appRoot?.dataset.encodeUrl || "/encode";
const decodeUrl = appRoot?.dataset.decodeUrl || "/decode";
let storedFile = null;

const updateStatus = (message, isError = false) => {
statusMsg.textContent = message;
statusMsg.style.color = isError ? "#b42318" : "";
};

const resetResult = () => {
result.style.display = "none";
downloadBtn.removeAttribute("download");
downloadBtn.href = "#";
resultMeta.textContent = "";
};

const setProgress = (value) => {
progress.style.display = "block";
bar.style.width = `${value}%`;
};

const selectFile = (file) => {
storedFile = file;
filePath.textContent = file ? `Selected: ${file.name} (${(file.size / 1024).toFixed(1)} KB)` : "";
resetResult();
};

if (fileInput) {
fileInput.addEventListener("change", (event) => {
const file = event.target.files[0];
if (file) {
selectFile(file);
}
});
}

if (dropzone) {
dropzone.addEventListener("dragover", (event) => {
event.preventDefault();
dropzone.classList.add("dragover");
});

dropzone.addEventListener("dragleave", () => {
dropzone.classList.remove("dragover");
});

dropzone.addEventListener("drop", (event) => {
event.preventDefault();
dropzone.classList.remove("dragover");
const file = event.dataTransfer.files[0];
if (file) {
fileInput.files = event.dataTransfer.files;
selectFile(file);
}
});
}

if (uploadForm) {
uploadForm.addEventListener("submit", async (event) => {
event.preventDefault();
resetResult();
if (!storedFile) {
updateStatus("Please select a file before submitting.", true);
return;
}

const mode = document.querySelector("input[name='mode']:checked")?.value || "encode";
const targetUrl = mode === "decode" ? decodeUrl : encodeUrl;
const formData = new FormData();
formData.append("file", storedFile);

submitBtn.disabled = true;
updateStatus(`Sending file to ${mode === "decode" ? "decoder" : "encoder"}...`);
setProgress(20);

try {
const response = await fetch(targetUrl, {
method: "POST",
body: formData
});

if (!response.ok) {
throw new Error(`Server returned ${response.status}`);
}

setProgress(70);
const blob = await response.blob();

const disposition = response.headers.get("content-disposition") || "";
const match = disposition.match(/filename\*?=(?:UTF-8'')?"?([^";]+)"?/i);
const suggestedName = match ? decodeURIComponent(match[1]) : `${mode}_${storedFile.name}`;

const url = URL.createObjectURL(blob);
downloadBtn.href = url;
downloadBtn.download = suggestedName;
resultMeta.textContent = `Ready: ${suggestedName}`;
result.style.display = "flex";
setProgress(100);
updateStatus("Processing complete. Download your file below.");
} catch (error) {
updateStatus(`Processing failed: ${error.message}`, true);
setProgress(0);
} finally {
submitBtn.disabled = false;
}
});
}