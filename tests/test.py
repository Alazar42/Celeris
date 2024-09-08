import requests
import json

# URL of the Celeris server endpoint
url = "http://localhost:8080/echo"

# JSON payload to send in the request
payload = {
    "key": "value"
}

# Headers for the request
headers = {
    "Content-Type": "application/json"
}

try:
    # Sending POST request
    response = requests.post(url, headers=headers, data=json.dumps(payload))
    
    # Check if the response status code is 200 (OK)
    if response.status_code == 200:
        print("Response:", response.json())
    else:
        print(f"Error: Received status code {response.status_code}")
        print("Response content:", response.text)

except Exception as e:
    print(f"An error occurred: {e}")
