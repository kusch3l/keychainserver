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
    const jsonData = JSON.stringify(formObject); // Convert object to JSON string

    console.log('JSON Data:', jsonData);

    if (!formObject.old_conf_pw) {
        alert('Old password is required!');
        return;
    }
    if (!formObject.www) {
        jsonData["www"] = "off"
    }
    if (!formObject.game) {
        jsonData["game"] = "off"
    }
    if (!formObject.guestbook) {
        jsonData["guestbook"] = "off"
    }
    if (!formObject.debug) {
        jsonData["debug"] = "off"
    }
    if (!formObject.apmode) {
        jsonData["apmode"] = "off"
    }

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