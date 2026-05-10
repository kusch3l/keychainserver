const form = document.getElementById('userForm');

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
    // We'll add data handling here next
    const formData = new FormData(configForm);
    const formObject = formDataToObject(formData); // Convert to object
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