let storedFile = null;

document.getElementById("fileInput").addEventListener("change", function(event){

const file = event.target.files[0];

if(file){
storedFile = file;
document.getElementById("filePath").innerText = "Selected File: " + file.name;
}

});

function processFile(){

if(!storedFile){
alert("Please select a file first");
return;
}

document.getElementById("progress").style.display="block";
document.getElementById("bar").style.width="100%";

setTimeout(()=>{

alert("File processed successfully");

const blob = new Blob(
["Processed File : " + storedFile.name],
{type:'text/plain'}
);

const url = URL.createObjectURL(blob);

const download = document.getElementById("downloadBtn");
download.href = url;
download.download = "processed_" + storedFile.name;
download.style.display = "inline";

},1200);

}