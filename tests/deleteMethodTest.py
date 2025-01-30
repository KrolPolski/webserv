import requests

# Specify the URL
url = "http://localhost:8080/emptyDir"

try:
    # Send a DELETE request
    response = requests.delete(url)

    # Print the response
    print("Status Code:", response.status_code)
    print("Response Text:", response.text)
except requests.exceptions.RequestException as e:
    print("An error occurred:", e)
