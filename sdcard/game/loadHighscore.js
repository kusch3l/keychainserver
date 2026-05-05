var highScoreJson;

async function loadJson() {
  const requestURL ="list.json";
  const request = new Request(requestURL);

  const response = await fetch(request);
  const highScoreJson = await response.json();
 	console.log(highScoreJson);
 scoreWrite(highScoreJson);
}   

function scoreWrite(highScoreJson){

	highScoreJson.sort(function(a, b){
		return b.punkte - a.punkte;
	});
	console.log(highScoreJson);
	console.log("length: "+ highScoreJson.length);
	// forschleife für json
	for (let i = 0; i <= highScoreJson.length-1; i++){
		console.log("here");
		const eintrag = highScoreJson[i];
		let scoreTable = document.createDocumentFragment();
		let row = highScoreTable.appendChild(document.createElement('tr'));
		let col1 = row.appendChild(document.createElement('td'));
		let col2 = row.appendChild(document.createElement('td'));
		let col3 = row.appendChild(document.createElement('td'));
		col1.innerHTML = i+1;
		col2.innerHTML = eintrag.name;
		col3.innerHTML = eintrag.punkte;
		document.querySelector('#highScoreTable').appendChild(scoreTable);
	}
}
