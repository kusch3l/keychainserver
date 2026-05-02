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
	let counter = 1;

	highScoreJson.sort(function(a, b){
		return b.punkte - a.punkte;
	});
	console.log(highScoreJson);

	// forschleife für json
	let highScoreTable = document.createDocumentFragment();
	let row = highScoreTable.appendChild(document.createElement('tr'));
	let col1 = row.appendChild(document.createElement('td'));
	let col2 = row.appendChild(document.createElement('td'));
	let col3 = row.appendChild(document.createElement('td'));
	col1.innerHTML = roundCounter;
	col2.innerHTML = roundTime.toLocaleTimeString('de-DE');
	col3.innerHTML = time.toLocaleTimeString('de-DE');
	document.querySelector('#liste').appendChild(frag);

	counter++;
  }
