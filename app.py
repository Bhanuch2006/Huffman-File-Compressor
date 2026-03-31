import subprocess
import shutil
import tempfile
from pathlib import Path

from flask import Flask, after_this_request, jsonify, request, send_file
from werkzeug.utils import secure_filename

BASE_DIR = Path(__file__).resolve().parent
ENCODER_EXE = BASE_DIR / "huffman.exe"
DECODER_EXE = BASE_DIR / "unhuffman.exe"

app = Flask(__name__, static_folder=".", static_url_path="")


def _run_tool(exe_path: Path, input_path: Path, output_path: Path):
    if not exe_path.exists():
        return False, f"Missing executable: {exe_path.name}. Build the C code first."

    try:
        result = subprocess.run(
            [str(exe_path), str(input_path), str(output_path)],
            capture_output=True,
            text=True,
            check=True,
        )
        return True, result.stdout
    except subprocess.CalledProcessError as exc:
        message = exc.stderr.strip() or exc.stdout.strip() or "Unknown processing error"
        return False, message


def _handle_request(mode: str, exe_path: Path, output_suffix: str):
    if "file" not in request.files:
        return jsonify({"error": "No file uploaded."}), 400

    uploaded = request.files["file"]
    if uploaded.filename == "":
        return jsonify({"error": "Empty filename."}), 400

    safe_name = secure_filename(uploaded.filename)
    if not safe_name:
        return jsonify({"error": "Invalid filename."}), 400

    tmpdir = tempfile.mkdtemp(prefix="huffman_")
    tmp_path = Path(tmpdir)
    input_path = tmp_path / safe_name
    uploaded.save(input_path)

    output_name = f"{input_path.stem}{output_suffix}"
    output_path = tmp_path / output_name

    ok, message = _run_tool(exe_path, input_path, output_path)
    if not ok:
        shutil.rmtree(tmp_path, ignore_errors=True)
        return jsonify({"error": message}), 500

    if not output_path.exists():
        shutil.rmtree(tmp_path, ignore_errors=True)
        return jsonify({"error": f"{mode} failed to produce output."}), 500

    @after_this_request
    def cleanup(response):
        shutil.rmtree(tmp_path, ignore_errors=True)
        return response

    return send_file(
        output_path,
        as_attachment=True,
        download_name=output_name,
    )


@app.get("/")
def index():
    return app.send_static_file("index.html")


@app.get("/upload")
def upload():
    return app.send_static_file("upload.html")


@app.post("/encode")
def encode():
    return _handle_request("encode", ENCODER_EXE, ".hzip")


@app.post("/decode")
def decode():
    return _handle_request("decode", DECODER_EXE, ".txt")


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)
