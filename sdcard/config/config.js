const form = document.getElementById('configForm');

function formDataToObject(formData) {
    const obj = {};
    formData.forEach((value, key) => {
        // If key already exists, convert to array
        if (obj.hasOwnProperty(key)) {
            if (!Array.isArray(obj[key])) {
                obj[key] = [obj[key]]; // Wrap existing value in array
            }
            obj[key].push(value);
        } else {
            obj[key] = value;
        }
    });
    return obj;
}

form.addEventListener('submit', (e) => {
    e.preventDefault(); // Stop default submission

    const formData = new FormData(configForm);
    const formObject = formDataToObject(formData); // Convert to object

    if (!formObject.old_conf_pw) {
        alert('Admin password is required!');
        return;
    }

    formObject.www = form.querySelector('#www').checked ? "on" : "off";
    formObject.game = form.querySelector('#game').checked ? "on" : "off";
    formObject.guestbook = form.querySelector('#guestbook').checked ? "on" : "off";
    formObject.debug = form.querySelector('#debug').checked ? "on" : "off";
    formObject.apmode = form.querySelector('#apmode').checked ? "on" : "off";

    const jsonData = JSON.stringify(formObject); // Convert object to JSON string

    console.log('JSON Data:', jsonData);

    // Send JSON to server
    fetch('/api/config', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json', // Tell server we're sending JSON
        },
        body: jsonData, // The JSON string
    })
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! Status: ${response.status}`);
            }
            return response.json(); // Parse server response as JSON
        })
        .then(data => {
            console.log('Server Response:', data);
            alert('Data submitted successfully!');
        })
        .catch(error => {
            console.error('Error:', error);
            alert('Failed to submit data.');
        });
});

fetch("/api/config")
    .then(res => res.json())
    .then(data => {
        form.ssid.value = data["ssid"] ?? "";
        form.password.value = data["password"] ?? "";
        form.www.checked = data["www"] === "on";
        form.game.checked = data["game"] === "on";
        form.guestbook.checked = data["guestbook"] === "on";
        form.debug.checked = data["debug"] === "on";
        form.apmode.checked = data["apmode"] === "on";
    }
    );